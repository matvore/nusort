#include "commands.h"
#include "kanji_db.h"
#include "rank_coverage.h"
#include "romazi.h"
#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct line_stats {
	unsigned short last_char_rank;

	/* この行の全漢字の入力コードがこのキーから始まる */
	char key_ch;

	/* 未使用コード数 */
	unsigned char available;

	short offset_to_target;
	int cumulative_offset;

	/* 部首＋画数順位における、この行の全漢字より前（または同値）である。*/
	const struct kanji_entry *cutoff;

	unsigned char e_nr;
	const struct kanji_entry *e[KANJI_KEY_COUNT + 1];
};

struct kanji_distribution {
	unsigned sort_each_line_by_rad_so : 1;

	unsigned short total_rank;
	unsigned short total_chars;
	unsigned short target_rank;

	size_t line_stats_nr;
	struct line_stats line_stats[KANJI_KEY_COUNT];
};

static int first_key(
	const struct kanji_entry *kanji,
	const struct kanji_distribution *kd)
{
	size_t min = 0;
	size_t max = kd->line_stats_nr - 1;

	do {
		size_t mid = (min + max) / 2;
		if (kd->line_stats[mid + 1].cutoff->rad_so_sort_key <=
				kanji->rad_so_sort_key)
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

/* コマンドフラグ */
static int hide_kanji;
static int show_per_line_kanji_count;

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

static void end_line(struct kanji_distribution *kd, struct line_stats *s)
{
	size_t i;

	if (kd->sort_each_line_by_rad_so)
		/* 部首＋画数で並べ替える */
		QSORT(, s->e, s->e_nr,
		      s->e[a]->rad_so_sort_key < s->e[b]->rad_so_sort_key);

	if (!hide_kanji) {
		for (i = 0; i < s->e_nr; i++)
			xfprintf(out, "%s", s->e[i]->c);
		xfprintf(out, " ");
	}

	s->cumulative_offset = s->offset_to_target;
	if (s != kd->line_stats)
		s->cumulative_offset += (s - 1)->cumulative_offset;
	xfprintf(out, "(%d . %d . %d",
		 s->last_char_rank, s->offset_to_target, s->cumulative_offset);
	if (show_per_line_kanji_count)
		xfprintf(out, " . %d", s->e_nr);

	xfprintf(out, ")\n");

	kd->total_rank += s->last_char_rank;
}

static void print_stats_summary(struct kanji_distribution *kd)
{
	xfprintf(out, "各行平均位: %.1f\n",
		 (float) kd->total_rank / kd->line_stats_nr);
	xfprintf(out, "目標位:  %d\n", kd->target_rank);
	xfprintf(out, "合計漢字数:  %d\n", kd->total_chars);
}

static const struct kanji_entry *first_kanji_in_rad_so(void)
{
	struct kanji_entry *cutoff = NULL;
	BSEARCH(cutoff, kanji_db(), kanji_db_nr(),
		strcmp(cutoff->c, "一"));
	if (!cutoff)
		BUG("「一」が漢字データベースで見つかりませんでした。");
	return cutoff;
}

static int read_user_cutoff_kanji(
	struct kanji_distribution *kd,
	size_t cutoff_kanji_count,
	const char **cutoff_kanji_raw)
{
	size_t i;

	if (cutoff_kanji_count != kd->line_stats_nr - 1) {
		fprintf(err,
			"%ld個の区切り漢字を必するけれど、%ld個が渡された。\n",
			kd->line_stats_nr - 1, cutoff_kanji_count);
		return 1;
	}

	for (i = 0; i < cutoff_kanji_count; i++) {
		const struct kanji_entry *cutoff = NULL;

		BSEARCH(cutoff, kanji_db(), kanji_db_nr(),
			strcmp(cutoff->c, cutoff_kanji_raw[i]));

		if (!cutoff) {
			fprintf(err,
				"[ %s ] は区切り漢字に指定されている"
				"けれど、KANJI配列に含まれていない。\n",
				cutoff_kanji_raw[i]);
			return 2;
		}
		if (!cutoff->cutoff_type) {
			fprintf(err, "[ %s ] は区切り漢字として使えません。\n",
				cutoff_kanji_raw[i]);
			return 3;
		}

		kd->line_stats[i + 1].cutoff = cutoff;
	}

	return 0;
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
	const struct kanji_entry **k,
	size_t k_nr)
{
	size_t ki;
	int best_offset;
	ssize_t best_ki = -1;

	for (ki = start_from_kanji; ki < k_nr; ki++) {
		int next_offset = rank_coverage_add_kanji(k[ki]->ranking);
		if (!k[ki + 1]->cutoff_type)
			continue;
		next_offset += *cumulative_offset;
		if (best_ki != -1) {
			if (abs(next_offset) > abs(best_offset))
				break;

			if (abs(next_offset) == abs(best_offset) &&
			    !is_better_cutoff(k[best_ki], k[ki + 1]))
				continue;
		}

		best_offset = next_offset;
		best_ki = ki + 1;
	}

	if (ki == k_nr) {
		fprintf(stderr, "kanji_dbは小さすぎます。\n");
		exit(4);
	}
	if (best_ki == -1) {
		fprintf(stderr, "区切り漢字が見つかりませんでした。\n");
		exit(29);
	}

	*cumulative_offset = best_offset;
	return best_ki;
}

static int print_last_rank_contained_parsed_args(
	size_t cutoff_kanji_count,
	const char **cutoff_kanji_raw,
	struct kanji_distribution *kd)
{
	const struct kanji_entry **resorted;
	struct line_stats *line_stats = NULL;
	size_t resorted_nr;
	size_t i;
	int res = 0;

	get_top_keys(kd);

	resorted = xcalloc(kanji_db_nr(), sizeof(*resorted));
	resorted_nr = 0;
	for (i = 0; i < kanji_db_nr(); i++) {
		const struct kanji_entry *e = kanji_db() + i;
		if (!is_target_non_sorted_string(e->c) && e->ranking < 0xffff)
			resorted[resorted_nr++] = e;
	}
	QSORT(, resorted, resorted_nr,
	      resorted[a]->ranking < resorted[b]->ranking);
	kd->target_rank = resorted[kd->total_chars]->ranking;

	kd->line_stats[0].cutoff = first_kanji_in_rad_so();

	if (cutoff_kanji_count) {
		res = read_user_cutoff_kanji(
			kd, cutoff_kanji_count, cutoff_kanji_raw);
		if (res)
			goto cleanup;
	} else {
		int cumulative_offset = 0;
		size_t ki = 0;
		QSORT(, resorted, resorted_nr, resorted[a]->rad_so_sort_key <
					       resorted[b]->rad_so_sort_key);
		for (cutoff_kanji_count = 1;
		     cutoff_kanji_count < kd->line_stats_nr;
		     cutoff_kanji_count++) {
			rank_coverage_reset(
				kd->target_rank,
				kd->line_stats[cutoff_kanji_count - 1]
					.available);

			ki = find_best_cutoff(
				&cumulative_offset, ki, resorted, resorted_nr);

			kd->line_stats[cutoff_kanji_count].cutoff =
				resorted[ki];
		}
	}

	QSORT(, resorted, resorted_nr,
	      first_key_then_rank_lt(kd, resorted[a], resorted[b]));

	for (i = 0; i < resorted_nr; i++) {
		if (!line_stats ||
		    (line_stats != &kd->line_stats[kd->line_stats_nr - 1] &&
		     (line_stats + 1)->cutoff->rad_so_sort_key <=
		     resorted[i]->rad_so_sort_key)) {
			if (!line_stats) {
				line_stats = &kd->line_stats[0];
			} else {
				end_line(kd, line_stats);
				line_stats++;
			}
			xfprintf(out, "[ %s ] %c ",
				 line_stats->cutoff->c, line_stats->key_ch);
		}

		if (resorted[i]->ranking <= kd->target_rank)
			line_stats->offset_to_target--;
		if (!line_stats->available)
			continue;

		line_stats->last_char_rank = resorted[i]->ranking;
		line_stats->e[line_stats->e_nr++] = resorted[i];
		line_stats->available--;
		line_stats->offset_to_target++;
	}
	end_line(kd, line_stats);
	print_stats_summary(kd);

cleanup:
	free(resorted);
	return res;
}

int print_last_rank_contained(const char **argv, int argc)
{
	struct kanji_distribution kanji_distribution = {0};
	show_per_line_kanji_count = 0;

	while (argc > 0 && argv[0][0] == '-') {
		const char *arg = argv[0];
		argc--;
		argv++;
		if (!strcmp(arg, "-s")) {
			kanji_distribution.sort_each_line_by_rad_so = 1;
		} else if (!strcmp(arg, "-k")) {
			hide_kanji = 1;
		} else if (!strcmp(arg, "-n")) {
			show_per_line_kanji_count = 1;
		} else if (!strcmp(arg, "--")) {
			break;
		} else {
			fprintf(err, "フラグを認識できませんでした：%s\n", arg);
			return 3;
		}
	}

	return print_last_rank_contained_parsed_args(
		argc, argv, &kanji_distribution);
}
