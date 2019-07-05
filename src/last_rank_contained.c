#include "commands.h"
#include "kanji_db.h"
#include "romazi.h"
#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cutoff_kanji {
	unsigned int key_count;
	struct kanji_entry *k[KANJI_KEY_COUNT - 1];
};

static int first_key(
	const struct kanji_entry *kanji,
	const struct cutoff_kanji *cutoff_kanji)
{
	size_t min = 0;
	size_t max = cutoff_kanji->key_count - 1;

	do {
		size_t mid = (min + max) / 2;
		if (cutoff_kanji->k[mid]->rad_so_sort_key <=
				kanji->rad_so_sort_key)
			min = mid + 1;
		else
			max = mid;
	} while (min < max);

	return min;
}

static int first_key_then_rank_lt(
	const struct cutoff_kanji *cutoff_kanji,
	const struct kanji_entry *a, const struct kanji_entry *b)
{
	int a_first_key = first_key(a, cutoff_kanji);
	int b_first_key = first_key(b, cutoff_kanji);

	if (a_first_key != b_first_key)
		return a_first_key < b_first_key;
	return a->ranking < b->ranking;
}

struct top_key {
	char key_ch;
	unsigned char available;
};

struct line_stats {
	unsigned short last_char_rank;
	unsigned short total_rank;

	unsigned short k_nr;
	struct top_key k[KANJI_KEY_COUNT];
	short offset_to_target;
	int overall_offset;

	unsigned short total_chars;
	unsigned short target_rank;

	unsigned char e_nr;
	const struct kanji_entry *e[KANJI_KEY_COUNT + 1];
	unsigned sort_each_line_by_rad_so : 1;
};

static void get_top_keys(struct line_stats *s)
{
	struct unused_kanji_keys unused_kk;
	size_t unused_kk_index = 0;

	get_free_kanji_keys_count(&unused_kk);

	for (unused_kk_index = 0; unused_kk_index < KANJI_KEY_COUNT;
			unused_kk_index++) {
		if (!unused_kk.count[unused_kk_index])
			continue;
		s->k[s->k_nr].key_ch =
			KEY_INDEX_TO_CHAR_MAP[unused_kk_index];
		s->k[s->k_nr].available =
			(unsigned char) unused_kk.count[unused_kk_index];
		s->k_nr++;
		s->total_chars += unused_kk.count[unused_kk_index];
	}
}

static void output_char(struct line_stats *s, struct kanji_entry *k)
{
	s->last_char_rank = k->ranking;
	s->e[s->e_nr++] = k;
}

static void end_line(struct line_stats *s)
{
	size_t i;

	if (!s->last_char_rank)
		return;

	if (s->sort_each_line_by_rad_so)
		/* 部首＋画数で並べ替える */
		QSORT(, s->e, s->e_nr,
		      s->e[a]->rad_so_sort_key < s->e[b]->rad_so_sort_key);

	for (i = 0; i < s->e_nr; i++)
		fprintf(out, "%s", s->e[i]->c);

	s->overall_offset += s->offset_to_target;
	fprintf(out, " (%d . %d . %d)\n",
		s->last_char_rank, s->offset_to_target, s->overall_offset);

	s->total_rank += s->last_char_rank;
	s->last_char_rank = 0;
	s->offset_to_target = 0;
	s->e_nr = 0;
}

static void print_stats_summary(struct line_stats *s)
{
	fprintf(out, "各行平均位: %.1f\n", (float) s->total_rank / s->k_nr);
	fprintf(out, "目標位:  %d\n", s->target_rank);
	fprintf(out, "合計漢字数:  %d\n", s->total_chars);
}

static int print_last_rank_contained_parsed_args(
	size_t cutoff_kanji_count,
	const char **cutoff_kanji_raw,
	int sort_each_line_by_rad_so)
{
	struct kanji_entry **resorted;
	size_t resorted_nr;
	struct cutoff_kanji cutoff_kanji;
	size_t i;
	int curr_top_key = -1;
	struct line_stats line_stats;

	memset(&line_stats, 0, sizeof(line_stats));
	line_stats.sort_each_line_by_rad_so = sort_each_line_by_rad_so;

	get_top_keys(&line_stats);
	if (cutoff_kanji_count != line_stats.k_nr - 1) {
		fprintf(err,
			"%d個の区切り漢字を必するけれど、%ld個が渡された。\n",
			line_stats.k_nr - 1, cutoff_kanji_count);
		return 1;
	}

	for (i = 0; i < cutoff_kanji_count; i++) {
		BSEARCH(cutoff_kanji.k[i], kanji_db(), kanji_db_nr(),
			strcmp(cutoff_kanji.k[i]->c, cutoff_kanji_raw[i]));

		if (!cutoff_kanji.k[i]) {
			fprintf(err,
				"[ %s ] は区切り漢字に指定されている"
				"けれど、KANJI配列に含まれていない。\n",
				cutoff_kanji_raw[i]);
			exit(2);
		}
		if (!cutoff_kanji.k[i]->cutoff_type) {
			fprintf(err, "[ %s ] は区切り漢字として使えません。\n",
				cutoff_kanji_raw[i]);
			exit(3);
		}
	}
	cutoff_kanji.key_count = cutoff_kanji_count + 1;

	resorted = xcalloc(kanji_db_nr(), sizeof(*resorted));
	resorted_nr = 0;
	for (i = 0; i < kanji_db_nr(); i++) {
		if (!is_target_non_sorted_string(kanji_db()[i].c))
			resorted[resorted_nr++] = kanji_db() + i;
	}
	QSORT(, resorted, resorted_nr,
	      resorted[a]->ranking < resorted[b]->ranking);
	line_stats.target_rank = resorted[line_stats.total_chars]->ranking;
	QSORT(, resorted, resorted_nr,
	      first_key_then_rank_lt(&cutoff_kanji, resorted[a], resorted[b]));

	for (i = 0; i < resorted_nr; i++) {
		if (curr_top_key < 0 ||
		    (curr_top_key < line_stats.k_nr - 1 &&
		     cutoff_kanji.k[curr_top_key]->rad_so_sort_key <=
		     resorted[i]->rad_so_sort_key)) {
			const char *cutoff;

			end_line(&line_stats);

			if (curr_top_key == -1)
				cutoff = "一";
			else
				cutoff = cutoff_kanji.k[curr_top_key]->c;
			curr_top_key++;
			fprintf(out, "[ %s ] %c ",
				cutoff, line_stats.k[curr_top_key].key_ch);
		}

		if (resorted[i]->ranking <= line_stats.target_rank)
			line_stats.offset_to_target--;
		if (!line_stats.k[curr_top_key].available)
			continue;

		output_char(&line_stats, resorted[i]);
		line_stats.k[curr_top_key].available--;
		line_stats.offset_to_target++;
	}
	end_line(&line_stats);
	print_stats_summary(&line_stats);
	return 0;
}

int print_last_rank_contained(const char **argv, int argc)
{
	int sort_each_line_by_rad_so = 0;
	while (argc > 0 && argv[0][0] == '-') {
		const char *arg = argv[0];
		argc--;
		argv++;
		if (!strcmp(arg, "-s")) {
			sort_each_line_by_rad_so = 1;
		} else if (!strcmp(arg, "--")) {
			break;
		} else {
			fprintf(err, "フラグを認識できませんでした：%s\n", arg);
			return 3;
		}
	}
	return print_last_rank_contained_parsed_args(
			argc, argv, sort_each_line_by_rad_so);
}
