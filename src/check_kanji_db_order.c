#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands.h"
#include "kanji_db.h"
#include "residual_stroke_count.h"
#include "streams.h"
#include "util.h"

static const char *ADOBE_JAPAN = "\tkRSAdobe_Japan1_6";

static struct {
	unsigned db_out : 1;
	unsigned quiet : 1;
	unsigned residual_stroke_counts : 1;
} flags;

struct sort_key {
	unsigned rad: 8;
	unsigned strokes: 8;
};

static int sort_key_cmp(const struct sort_key *a, const struct sort_key *b)
{
	if (a->rad != b->rad)
		return a->rad < b->rad ? -1 : 1;
	if (a->strokes != b->strokes)
		return a->strokes < b->strokes ? -1 : 1;
	return 0;
}

#define MAX_K 15

struct sort_info {
	char c[5];
	struct sort_key k[MAX_K];
};

static void sort_and_dedup_keys(struct sort_info *si)
{
	size_t src, dest = 0;
	QSORT(, si->k, MAX_K, sort_key_cmp(si->k + a, si->k + b) < 0);

	for (src = 0; src < MAX_K; src++) {
		if (!si->k[src].rad)
			continue;
		if (src > 0 && !sort_key_cmp(&si->k[src - 1], &si->k[src]))
			continue;
		si->k[dest++] = si->k[src];
	}
	memset(si->k + dest, 0, sizeof(*si->k) * (MAX_K - dest));
}

static struct {
	struct sort_info *el;
	size_t cnt;
	size_t alloc;
} sort_infos;

static struct sort_info *last_si(void)
{
	return &sort_infos.el[sort_infos.cnt - 1];
}

static void set_utf8_byte(char *d, unsigned char c) { *d = 0x80 | (c & 0x3f); }

static void decode_codepoint(char *d, const char *codepoint_str)
{
	long codepoint = strtol(codepoint_str, NULL, 16);
	if (codepoint < 0x0800)
		DIE(0, "コードポイントが対象外: %ld", codepoint);
	if (codepoint <= 0xffff) {
		*d++ = 0xe0 | (codepoint >> 12);
	} else {
		*d++ = 0xf0 | (codepoint >> 18);
		set_utf8_byte(d++, (unsigned char)(codepoint >> 12));
	}
	set_utf8_byte(d++, (unsigned char)(codepoint >> 6));
	set_utf8_byte(d++, (unsigned char)codepoint);
	*d = 0;
}

static int add_key(int rad, int so)
{
	size_t i;
	for (i = 0; i < MAX_K; i++) {
		if (!last_si()->k[i].rad) {
			last_si()->k[i].rad = rad;
			last_si()->k[i].strokes = so;
			return 1;
		}
	}

	fprintf(err, "並べ替えキーが多すぎます\n");
	return 0;
}

static struct {
	char c[5];
	unsigned rad : 8;
	unsigned strokes : 8;
} supplemental_keys[] = {
	{"屠", 0x2d, 0x09},
	{"斎", 0x44, 0x07},
	{"蒸", 0x57, 0x09},
	{"萬", 0x73, 0x07},
	{"采", 0xa6, 0x00},
	{"舎", 0x88, 0x02},
	{"舗", 0x88, 0x09},
	{"鼡", 0x1e, 0x05},
	{"単", 0x1e, 0x06},
	{"巣", 0x1e, 0x08},
	{"営", 0x1e, 0x09},
	{"厳", 0x1e, 0x0e},
	{"柴", 0x4c, 0x05},
};

