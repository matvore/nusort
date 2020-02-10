#include "kanji_distribution.h"

#include "commands.h"
#include "rank_coverage.h"
#include "streams.h"
#include "util.h"

int parse_kanji_distribution_flags(
	int *argc, char const *const **argv, struct kanji_distribution *kd)
{
	if (!strcmp((*argv)[0], "--short-shifted-codes")) {
		kd->short_shifted_codes = 1;
		(*argv)++;
		(*argc)--;
		return 1;
	}

	return 0;
}

struct used_bit_map {
	char m[MAPPABLE_CHAR_COUNT * MAPPABLE_CHAR_COUNT];
};

static void fill_used_bit_map(
	struct key_mapping_array const *m, struct used_bit_map *used)
{
	size_t i;

	if (!bytes_are_zero(used, sizeof(*used)))
		DIE(0, "!bytes_are_zero");

	for (i = 0; i < m->cnt; i++) {
		ssize_t first_key_off = char_to_key_index(m->el[i].orig[0]);

		if (first_key_off == -1)
			continue;

		first_key_off *= MAPPABLE_CHAR_COUNT;

		if (strlen(m->el[i].orig) >= 2)
			used->m[
				first_key_off
				+ char_to_key_index(m->el[i].orig[1])
			] = 1;
		else
			memset(used->m + first_key_off, 1, MAPPABLE_CHAR_COUNT);
	}
}

static int maybe_add_short_shifted_code(
	struct kanji_distribution *kd, struct used_bit_map const *used, int key)
{
	int shifted_key;

	if (!kd->short_shifted_codes)
		return 0;

	shifted_key = key + MAPPABLE_CHAR_COUNT / 2;
	if (!bytes_are_zero(used->m + shifted_key * MAPPABLE_CHAR_COUNT,
			    MAPPABLE_CHAR_COUNT))
		return 0;

	GROW_ARRAY_BY(kd->unused_kanji_origs, 1);
	kd->unused_kanji_origs.el[kd->unused_kanji_origs.cnt - 1][0] =
		KEY_INDEX_TO_CHAR_MAP[shifted_key];
	return 1;
}

static void fill_unused_kanji_origs(
	struct kanji_distribution *kd, struct used_bit_map const *used)
{
	size_t key1;

	if (!bytes_are_zero(&kd->unused_kanji_origs,
			    sizeof(kd->unused_kanji_origs)))
		DIE(0, "!bytes_are_zero");

	for (key1 = 0; key1 < KANJI_KEY_COUNT; key1++) {
		size_t key2;
		int unused = 0;
		struct line_stats *s;
		char key1_char = KEY_INDEX_TO_CHAR_MAP[key1];

		for (key2 = 0; key2 < KANJI_KEY_COUNT; key2++) {
			int last_index = kd->unused_kanji_origs.cnt;

			if (used->m[key1 * MAPPABLE_CHAR_COUNT + key2])
				continue;

			GROW_ARRAY_BY(kd->unused_kanji_origs, 1);
			kd->unused_kanji_origs.el[last_index][0] = key1_char;
			kd->unused_kanji_origs.el[last_index][1] =
				KEY_INDEX_TO_CHAR_MAP[key2];
			unused++;
		}

		unused += maybe_add_short_shifted_code(kd, used, key1);

		if (!unused)
			continue;

		s = &kd->line_stats[kd->line_stats_nr++];
		s->key_ch = key1_char;
		s->available = unused;
		kd->total_chars += unused;
	}
}

static int first_key(
	const struct kanji_entry *kanji,
	const struct kanji_distribution *kd)
{
	int min = 0;
	int max = kd->line_stats_nr - 1;

	if (!kd->line_stats_nr)
		DIE(0, "行が足りない");

	while (min < max) {
		int mid = (min + max) / 2;
		struct kanji_entry const *co = kd->line_stats[mid + 1].cutoff;
		if (!co)
			DIE(0, "co == NULL: [%d, %d]", min, max);
		else if (distinct_rsc_cmp(co, kanji) <= 0)
			min = mid + 1;
		else
			max = mid;
	}

	return min;
}

static int first_key_then_rank_lt(
	const struct kanji_distribution *kd,
	const struct kanji_entry *a, const struct kanji_entry *b)
{
	int a_first_key = first_key(a, kd);
	int b_first_key = first_key(b, kd);

	if (a_first_key != b_first_key)
		return a_first_key < b_first_key;
	return a->ranking < b->ranking;
}

static int is_better_cutoff(
	const struct kanji_entry *best, const struct kanji_entry *candidate)
{
	if (best->cutoff_type != candidate->cutoff_type)
		return best->cutoff_type < candidate->cutoff_type;
	return best->ranking > candidate->ranking;
}

static unsigned find_best_cutoff_for_rank_target(
	int *cumulative_offset,
	unsigned start_from_kanji,
	struct kanji_distribution const *kd)
{
	unsigned ki;
	int best_offset;
	long best_ki = -1;

