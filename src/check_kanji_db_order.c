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
	{"菐", 0x8d, 0x08},
	{"鼡", 0x1e, 0x05},
	{"単", 0x1e, 0x06},
	{"巣", 0x1e, 0x08},
	{"営", 0x1e, 0x09},
	{"厳", 0x1e, 0x0e},
	{"塭", 0x21, 0x0a},
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
			continue;
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

static void addkj(const char *c)
{
	GROW_ARRAY_BY(sort_infos, 1);
	strncpy(last_si()->c, c, sizeof(last_si()->c));
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

	/* kRSAdobe_Japan1_6辞書に入ってない字を手動で定義する。*/
	addkj("值"); add_key(0x09, 0x08);
	addkj("倕"); add_key(0x09, 0x08);
	addkj("倠"); add_key(0x09, 0x08);
	addkj("俷"); add_key(0x09, 0x08);
	addkj("俴"); add_key(0x09, 0x08);
	addkj("倷"); add_key(0x09, 0x08);
	addkj("刜"); add_key(0x12, 0x05);
	addkj("刞"); add_key(0x12, 0x05);
	addkj("厬"); add_key(0x1b, 0x0c);
	addkj("厹"); add_key(0x1c, 0x02);
	addkj("⺍"); add_key(0x1e, 0x00);
	addkj("叼"); add_key(0x1f, 0x02);
	addkj("叻"); add_key(0x1f, 0x02);
	addkj("叴"); add_key(0x1f, 0x02);
	addkj("吷"); add_key(0x1f, 0x04);
	addkj("呔"); add_key(0x1f, 0x04);
	addkj("吰"); add_key(0x1f, 0x04);
	addkj("唬"); add_key(0x1f, 0x08);
	addkj("啃"); add_key(0x1f, 0x08);
	addkj("啪"); add_key(0x1f, 0x08);
	addkj("啕"); add_key(0x1f, 0x08);
	addkj("唷"); add_key(0x1f, 0x08);
	addkj("唴"); add_key(0x1f, 0x08);
	addkj("啒"); add_key(0x1f, 0x08);
	addkj("啵"); add_key(0x1f, 0x08);
	addkj("啶"); add_key(0x1f, 0x08);
	addkj("啢"); add_key(0x1f, 0x08);
	addkj("啥"); add_key(0x1f, 0x08);
	addkj("唰"); add_key(0x1f, 0x08);
	addkj("啎"); add_key(0x1f, 0x08);
	addkj("啷"); add_key(0x1f, 0x09);
	addkj("嗡"); add_key(0x1f, 0x0a);
	addkj("嗥"); add_key(0x1f, 0x0a);
	addkj("嗦"); add_key(0x1f, 0x0a);
	addkj("嗨"); add_key(0x1f, 0x0a);
	addkj("嗯"); add_key(0x1f, 0x0a);
	addkj("喍"); add_key(0x1f, 0x0a);
	addkj("嗀"); add_key(0x1f, 0x0a);
	addkj("嗈"); add_key(0x1f, 0x0a);
	addkj("嗊"); add_key(0x1f, 0x0a);
	addkj("嗏"); add_key(0x1f, 0x0a);
	addkj("嗐"); add_key(0x1f, 0x0a);
	addkj("嗕"); add_key(0x1f, 0x0a);
	addkj("嗖"); add_key(0x1f, 0x0a);
	addkj("嗡"); add_key(0x1f, 0x0a);
	addkj("嗥"); add_key(0x1f, 0x0a);
	addkj("嗦"); add_key(0x1f, 0x0a);
	addkj("嗧"); add_key(0x1f, 0x0a);
	addkj("嗨"); add_key(0x1f, 0x0a);
	addkj("嗯"); add_key(0x1f, 0x0a);
	addkj("嗲"); add_key(0x1f, 0x0a);
	addkj("塓"); add_key(0x21, 0x0a);
	addkj("塭"); add_key(0x21, 0x0a);
	addkj("塯"); add_key(0x21, 0x0a);
	addkj("墣"); add_key(0x21, 0x0c);
	addkj("墬"); add_key(0x21, 0x0c);
	addkj("墯"); add_key(0x21, 0x0c);
	addkj("夠"); add_key(0x25, 0x08);
	addkj("孓"); add_key(0x28, 0x00);
	addkj("怋"); add_key(0x3e, 0x05);
	addkj("怬"); add_key(0x3e, 0x05);
	addkj("怮"); add_key(0x3e, 0x05);
	addkj("怴"); add_key(0x3e, 0x05);
	addkj("怷"); add_key(0x3e, 0x05);
	addkj("怲"); add_key(0x3e, 0x05);
	addkj("怹"); add_key(0x3e, 0x05);
	addkj("怞"); add_key(0x3e, 0x05);
	addkj("惾"); add_key(0x3e, 0x09);
	addkj("愅"); add_key(0x3e, 0x09);
	addkj("愣"); add_key(0x3e, 0x09);
	addkj("拋"); add_key(0x41, 0x05);
	addkj("抪"); add_key(0x41, 0x05);
	addkj("抭"); add_key(0x41, 0x05);
	addkj("抮"); add_key(0x41, 0x05);
	addkj("抴"); add_key(0x41, 0x05);
	addkj("揍"); add_key(0x41, 0x09);
	addkj("搌"); add_key(0x41, 0x0a);
	addkj("搕"); add_key(0x41, 0x0a);
	addkj("搚"); add_key(0x41, 0x0a);
	addkj("搟"); add_key(0x41, 0x0a);
	addkj("搣"); add_key(0x41, 0x0a);
	addkj("搧"); add_key(0x41, 0x0a);
	addkj("搫"); add_key(0x41, 0x0a);
	addkj("搳"); add_key(0x41, 0x0a);
	addkj("搷"); add_key(0x41, 0x0a);
	addkj("搹"); add_key(0x41, 0x0a);
	addkj("摀"); add_key(0x41, 0x0a);
	addkj("摁"); add_key(0x41, 0x0a);
	addkj("摃"); add_key(0x41, 0x0a);
	addkj("搋"); add_key(0x41, 0x0a);
	addkj("啟"); add_key(0x43, 0x07);
	addkj("朹"); add_key(0x4c, 0x02);
	addkj("朻"); add_key(0x4c, 0x02);
	addkj("朼"); add_key(0x4c, 0x02);
	addkj("查"); add_key(0x4c, 0x05);
	addkj("棳"); add_key(0x4c, 0x08);
	addkj("棸"); add_key(0x4c, 0x08);
	addkj("椔"); add_key(0x4c, 0x08);
	addkj("槬"); add_key(0x4c, 0x0b);
	addkj("槷"); add_key(0x4c, 0x0b);
	addkj("槸"); add_key(0x4c, 0x0b);
	addkj("樆"); add_key(0x4c, 0x0b);
	addkj("槤"); add_key(0x4c, 0x0b);
	addkj("樧"); add_key(0x4c, 0x0b);
	addkj("汦"); add_key(0x56, 0x04);
	addkj("汩"); add_key(0x56, 0x04);
	addkj("汱"); add_key(0x56, 0x04);
	addkj("沋"); add_key(0x56, 0x04);
	addkj("沏"); add_key(0x56, 0x04);
	addkj("涳"); add_key(0x56, 0x08);
	addkj("涾"); add_key(0x56, 0x08);
	addkj("淔"); add_key(0x56, 0x08);
	addkj("淜"); add_key(0x56, 0x08);
	addkj("淭"); add_key(0x56, 0x08);
	addkj("湴"); add_key(0x56, 0x08);
	addkj("渃"); add_key(0x56, 0x08);
	addkj("渜"); add_key(0x56, 0x09);
	addkj("渱"); add_key(0x56, 0x09);
	addkj("渳"); add_key(0x56, 0x09);
	addkj("渽"); add_key(0x56, 0x09);
	addkj("渿"); add_key(0x56, 0x09);
	addkj("湀"); add_key(0x56, 0x09);
	addkj("湁"); add_key(0x56, 0x09);
	addkj("湆"); add_key(0x56, 0x09);
	addkj("湇"); add_key(0x56, 0x09);
	addkj("湠"); add_key(0x56, 0x09);
	addkj("湡"); add_key(0x56, 0x09);
	addkj("湩"); add_key(0x56, 0x09);
	addkj("溈"); add_key(0x56, 0x09);
	addkj("湱"); add_key(0x56, 0x09);
	addkj("渨"); add_key(0x56, 0x09);
	addkj("渰"); add_key(0x56, 0x09);
	addkj("碄"); add_key(0x71, 0x08);
	addkj("碅"); add_key(0x71, 0x08);
	addkj("碉"); add_key(0x71, 0x08);
	addkj("萐"); add_key(0x8d, 0x08);
	addkj("菆"); add_key(0x8d, 0x08);
	addkj("菂"); add_key(0x8d, 0x08);
	addkj("菈"); add_key(0x8d, 0x08);
	addkj("菋"); add_key(0x8d, 0x08);
	addkj("菕"); add_key(0x8d, 0x08);
	addkj("菙"); add_key(0x8d, 0x08);
	addkj("菞"); add_key(0x8d, 0x08);
	addkj("菤"); add_key(0x8d, 0x08);
	addkj("菧"); add_key(0x8d, 0x08);
	addkj("菳"); add_key(0x8d, 0x08);
	addkj("菵"); add_key(0x8d, 0x08);
	addkj("菺"); add_key(0x8d, 0x08);
	addkj("菿"); add_key(0x8d, 0x08);
	addkj("萉"); add_key(0x8d, 0x08);
	addkj("萒"); add_key(0x8d, 0x08);
	addkj("萣"); add_key(0x8d, 0x08);
	addkj("萭"); add_key(0x8d, 0x09);
	addkj("萷"); add_key(0x8d, 0x09);
	addkj("萺"); add_key(0x8d, 0x09);
	addkj("萿"); add_key(0x8d, 0x09);
	addkj("葀"); add_key(0x8d, 0x09);
	addkj("葂"); add_key(0x8d, 0x09);
	addkj("葃"); add_key(0x8d, 0x09);
	addkj("葋"); add_key(0x8d, 0x09);
	addkj("葌"); add_key(0x8d, 0x09);
	addkj("葝"); add_key(0x8d, 0x09);
	addkj("葞"); add_key(0x8d, 0x09);
	addkj("蒆"); add_key(0x8d, 0x09);
	addkj("蒍"); add_key(0x8d, 0x09);
	addkj("蒏"); add_key(0x8d, 0x09);
	addkj("蒏"); add_key(0x8d, 0x09);
	addkj("蒏"); add_key(0x8d, 0x09);
	addkj("蒏"); add_key(0x8d, 0x09);
	addkj("萴"); add_key(0x8d, 0x09);
	addkj("萰"); add_key(0x8d, 0x09);
	addkj("萲"); add_key(0x8d, 0x09);
	addkj("蓨"); add_key(0x8d, 0x0b);
	addkj("蓩"); add_key(0x8d, 0x0b);
	addkj("蓳"); add_key(0x8d, 0x0b);
	addkj("蓶"); add_key(0x8d, 0x0b);
	addkj("蓹"); add_key(0x8d, 0x0b);
	addkj("蓾"); add_key(0x8d, 0x0b);
	addkj("蔈"); add_key(0x8d, 0x0b);
	addkj("蔍"); add_key(0x8d, 0x0b);
	addkj("蔏"); add_key(0x8d, 0x0b);
	addkj("蔒"); add_key(0x8d, 0x0b);
	addkj("蔖"); add_key(0x8d, 0x0b);
	addkj("蔝"); add_key(0x8d, 0x0b);
	addkj("蔠"); add_key(0x8d, 0x0b);
	addkj("蔨"); add_key(0x8d, 0x0b);
	addkj("蔩"); add_key(0x8d, 0x0b);
	addkj("蔮"); add_key(0x8d, 0x0b);
	addkj("蔰"); add_key(0x8d, 0x0b);
	addkj("蔉"); add_key(0x8d, 0x0b);
	addkj("蔊"); add_key(0x8d, 0x0b);
	addkj("蔻"); add_key(0x8d, 0x0b);
	addkj("蓫"); add_key(0x8d, 0x0b);
	addkj("藸"); add_key(0x8d, 0x10);
	addkj("蘉"); add_key(0x8d, 0x10);
	addkj("蘁"); add_key(0x8d, 0x10);
	addkj("蜠"); add_key(0x8f, 0x08);
	addkj("蜤"); add_key(0x8f, 0x08);
	addkj("蜦"); add_key(0x8f, 0x08);
	addkj("蜧"); add_key(0x8f, 0x08);
	addkj("蜪"); add_key(0x8f, 0x08);
	addkj("蜬"); add_key(0x8f, 0x08);
	addkj("蜭"); add_key(0x8f, 0x08);
	addkj("蜰"); add_key(0x8f, 0x08);
	addkj("蜳"); add_key(0x8f, 0x08);
	addkj("蝁"); add_key(0x8f, 0x08);
	addkj("蝂"); add_key(0x8f, 0x08);
	addkj("蜛"); add_key(0x8f, 0x08);
	addkj("蜸"); add_key(0x8f, 0x09);
	addkj("蜵"); add_key(0x8f, 0x09);
	addkj("蠫"); add_key(0x8f, 0x0f);
	addkj("蠩"); add_key(0x8f, 0x10);
	addkj("蠪"); add_key(0x8f, 0x10);
	addkj("蠬"); add_key(0x8f, 0x10);
	addkj("蠥"); add_key(0x8f, 0x10);
	addkj("蠦"); add_key(0x8f, 0x10);
	addkj("裮"); add_key(0x92, 0x08);
	addkj("裶"); add_key(0x92, 0x08);
	addkj("裺"); add_key(0x92, 0x08);
	addkj("裻"); add_key(0x92, 0x08);
	addkj("覢"); add_key(0x94, 0x08);
	addkj("覣"); add_key(0x94, 0x08);
	addkj("覤"); add_key(0x94, 0x08);
	addkj("觾"); add_key(0x95, 0x10);
	addkj("讆"); add_key(0x96, 0x10);
	addkj("賮"); add_key(0x9b, 0x09);
	addkj("跺"); add_key(0x9e, 0x06);
	addkj("邧"); add_key(0xa4, 0x04);
	addkj("邞"); add_key(0xa4, 0x04);
	addkj("邟"); add_key(0xa4, 0x04);
	addkj("鈏"); add_key(0xa8, 0x04);
	addkj("鈃"); add_key(0xa8, 0x04);
	addkj("鈚"); add_key(0xa8, 0x04);
	addkj("镺"); add_key(0xa9, 0x04);
	addkj("阰"); add_key(0xab, 0x04);
	addkj("雗"); add_key(0xad, 0x0a);

	XFCLOSE(db_stream);

	QSORT(, sort_infos.el, sort_infos.cnt,
	      strcmp(sort_infos.el[a].c, sort_infos.el[b].c) < 0);
	fprintf(err, "%zu字の並べ替えキーを読み込み済み\n", sort_infos.cnt);

	if (!res)
		res = check_order();

	DESTROY_ARRAY(sort_infos);

	return res;
}
