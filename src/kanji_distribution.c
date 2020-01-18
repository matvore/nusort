#include "kanji_distribution.h"

#include "commands.h"
#include "rank_coverage.h"
#include "streams.h"
#include "util.h"

struct used_bit_map {
	char m[MAPPABLE_CHAR_COUNT * MAPPABLE_CHAR_COUNT];
};

static int free_as_singleton_code(struct used_bit_map const *used, int key_code)
{
	return bytes_are_zero(used->m + key_code * MAPPABLE_CHAR_COUNT,
			      MAPPABLE_CHAR_COUNT);
}

static void fill_used_bit_map(
	struct key_mapping_array const *m, struct used_bit_map *used)
{
	size_t i;

	if (!bytes_are_zero(used, sizeof(*used)))
		BUG("!bytes_are_zero");

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

static void fill_unused_kanji_orig_cnts(
	struct kanji_distribution *kd, struct used_bit_map const *used)
{
	size_t key1;

	if (!bytes_are_zero(kd->unused_kanji_orig_cnts,
			    sizeof(kd->unused_kanji_orig_cnts)))
		BUG("!bytes_are_zero");

	for (key1 = 0; key1 < KANJI_KEY_COUNT; key1++) {
		size_t key2;
		size_t shifted_key1 = key1 + MAPPABLE_CHAR_COUNT / 2;

		for (key2 = 0; key2 < KANJI_KEY_COUNT; key2++) {
			if (!used->m[key1 * MAPPABLE_CHAR_COUNT + key2])
				kd->unused_kanji_orig_cnts[key1]++;
		}

		if (free_as_singleton_code(used, shifted_key1))
			kd->unused_kanji_orig_cnts[key1]++;
	}
}

static void fill_unused_kanji_origs(
	struct kanji_distribution *kd, struct used_bit_map const *used)
{
	size_t key1;

	if (!bytes_are_zero(&kd->unused_kanji_origs,
			    sizeof(kd->unused_kanji_origs)))
		BUG("!bytes_are_zero");

	for (key1 = 0; key1 < KANJI_KEY_COUNT; key1++) {
		size_t key2;
		size_t shifted_key1 = key1 + MAPPABLE_CHAR_COUNT / 2;

		for (key2 = 0; key2 < KANJI_KEY_COUNT; key2++) {
			int last_index = kd->unused_kanji_origs.cnt;

			if (used->m[key1 * MAPPABLE_CHAR_COUNT + key2])
				continue;

			GROW_ARRAY_BY(kd->unused_kanji_origs, 1);
			kd->unused_kanji_origs.el[last_index][0] =
				KEY_INDEX_TO_CHAR_MAP[key1];
			kd->unused_kanji_origs.el[last_index][1] =
				KEY_INDEX_TO_CHAR_MAP[key2];
		}

		if (free_as_singleton_code(used, shifted_key1)) {
			int last_index = kd->unused_kanji_origs.cnt;

			GROW_ARRAY_BY(kd->unused_kanji_origs, 1);
			kd->unused_kanji_origs.el[last_index][0] =
				KEY_INDEX_TO_CHAR_MAP[shifted_key1];
		}
	}
}

static int first_key(
	const struct kanji_entry *kanji,
	const struct kanji_distribution *kd)
{
	size_t min = 0;
	size_t max = kd->line_stats_nr - 1;

	do {
		size_t mid = (min + max) / 2;
		if (kd->line_stats[mid + 1].cutoff->rsc_sort_key <=
				kanji->rsc_sort_key)
			min = mid + 1;
		else
			max = mid;
	} while (min < max);

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

static void get_top_keys(struct kanji_distribution *kd)
{
	KeyIndex ki;

	for (ki = 0; ki < KANJI_KEY_COUNT; ki++) {
		struct line_stats *s;
		uint8_t unused = kd->unused_kanji_orig_cnts[ki];
		if (!unused)
			continue;
		s = &kd->line_stats[kd->line_stats_nr++];
		s->key_ch = KEY_INDEX_TO_CHAR_MAP[ki];
		s->available = unused;
		kd->total_chars += unused;
	}
}

static int is_better_cutoff(
	const struct kanji_entry *best, const struct kanji_entry *candidate)
{
	if (best->cutoff_type != candidate->cutoff_type)
		return best->cutoff_type < candidate->cutoff_type;
	return best->ranking > candidate->ranking;
}

static size_t find_best_cutoff(
	int *cumulative_offset,
	size_t start_from_kanji,
	struct kanji_distribution const *kd)
{
	size_t ki;
	int best_offset;
	ssize_t best_ki = -1;

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
	struct kanji_distribution *kd, struct key_mapping_array const *m)
{
	int i;
	struct used_bit_map used = {0};

	Conv *preexisting_convs = xcalloc(m->cnt, sizeof(Conv));

	fill_used_bit_map(m, &used);

	fill_unused_kanji_orig_cnts(kd, &used);
	get_top_keys(kd);
	fill_unused_kanji_origs(kd, &used);

	for (i = 0; i < m->cnt; i++)
		memcpy(preexisting_convs[i], m->el[i].conv, sizeof(Conv));;

	QSORT(, preexisting_convs, m->cnt,
	      strcmp(preexisting_convs[a], preexisting_convs[b]) < 0);

	if (!bytes_are_zero(&kd->available, sizeof(kd->available)))
		BUG("!bytes_are_zero");

	for (i = 0; i < kanji_db_nr(); i++) {
		const struct kanji_entry *e = kanji_db() + i;
		Conv *prec = NULL;

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
		BUG("マッピングに使える漢字が足りない: %zu <= %d",
		    kd->available.cnt, kd->total_chars);

	kd->target_rank = kd->available.el[kd->total_chars]->ranking;

	kd->line_stats[0].cutoff = kanji_db_lookup("一");
	if (!kd->line_stats[0].cutoff)
		BUG("「一」が漢字データベースで見つかりませんでした。");
}

void kanji_distribution_auto_pick_cutoff(struct kanji_distribution *kd)
{
	int cumulative_offset = 0;
	size_t ki = 0;
	size_t cutoff_kanji_count;

	predictably_sort_by_rsc(kd->available.el, kd->available.cnt);
	for (cutoff_kanji_count = 1;
	     cutoff_kanji_count < kd->line_stats_nr;
	     cutoff_kanji_count++) {
		rank_coverage_reset(
			kd->target_rank,
			kd->line_stats[cutoff_kanji_count - 1].available);

		ki = find_best_cutoff(&cumulative_offset, ki, kd);

		kd->line_stats[cutoff_kanji_count].cutoff =
			kd->available.el[ki];
	}
}

int kanji_distribution_parse_user_cutoff(
	struct kanji_distribution *kd,
	char const *const *argv,
	int argc)
{
	size_t i;

	if (argc != kd->line_stats_nr - 1) {
		xfprintf(err,
			 "%ld個の区切り漢字を必するけれど、%d個が渡された。\n",
			 kd->line_stats_nr - 1, argc);
		return 1;
	}

	for (i = 0; i < argc; i++) {
		const struct kanji_entry *cutoff = kanji_db_lookup(argv[i]);

		if (!cutoff) {
			xfprintf(err,
				 "[ %s ] は区切り漢字に指定されている"
				 "けれど、KANJI配列に含まれていない。\n",
				 argv[i]);
			return 2;
		}
		if (!cutoff->cutoff_type) {
			xfprintf(err, "[ %s ] は区切り漢字として使えません。\n",
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
		     (line_stats + 1)->cutoff->rsc_sort_key <=
		     kd->available.el[i]->rsc_sort_key) {
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