	for (ki = start_from_kanji; ki < kd->available.cnt; ki++) {
		int next_offset;
		if (kd->available.el[ki]->ranking == 0xffff)
			continue;
		next_offset = rank_coverage_add_kanji(
			kd->available.el[ki]->ranking);
		if (!kd->available.el[ki + 1]->cutoff_type)
			continue;
		next_offset += *cumulative_offset;
		if (best_ki != -1) {
			if (abs(next_offset) > abs(best_offset))
				break;

			if (abs(next_offset) == abs(best_offset) &&
			    !is_better_cutoff(kd->available.el[best_ki],
					      kd->available.el[ki + 1]))
				continue;
		}

		best_offset = next_offset;
		best_ki = ki + 1;
	}

	if (ki == kd->available.cnt)
		DIE(0, "kanji_dbは小さすぎます。");
	if (best_ki == -1)
		DIE(0, "区切り漢字が見つかりませんでした。");

	*cumulative_offset = best_offset;
	return best_ki;
}

static void end_line(struct kanji_distribution *kd, struct line_stats *ls)
{
	kd->total_rank += ls->last_char_rank;
	ls->cumulative_offset = ls->offset_to_target;
	if (ls != kd->line_stats)
		ls->cumulative_offset += (ls - 1)->cumulative_offset;

	if (kd->sort_each_line_by_rsc)
		/* 部首＋画数で並べ替える */
		predictably_sort_by_rsc(ls->e, ls->e_nr);
}

void kanji_distribution_set_preexisting_convs(
	struct kanji_distribution *kd, struct key_mapping_array const *m,
	int block_already_used)
{
	int i;
	struct used_bit_map used = {0};

	Conv *preexisting_convs = xcalloc(m->cnt, sizeof(Conv));

	if (!kd->rsc_range_end)
		kd->rsc_range_end = kanji_db_nr();
	if (kd->rsc_range_end > kanji_db_nr())
		DIE(0, "rsc_range_end が範囲外: %d", kd->rsc_range_end);
	if (kd->rsc_range_end < kanji_db_nr()) {
		struct kanji_entry const *end =
			kanji_from_rsc_index(kd->rsc_range_end);
		if (!end->cutoff_type)
			DIE(0, "rsc_range_end の該当する字が区切り字ではない: "
			    "%s (%d)", end->c, kd->rsc_range_end);
	}
	if (kd->rsc_range_start > kd->rsc_range_end)
		DIE(0, "start > end: %d > %d",
		    kd->rsc_range_start, kd->rsc_range_end);

	if (block_already_used)
		fill_used_bit_map(m, &used);
	fill_unused_kanji_origs(kd, &used);

	for (i = 0; i < m->cnt; i++)
		memcpy(preexisting_convs[i], m->el[i].conv, sizeof(Conv));;

	QSORT(, preexisting_convs, m->cnt,
	      strcmp(preexisting_convs[a], preexisting_convs[b]) < 0);

	if (!bytes_are_zero(&kd->available, sizeof(kd->available)))
		DIE(0, "!bytes_are_zero");

	for (i = 0; i < kanji_db_nr(); i++) {
		const struct kanji_entry *e = kanji_db() + i;
		unsigned rsc_index = kanji_db_rsc_index(e);
		Conv *prec = NULL;

		if (rsc_index >= kd->rsc_range_end)
			continue;
		if (rsc_index < kd->rsc_range_start)
			continue;

		BSEARCH(prec, preexisting_convs, m->cnt, strcmp(*prec, e->c));

		if (prec)
			continue;

		GROW_ARRAY_BY(kd->available, 1);
		kd->available.el[kd->available.cnt - 1] = e;
	}

	FREE(preexisting_convs);

	QSORT(, kd->available.el, kd->available.cnt,
	      kd->available.el[a]->ranking < kd->available.el[b]->ranking);

	if (kd->available.cnt <= kd->total_chars)
		kd->target_rank = 0xffff;
	else
		kd->target_rank = kd->available.el[kd->total_chars]->ranking;

	kd->line_stats[0].cutoff = kanji_from_rsc_index(kd->rsc_range_start);
	if (!kd->line_stats[0].cutoff->cutoff_type)
		DIE(0, "rsc_range_start の該当する字が区切り字ではない: "
		    "%s (%d)",
		    kd->line_stats[0].cutoff->c, kd->rsc_range_start);
}

void kanji_distribution_auto_pick_cutoff(struct kanji_distribution *kd)
{
	int cumulative_offset = 0;
	int line;
	unsigned ki = 0;

	predictably_sort_by_rsc(kd->available.el, kd->available.cnt);
	for (line = 1; line < kd->line_stats_nr; line++) {
		rank_coverage_reset(
			kd->target_rank, kd->line_stats[line - 1].available);
		ki = find_best_cutoff_for_rank_target(
			&cumulative_offset, ki, kd);

		kd->line_stats[line].cutoff = kd->available.el[ki];
	}
}