static int process_rsc_line(char const *line)
{
	char codepoint_str[6];
	int prefix_len;
	const char *cur;
	size_t i;

	if (line[0] == '#' || line[0] == '\n')
		return 0;
	switch (sscanf(line, "U+%5[0-9A-F]%n", codepoint_str, &prefix_len)) {
	case 0:
		fprintf(err,
			"警告: 行の形式が間違っています：%s\n",
			line);
		return 0;
	case 1:
		break;
	default:
		DIE(1, "行の認識に失敗しました: %s", line);
	}

	cur = line + prefix_len;
	if (strncmp(ADOBE_JAPAN, cur, strlen(ADOBE_JAPAN)))
		return 0;

	GROW_ARRAY_BY(sort_infos, 1);

	cur += strlen(ADOBE_JAPAN);
	while (1) {
		int rad, so, len;
		if (!*cur || *cur == '\n')
			break;

		switch (sscanf(cur, " %*1[CV]+%*d+%d.%*d.%d%n",
			       &rad, &so, &len)) {
		case 0:
		case 1:
			fprintf(err, "行の形式が間違っています: (列 %d)\n%s",
				(int)(cur - line), line);
			return 11;
		case 2:
			break;
		default:
			DIE(1, "行の認識に失敗しました: %s", line);
		}
		cur += len;

		/* # 一画の部首を全て同一の部首として見なす。 */
		if (rad <= 6)
			rad = 1;
		/* 匚と匸を同一の部首として見なす。*/
		if (rad == 23)
			rad = 22;
		/* 夊と夂を同一の部首として見なす。 */
		if (rad == 35)
			rad = 34;
		/* 日と曰を同一の部首と見なす。*/
		if (rad == 73)
			rad = 72;
		/*
		 * 『口』以降の部首番号に一を足す。こうするとツかんむりが『口』
		 * のもとの番号を取ることができます。
		 */
		if (rad >= 30)
			rad += 1;

		if (!add_key(rad, so))
			return 13;
	}

	decode_codepoint(last_si()->c, codepoint_str);
	for (i = 0; i < sizeof(supplemental_keys) / sizeof(*supplemental_keys);
	     i++) {
		if (!strcmp(supplemental_keys[i].c, last_si()->c) &&
		    !add_key(supplemental_keys[i].rad,
			     supplemental_keys[i].strokes))
			return 14;
	}
	sort_and_dedup_keys(last_si());
	return 0;
}

static void output_char_line(
	const struct kanji_entry *k, const struct sort_info *si)
{
	size_t ki;
	fprintf(out, "%s\t", k->c);
	for (ki = 0; ki < MAX_K && si->k[ki].rad; ki++)
		fprintf(out, "%02x%02x ",
			(int) si->k[ki].rad, (int) si->k[ki].strokes);
	fprintf(out, "\n");
}

static int figure_cutoff_type(
	const struct sort_info *prev_si, const struct sort_info *si)
{
	size_t i, j;

	/*  cutoff_typeを自動的に指定する */
	if (!prev_si)
		return 3;

	for (i = 0; i < MAX_K; i++)
		for (j = 0; j < MAX_K; j++)
			if (prev_si->k[i].rad &&
			    !sort_key_cmp(&prev_si->k[i], &si->k[j]))
				return 0;

	return 1;
}

static void output_db_line(
	const struct kanji_entry *k,
	const struct sort_info *prev_si,
	const struct sort_info *si)
{
	int cutoff_type = k->cutoff_type;
	if (!cutoff_type)
		cutoff_type = figure_cutoff_type(prev_si, si);
	printf("\t{\"%s\", %5d, %d},\n", k->c, k->ranking, cutoff_type);
}

struct min_key_info {
	struct sort_key key;
	unsigned ki;
};

static void adjust_consecutive_key_info(
	struct sort_key const *prev, struct sort_key *curr, char const *k)
{
	int cmp;
	if (curr->rad > prev->rad && curr->strokes != 0)
		DIE(0, "「%s」の部首が kanji_db に入っていない?", k);
	cmp = sort_key_cmp(prev, curr);
	if (!cmp)
		curr->strokes++;
	else if (cmp > 0)
		DIE(0, "「%s」の画数が低すぎます", k);
}

