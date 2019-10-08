#include "kanji_distribution.h"

#include "commands.h"
#include "rank_coverage.h"
#include "util.h"

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
	struct unused_kanji_keys unused_kk;
	size_t unused_kk_index = 0;

	get_free_kanji_keys_count(&unused_kk);

	for (unused_kk_index = 0; unused_kk_index < KANJI_KEY_COUNT;
			unused_kk_index++) {
		struct line_stats *s;
		if (!unused_kk.count[unused_kk_index])
			continue;
		s = &kd->line_stats[kd->line_stats_nr++];
		s->key_ch = KEY_INDEX_TO_CHAR_MAP[unused_kk_index];
		s->available = (unsigned char) unused_kk.count[unused_kk_index];
		kd->total_chars += unused_kk.count[unused_kk_index];
	}
}

static struct kanji_entry const *first_kanji_in_rsc(void)
{
	struct kanji_entry const *cutoff = NULL;
	BSEARCH(cutoff, kanji_db(), kanji_db_nr(),
		strcmp(cutoff->c, "一"));
	if (!cutoff)
		BUG("「一」が漢字データベースで見つかりませんでした。");
	return cutoff;
}

static int is_better_cutoff(
	const struct kanji_entry *best, const struct kanji_entry *candidate)
{
	if (best->cutoff_type != candidate->cutoff_type)
		return best->cutoff_type < candidate->cutoff_type;
	return best->ranking > candidate->ranking;
}

struct resorted_kanji_db {
	const struct kanji_entry **el;
	size_t cnt;
	size_t alloc;
};

static size_t find_best_cutoff(
	int *cumulative_offset,
	size_t start_from_kanji,
	struct resorted_kanji_db *k)
{
	size_t ki;
	int best_offset;
	ssize_t best_ki = -1;

	for (ki = start_from_kanji; ki < k->cnt; ki++) {
		int next_offset = rank_coverage_add_kanji(k->el[ki]->ranking);
		if (!k->el[ki + 1]->cutoff_type)
			continue;
		next_offset += *cumulative_offset;
		if (best_ki != -1) {
			if (abs(next_offset) > abs(best_offset))
				break;

			if (abs(next_offset) == abs(best_offset) &&
			    !is_better_cutoff(k->el[best_ki], k->el[ki + 1]))
				continue;
		}

		best_offset = next_offset;
		best_ki = ki + 1;
	}

	if (ki == k->cnt)
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
		QSORT(, ls->e, ls->e_nr, ls->e[a]->distinct_rsc_sort_key <
					 ls->e[b]->distinct_rsc_sort_key);
}

static void populate_non_hard_coded(struct resorted_kanji_db *resorted)
{
	size_t i;
	for (i = 0; i < kanji_db_nr(); i++) {
		const struct kanji_entry *e = kanji_db() + i;
		if (is_target_non_sorted_string(e->c) || e->ranking == 0xffff)
			continue;

		GROW_ARRAY_BY(*resorted, 1);
		resorted->el[resorted->cnt - 1] = e;
	}
}

static void common_init(struct kanji_distribution *kd)
{
	struct resorted_kanji_db resorted = {0};
	get_top_keys(kd);
	populate_non_hard_coded(&resorted);
	QSORT(, resorted.el, resorted.cnt,
	      resorted.el[a]->ranking < resorted.el[b]->ranking);
	kd->target_rank = resorted.el[kd->total_chars]->ranking;
	DESTROY_ARRAY(resorted);

	kd->line_stats[0].cutoff = first_kanji_in_rsc();
}

void kanji_distribution_auto_pick_cutoff(struct kanji_distribution *kd)
{
	struct resorted_kanji_db resorted = {0};
	int cumulative_offset = 0;
	size_t ki = 0;
	size_t cutoff_kanji_count;

	common_init(kd);
	populate_non_hard_coded(&resorted);

	QSORT(, resorted.el, resorted.cnt,
	      resorted.el[a]->distinct_rsc_sort_key <
	      resorted.el[b]->distinct_rsc_sort_key);
	for (cutoff_kanji_count = 1;
	     cutoff_kanji_count < kd->line_stats_nr;
	     cutoff_kanji_count++) {
		rank_coverage_reset(
			kd->target_rank,
			kd->line_stats[cutoff_kanji_count - 1].available);

		ki = find_best_cutoff(&cumulative_offset, ki, &resorted);

		kd->line_stats[cutoff_kanji_count].cutoff = resorted.el[ki];
	}
	DESTROY_ARRAY(resorted);
}

int kanji_distribution_parse_user_cutoff(
	struct kanji_distribution *kd,
	char const *const *argv,
	int argc)
{
	size_t i;
	common_init(kd);

	if (argc != kd->line_stats_nr - 1) {
		xfprintf(err,
			 "%ld個の区切り漢字を必するけれど、%d個が渡された。\n",
			 kd->line_stats_nr - 1, argc);
		return 1;
	}

	for (i = 0; i < argc; i++) {
		const struct kanji_entry *cutoff = NULL;

		BSEARCH(cutoff, kanji_db(), kanji_db_nr(),
			strcmp(cutoff->c, argv[i]));

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
	struct resorted_kanji_db resorted = {0};
	size_t i;

	populate_non_hard_coded(&resorted);
	QSORT(, resorted.el, resorted.cnt,
	      first_key_then_rank_lt(kd, resorted.el[a], resorted.el[b]));

	line_stats = &kd->line_stats[0];
	for (i = 0; i < resorted.cnt; i++) {
		if (line_stats != &kd->line_stats[kd->line_stats_nr - 1] &&
		     (line_stats + 1)->cutoff->rsc_sort_key <=
		     resorted.el[i]->rsc_sort_key) {
			end_line(kd, line_stats);
			line_stats++;
		}

		if (resorted.el[i]->ranking <= kd->target_rank)
			line_stats->offset_to_target--;
		if (!line_stats->available)
			continue;

		line_stats->last_char_rank = resorted.el[i]->ranking;
		line_stats->e[line_stats->e_nr++] = resorted.el[i];
		line_stats->available--;
		line_stats->offset_to_target++;
	}
	end_line(kd, line_stats);

	DESTROY_ARRAY(resorted);
}