static unsigned find_best_cutoff_for_assigning_all_kanji(
	unsigned start_from_kanji,
	unsigned max_basic_kanji_per_line,
	struct kanji_distribution const *kd)
{
	struct kanji_entry const **start = kd->available.el + start_from_kanji;
	struct kanji_entry const **k = start;
	unsigned basic_added = 0;

	while (k - kd->available.el < kd->available.cnt) {
		if ((**k).ranking <= BASIC_KANJI_MAX_PRI) {
			if (++basic_added > max_basic_kanji_per_line)
				break;
		}
		k++;
	}

	if (k - kd->available.el < kd->available.cnt &&
	    (**k).rsc_sort_key > (**start).rsc_sort_key) {
		while ((**k).rsc_sort_key == (**(k - 1)).rsc_sort_key)
			k--;
	}

	return k - kd->available.el;
}

void kanji_distribution_auto_pick_cutoff_exhaustive(
	struct kanji_distribution *kd, char prior_key,
	unsigned max_basic_kanji_per_line, int six_is_rh)
{
	int count = 1;
	unsigned ki = 0;
	char keys[KANJI_KEY_COUNT];
	int line;

	predictably_sort_by_rsc(kd->available.el, kd->available.cnt);
	while (1) {
		ki = find_best_cutoff_for_assigning_all_kanji(
			ki, max_basic_kanji_per_line, kd);
		if (ki == kd->available.cnt)
			break;

		if (count >= kd->line_stats_nr)
			DIE(0, "line_stats の項目がたりない: %zu",
			    kd->available.cnt);
		kd->line_stats[count].cutoff = kd->available.el[ki];
		count++;
	}

	for (line = 0; line < kd->line_stats_nr; line++)
		keys[line] = kd->line_stats[line].key_ch;

	if (!prior_key || char_to_key_index(prior_key) < 0)
		DIE(0, "prior_key が無効です: %d", prior_key);

	QSORT(, keys, kd->line_stats_nr,
	      ergonomic_lt_same_first_key(prior_key, keys[a], keys[b],
					  six_is_rh));

	QSORT(, keys, count, (char_to_key_index_or_die(keys[a]) <
			      char_to_key_index_or_die(keys[b])));

	for (line = 0; line < count; line++)
		kd->line_stats[line].key_ch = keys[line];

	kd->line_stats_nr = count;
	memset(kd->line_stats + count, 0,
	       sizeof(kd->line_stats) - sizeof(*kd->line_stats) * count);
}

int kanji_distribution_parse_user_cutoff(
	struct kanji_distribution *kd,
	char const *const *argv,
	int argc)
{
	size_t i;

	if (argc != kd->line_stats_nr - 1) {
		fprintf(err,
			 "%ld個の区切り漢字を必するけれど、%d個が渡された。\n",
			 kd->line_stats_nr - 1, argc);
		return 1;
	}

	for (i = 0; i < argc; i++) {
		const struct kanji_entry *cutoff = kanji_db_lookup(argv[i]);

		if (!cutoff) {
			fprintf(err,
				 "[ %s ] は区切り漢字に指定されている"
				 "けれど、KANJI配列に含まれていない。\n",
				 argv[i]);
			return 2;
		}
		if (!cutoff->cutoff_type) {
			fprintf(err, "[ %s ] は区切り漢字として使えません。\n",
				 argv[i]);
			return 3;
		}

		kd->line_stats[i + 1].cutoff = cutoff;
	}

	return 0;
}

void kanji_distribution_populate(struct kanji_distribution *kd)
{
	struct line_stats *line_stats;
	size_t i;

	QSORT(, kd->available.el, kd->available.cnt,
	      first_key_then_rank_lt(kd,
				     kd->available.el[a],
				     kd->available.el[b]));

	line_stats = &kd->line_stats[0];
	for (i = 0; i < kd->available.cnt; i++) {
		if (line_stats != &kd->line_stats[kd->line_stats_nr - 1] &&
		    distinct_rsc_cmp((line_stats + 1)->cutoff,
				     kd->available.el[i]) <= 0) {
			end_line(kd, line_stats);
			line_stats++;
		}

		if (kd->available.el[i]->ranking <= kd->target_rank)
			line_stats->offset_to_target--;
		if (!line_stats->available)
			continue;

		line_stats->last_char_rank = kd->available.el[i]->ranking;
		line_stats->e[line_stats->e_nr++] = kd->available.el[i];
		line_stats->available--;
		line_stats->offset_to_target++;
	}
	end_line(kd, line_stats);
}

void kanji_distribution_destroy(struct kanji_distribution *kd)
{
	DESTROY_ARRAY(kd->available);
	DESTROY_ARRAY(kd->unused_kanji_origs);
	memset(kd, 0, sizeof(*kd));
}