static int check_order(void)
{
	size_t i;
	struct sort_key key = {0, 0};
	int res = 0;
	struct sort_info *prev_si = NULL;
	struct kanji_entry const *k = kanji_db();
	uint16_t const *rsc = kanji_db_rsc_sorted();

	struct min_key_info *min_keys = xcalloc(largest_rsc_sort_key(),
						sizeof(*min_keys));

	memset(min_keys, 0xff, sizeof(*min_keys) * largest_rsc_sort_key());

	for (i = 0; i < kanji_db_nr(); i++) {
		struct sort_key smallest_matching = {0xff, 0xff};
		struct sort_info *si;
		size_t ki;
		struct kanji_entry const *ke = k + rsc[i];
		struct min_key_info *min_key = min_keys + ke->rsc_sort_key - 1;
		int residual_stroke_count_in_unihan_data = 0;

		BSEARCH(si, sort_infos.el, sort_infos.cnt,
			strcmp(si->c, ke->c));

		if (!si) {
			fprintf(err,
				 "Unihanで並べ替えキーが見つかり"
				 "ませんでした: %s\n",
				 ke->c);
			res = 30;
			break;
		}

		for (ki = 0; ki < MAX_K && si->k[ki].rad; ki++) {
			if (si->k[ki].strokes == residual_stroke_count(ke))
				residual_stroke_count_in_unihan_data = 1;
			if (sort_key_cmp(&si->k[ki], &key) < 0)
				continue;
			if (sort_key_cmp(&si->k[ki], &smallest_matching) < 0)
				smallest_matching = si->k[ki];
		}
		if (smallest_matching.rad == 0xff) {
			fprintf(err, "err: %s\n", ke->c);
			res = 31;
			goto cleanup;
		}
		if (!residual_stroke_count_in_unihan_data) {
			res = 32;
			fprintf(err, "部首外画数が Unihan に認められていない: "
				     "%s (%d)\n",
				ke->c, residual_stroke_count(ke));
		}
		key = smallest_matching;
		if (!flags.quiet) {
			if (!flags.db_out)
				output_char_line(ke, si);
			else
				output_db_line(ke, prev_si, si);
		}

		prev_si = si;

		if (sort_key_cmp(&smallest_matching, &min_key->key) < 0)
			min_key->key = smallest_matching;
		if ((min_key->ki & 0xffff) == 0xffff)
			min_key->ki = rsc[i];
	}

	for (i = 0; i < largest_rsc_sort_key(); i++) {
		struct kanji_entry const *k = kanji_db() + min_keys[i].ki;

		if (min_keys[i].ki >= 0xffff)
			DIE(0, "並べ替えキー %zu に該当する漢字がない?", i);

		if (i > 0)
			adjust_consecutive_key_info(
				&min_keys[i-1].key, &min_keys[i].key, k->c);

		if (flags.residual_stroke_counts)
			fprintf(out, "%d, /* %s */\n",
				min_keys[i].key.strokes, k->c);
	}

cleanup:
	FREE(min_keys);

	return res;
}

int check_kanji_db_order(char const *const *argv, int argc)
{
	FILE *db_stream = NULL;
	int res = 0;
	char line[512];

	memset(&flags, 0, sizeof(flags));

	while (argc > 0 && argv[0][0] == '-') {
		const char *arg = argv[0];
		argc--;
		argv++;
		if (!strcmp(arg, "-q")) {
			flags.quiet = 1;
		} else if (!strcmp(arg, "--db-out")) {
			flags.db_out = 1;
		} else if (!strcmp(arg, "--residual-stroke-counts")) {
			flags.residual_stroke_counts = 1;
		} else if (!strcmp(arg, "--")) {
			break;
		} else {
			fprintf(err, "フラグを認識できませんでした：%s\n",
				arg);
			return 3;
		}
	}

	db_stream = xfopen(
		"../third_party/Unihan_RadicalStrokeCounts.txt", "r");

	while (!res && fgets(line, sizeof(line), db_stream)) {
		res = process_rsc_line(line);
	}

	/*
	 * 部首の「⺍」はkRSAdobe_Japan1_6辞書に入ってないため、手動で定義する。
	 */
	GROW_ARRAY_BY(sort_infos, 1);
	strncpy(last_si()->c, "⺍", sizeof(last_si()->c));
	add_key(0x1e, 0x00);
	GROW_ARRAY_BY(sort_infos, 1);
	strncpy(last_si()->c, "夠", sizeof(last_si()->c));
	add_key(0x25, 0x08);
	GROW_ARRAY_BY(sort_infos, 1);
	strncpy(last_si()->c, "值", sizeof(last_si()->c));
	add_key(0x09, 0x08);
	GROW_ARRAY_BY(sort_infos, 1);
	strncpy(last_si()->c, "查", sizeof(last_si()->c));
	add_key(0x4c, 0x05);
	GROW_ARRAY_BY(sort_infos, 1);
	strncpy(last_si()->c, "跺", sizeof(last_si()->c));
	add_key(0x9e, 0x06);

	XFCLOSE(db_stream);

	QSORT(, sort_infos.el, sort_infos.cnt,
	      strcmp(sort_infos.el[a].c, sort_infos.el[b].c) < 0);
	fprintf(err, "%zu字の並べ替えキーを読み込み済み\n", sort_infos.cnt);

	if (!res)
		res = check_order();

	DESTROY_ARRAY(sort_infos);

	return res;
}
