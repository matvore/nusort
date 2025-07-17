#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands.h"
#include "kanji_db.h"
#include "radicals.h"
#include "residual_stroke_count.h"
#include "streams.h"
#include "util.h"

static const char *ADOBE_JAPAN = "\tkRSAdobe_Japan1_6";

static struct {
	unsigned quiet : 1;
	unsigned allkeyout : 1;
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
	{"值", 0x09, 0x08},
	{"倕", 0x09, 0x08},
	{"倠", 0x09, 0x08},
	{"俷", 0x09, 0x08},
	{"俴", 0x09, 0x08},
	{"倷", 0x09, 0x08},
	{"刜", 0x12, 0x05},
	{"刞", 0x12, 0x05},
	{"厬", 0x1b, 0x0c},
	{"厹", 0x1c, 0x02},
	{"⺍", 0x1e, 0x00},
	{"鼡", 0x1e, 0x05},
	{"単", 0x1e, 0x06},
	{"巣", 0x1e, 0x08},
	{"営", 0x1e, 0x09},
	{"厳", 0x1e, 0x0e},
	{"叼", 0x1f, 0x02},
	{"叻", 0x1f, 0x02},
	{"叴", 0x1f, 0x02},
	{"吷", 0x1f, 0x04},
	{"呔", 0x1f, 0x04},
	{"吰", 0x1f, 0x04},
	{"唬", 0x1f, 0x08},
	{"啃", 0x1f, 0x08},
	{"啪", 0x1f, 0x08},
	{"啕", 0x1f, 0x08},
	{"唷", 0x1f, 0x08},
	{"唴", 0x1f, 0x08},
	{"啒", 0x1f, 0x08},
	{"啵", 0x1f, 0x08},
	{"啶", 0x1f, 0x08},
	{"啢", 0x1f, 0x08},
	{"啥", 0x1f, 0x08},
	{"唰", 0x1f, 0x08},
	{"啎", 0x1f, 0x08},
	{"啷", 0x1f, 0x09},
	{"嗹", 0x1f, 0x0a},
	{"嗦", 0x1f, 0x0a},
	{"喍", 0x1f, 0x0a},
	{"嗀", 0x1f, 0x0a},
	{"嗈", 0x1f, 0x0a},
	{"嗊", 0x1f, 0x0a},
	{"嗏", 0x1f, 0x0a},
	{"嗐", 0x1f, 0x0a},
	{"嗕", 0x1f, 0x0a},
	{"嗖", 0x1f, 0x0a},
	{"嗡", 0x1f, 0x0a},
	{"嗥", 0x1f, 0x0a},
	{"嗧", 0x1f, 0x0a},
	{"嗨", 0x1f, 0x0a},
	{"嗯", 0x1f, 0x0a},
	{"嗲", 0x1f, 0x0a},
	{"圞", 0x20, 0x17},
	{"塭", 0x21, 0x0a},
	{"塓", 0x21, 0x0a},
	{"塭", 0x21, 0x0a},
	{"塯", 0x21, 0x0a},
	{"墣", 0x21, 0x0c},
	{"墬", 0x21, 0x0c},
	{"墯", 0x21, 0x0c},
	{"夠", 0x25, 0x08},
	{"孓", 0x28, 0x00},
	{"屠", 0x2d, 0x09},
	{"岤", 0x2f, 0x05},
	{"岮", 0x2f, 0x05},
	{"岯", 0x2f, 0x05},
	{"巔", 0x2f, 0x13},
	{"帗", 0x33, 0x05},
	{"庛", 0x36, 0x06},
	{"怋", 0x3e, 0x05},
	{"怬", 0x3e, 0x05},
	{"怮", 0x3e, 0x05},
	{"怴", 0x3e, 0x05},
	{"怷", 0x3e, 0x05},
	{"怲", 0x3e, 0x05},
	{"怹", 0x3e, 0x05},
	{"怞", 0x3e, 0x05},
	{"惾", 0x3e, 0x09},
	{"愅", 0x3e, 0x09},
	{"愣", 0x3e, 0x09},
	{"慥", 0x3e, 0x0a},
	{"拋", 0x41, 0x05},
	{"抪", 0x41, 0x05},
	{"抭", 0x41, 0x05},
	{"抮", 0x41, 0x05},
	{"抴", 0x41, 0x05},
	{"揍", 0x41, 0x09},
	{"搌", 0x41, 0x0a},
	{"搕", 0x41, 0x0a},
	{"搚", 0x41, 0x0a},
	{"搟", 0x41, 0x0a},
	{"搣", 0x41, 0x0a},
	{"搧", 0x41, 0x0a},
	{"搫", 0x41, 0x0a},
	{"搳", 0x41, 0x0a},
	{"搷", 0x41, 0x0a},
	{"搹", 0x41, 0x0a},
	{"摀", 0x41, 0x0a},
	{"摁", 0x41, 0x0a},
	{"摃", 0x41, 0x0a},
	{"搋", 0x41, 0x0a},
	{"撻", 0x41, 0x0c},
	{"攆", 0x41, 0x0f},
	{"擸", 0x41, 0x0f},
	{"啟", 0x43, 0x07},
	{"斎", 0x44, 0x07},
	{"朹", 0x4c, 0x02},
	{"朻", 0x4c, 0x02},
	{"朼", 0x4c, 0x02},
	{"𣏤", 0x4c, 0x04},
	{"柴", 0x4c, 0x05},
	{"查", 0x4c, 0x05},
	{"棳", 0x4c, 0x08},
	{"棸", 0x4c, 0x08},
	{"椔", 0x4c, 0x08},
	{"槤", 0x4c, 0x0a},
	{"槬", 0x4c, 0x0b},
	{"槷", 0x4c, 0x0b},
	{"槸", 0x4c, 0x0b},
	{"樆", 0x4c, 0x0b},
	{"樧", 0x4c, 0x0b},
	{"汦", 0x56, 0x04},
	{"汩", 0x56, 0x04},
	{"汱", 0x56, 0x04},
	{"沋", 0x56, 0x04},
	{"沏", 0x56, 0x04},
	{"涳", 0x56, 0x08},
	{"涾", 0x56, 0x08},
	{"淔", 0x56, 0x08},
	{"淜", 0x56, 0x08},
	{"淭", 0x56, 0x08},
	{"湴", 0x56, 0x08},
	{"渃", 0x56, 0x08},
	{"涭", 0x56, 0x08},
	{"渜", 0x56, 0x09},
	{"渱", 0x56, 0x09},
	{"渳", 0x56, 0x09},
	{"渽", 0x56, 0x09},
	{"渿", 0x56, 0x09},
	{"湀", 0x56, 0x09},
	{"湁", 0x56, 0x09},
	{"湆", 0x56, 0x09},
	{"湇", 0x56, 0x09},
	{"湠", 0x56, 0x09},
	{"湡", 0x56, 0x09},
	{"湩", 0x56, 0x09},
	{"溈", 0x56, 0x09},
	{"湱", 0x56, 0x09},
	{"渨", 0x56, 0x09},
	{"渰", 0x56, 0x09},
	{"溲", 0x56, 0x09},
	{"灨", 0x56, 0x18},
	{"灪", 0x56, 0x1d},
	{"蒸", 0x57, 0x09},
	{"燧", 0x57, 0x0c},
	{"燵", 0x57, 0x0c},
	{"爨", 0x57, 0x1a},
	{"牄", 0x5b, 0x0a},
	{"牬", 0x5e, 0x05},
	{"犕", 0x5e, 0x0a},
	{"犗", 0x5e, 0x0a},
	{"犿", 0x5f, 0x04},
	{"犽", 0x5f, 0x05},
	{"獀", 0x5f, 0x09}, {"獀", 0x5f, 0x0a},
	{"猼", 0x5f, 0x0a},
	{"獊", 0x5f, 0x0a},
	{"獂", 0x5f, 0x0a},
	{"玤", 0x61, 0x04},
	{"玡", 0x61, 0x05},
	{"瑵", 0x61, 0x0a},
	{"瓬", 0x63, 0x04},
	{"甈", 0x63, 0x0a},
	{"甭", 0x66, 0x04},
	{"瘦", 0x69, 0x09},
	{"癵", 0x69, 0x19},
	{"矓", 0x6e, 0x10},
	{"碄", 0x71, 0x08},
	{"碅", 0x71, 0x08},
	{"碉", 0x71, 0x08},
	{"萬", 0x73, 0x07},
	{"縋", 0x79, 0x09},
	{"縺", 0x79, 0x0a},
	{"繲", 0x79, 0x0d},
	{"繴", 0x79, 0x0d},
	{"繶", 0x79, 0x0d},
	{"繷", 0x79, 0x0d},
	{"繺", 0x79, 0x0d},
	{"腴", 0x83, 0x08},
	{"腛", 0x83, 0x09},
	{"腞", 0x83, 0x09},
	{"腢", 0x83, 0x09},
	{"腤", 0x83, 0x09},
	{"腲", 0x83, 0x09},
	{"腶", 0x83, 0x09},
	{"腜", 0x83, 0x09},
	{"膄", 0x83, 0x09},
	{"舎", 0x88, 0x02},
	{"舗", 0x88, 0x09},
	{"艐", 0x8a, 0x09},
	{"艒", 0x8a, 0x09},
	{"艓", 0x8a, 0x09},
	{"艵", 0x8c, 0x06},
	{"菐", 0x8d, 0x08},
	{"萐", 0x8d, 0x08},
	{"菆", 0x8d, 0x08},
	{"菂", 0x8d, 0x08},
	{"菈", 0x8d, 0x08},
	{"菋", 0x8d, 0x08},
	{"菕", 0x8d, 0x08},
	{"菙", 0x8d, 0x08},
	{"菞", 0x8d, 0x08},
	{"菤", 0x8d, 0x08},
	{"菧", 0x8d, 0x08},
	{"菳", 0x8d, 0x08},
	{"菵", 0x8d, 0x08},
	{"菺", 0x8d, 0x08},
	{"菿", 0x8d, 0x08},
	{"萉", 0x8d, 0x08},
	{"萒", 0x8d, 0x08},
	{"萣", 0x8d, 0x08},
	{"萭", 0x8d, 0x09},
	{"萷", 0x8d, 0x09},
	{"萺", 0x8d, 0x09},
	{"萿", 0x8d, 0x09},
	{"葀", 0x8d, 0x09},
	{"葂", 0x8d, 0x09},
	{"葃", 0x8d, 0x09},
	{"葋", 0x8d, 0x09},
	{"葌", 0x8d, 0x09},
	{"葝", 0x8d, 0x09},
	{"葞", 0x8d, 0x09},
	{"蒆", 0x8d, 0x09},
	{"蒍", 0x8d, 0x09},
	{"蒏", 0x8d, 0x09},
	{"蒏", 0x8d, 0x09},
	{"蒏", 0x8d, 0x09},
	{"蒏", 0x8d, 0x09},
	{"萴", 0x8d, 0x09},
	{"萰", 0x8d, 0x09},
	{"萲", 0x8d, 0x09},
	{"蓨", 0x8d, 0x0b},
	{"蓩", 0x8d, 0x0b},
	{"蓳", 0x8d, 0x0b},
	{"蓶", 0x8d, 0x0b},
	{"蓹", 0x8d, 0x0b},
	{"蓾", 0x8d, 0x0b},
	{"蔈", 0x8d, 0x0b},
	{"蔍", 0x8d, 0x0b},
	{"蔏", 0x8d, 0x0b},
	{"蔒", 0x8d, 0x0b},
	{"蔖", 0x8d, 0x0b},
	{"蔝", 0x8d, 0x0b},
	{"蔠", 0x8d, 0x0b},
	{"蔨", 0x8d, 0x0b},
	{"蔩", 0x8d, 0x0b},
	{"蔮", 0x8d, 0x0b},
	{"蔰", 0x8d, 0x0b},
	{"蔉", 0x8d, 0x0b},
	{"蔊", 0x8d, 0x0b},
	{"蔻", 0x8d, 0x0b},
	{"蓫", 0x8d, 0x0b},
	{"藸", 0x8d, 0x10},
	{"蘉", 0x8d, 0x10},
	{"蘁", 0x8d, 0x10},
	{"蘒", 0x8d, 0x16},
	{"虌", 0x8d, 0x19},
	{"虋", 0x8d, 0x20},
	{"蜠", 0x8f, 0x08},
	{"蜤", 0x8f, 0x08},
	{"蜦", 0x8f, 0x08},
	{"蜧", 0x8f, 0x08},
	{"蜪", 0x8f, 0x08},
	{"蜬", 0x8f, 0x08},
	{"蜭", 0x8f, 0x08},
	{"蜰", 0x8f, 0x08},
	{"蜳", 0x8f, 0x08},
	{"蝁", 0x8f, 0x08},
	{"蝂", 0x8f, 0x08},
	{"蜛", 0x8f, 0x08},
	{"蜸", 0x8f, 0x09},
	{"蜵", 0x8f, 0x09},
	{"螋", 0x8f, 0x09},
	{"蠫", 0x8f, 0x0f},
	{"蠩", 0x8f, 0x10},
	{"蠪", 0x8f, 0x10},
	{"蠬", 0x8f, 0x10},
	{"蠥", 0x8f, 0x10},
	{"蠦", 0x8f, 0x10},
	{"裮", 0x92, 0x08},
	{"裶", 0x92, 0x08},
	{"裺", 0x92, 0x08},
	{"裻", 0x92, 0x08},
	{"褪", 0x92, 0x09},
	{"覢", 0x94, 0x08},
	{"覣", 0x94, 0x08},
	{"覤", 0x94, 0x08},
	{"觾", 0x95, 0x10},
	{"譴", 0x96, 0x0d},
	{"讆", 0x96, 0x10},
	{"賮", 0x9b, 0x09},
	{"跺", 0x9e, 0x06},
	{"遳", 0xa3, 0x0a},
	{"邧", 0xa4, 0x04},
	{"邞", 0xa4, 0x04},
	{"邟", 0xa4, 0x04},
	{"鄛", 0xa4, 0x0b},
	{"鄝", 0xa4, 0x0b},
	{"鄟", 0xa4, 0x0b},
	{"鄡", 0xa4, 0x0b},
	{"鄤", 0xa4, 0x0b},
	{"醥", 0xa5, 0x0b},
	{"醧", 0xa5, 0x0b},
	{"采", 0xa6, 0x00},
	{"鈏", 0xa8, 0x04},
	{"鈃", 0xa8, 0x04},
	{"鈚", 0xa8, 0x04},
	{"鏈", 0xa8, 0x0a},
	{"鏀", 0xa8, 0x0b},
	{"鏂", 0xa8, 0x0b},
	{"鏏", 0xa8, 0x0b},
	{"鏬", 0xa8, 0x0b},
	{"鏚", 0xa8, 0x0b},
	{"镺", 0xa9, 0x04},
	{"闛", 0xaa, 0x0b},
	{"闥", 0xaa, 0x0c},
	{"阰", 0xab, 0x04},
	{"隞", 0xab, 0x0b},
	{"隨", 0xab, 0x0c},
	{"雗", 0xad, 0x0a},
	{"靆", 0xae, 0x0f},
	{"韃", 0xb2, 0x0c},
	{"颼", 0xb7, 0x09},
	{"髓", 0xbd, 0x0c},
	{"䯤", 0xbd, 0x0d},
	{"𩰤", 0xc1, 0x0a},
	{"鼀", 0xce, 0x05},
	{"鼁", 0xce, 0x05},
	{"鼇", 0xce, 0x0a},
	{"鼆", 0xce, 0x0a},
	{"鼊", 0xce, 0x0d},
};

#define SUPKEYCNT (sizeof(supplemental_keys) / sizeof(*supplemental_keys))

static int process_rsc_line(char const *line)
{
	char codepoint_str[6];
	int prefix_len;
	const char *cur;

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
	return 0;
}

static void output_all_keys(
	const struct kanji_entry *k, const struct sort_info *si)
{
	size_t ki;
	for (ki = 0; ki < MAX_K && si->k[ki].rad; ki++)
		fprintf(out, " %02x%02x",
			(int) si->k[ki].rad, (int) si->k[ki].strokes);
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
	fprintf(out, "\t{\"%s\", %5d, %d, %d, %d},",
		k->c, k->ranking, cutoff_type, k->jis_suijun, k->taiwan_kib);
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
	int radin = -1;

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

		if (ke->cutoff_type > 1) radin++;

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
			output_db_line(ke, prev_si, si);
			if (flags.allkeyout)
				output_all_keys(ke, si);
			else
				fprintf(out,	" %02x%02x",
						radical_num(radin),
						residual_stroke_count(ke));
			fputc('\n', out);
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
	}

cleanup:
	FREE(min_keys);

	return res;
}

static int consume_supkey(int *supki)
{
	char *supc;
	int supcmp;
	struct sort_info sit = {0};

	if (!sort_infos.cnt) return 0;
	while (*supki < SUPKEYCNT) {
		supc = supplemental_keys[*supki].c;
		supcmp = strcmp(supc, last_si()->c);
		if (supcmp > 0) break;
		if (supcmp < 0) {
			if (sit.c[0]) abort();
			sit = *last_si();
			memset(last_si()->k, 0, sizeof last_si()->k);
			strncpy(last_si()->c, supc, sizeof last_si()->c);
		}
		if (!add_key(	supplemental_keys[*supki].rad,
				supplemental_keys[*supki].strokes))
			return 14;
		(*supki)++;
	}
	if (sit.c[0]) {
		GROW_ARRAY_BY(sort_infos, 1);
		*last_si() = sit;
	}
	return 0;
}

int check_kanji_db_order(char const *const *argv, int argc)
{
	FILE *db_stream = NULL;
	int res, supki = 0;
	char line[512];

	memset(&flags, 0, sizeof(flags));

	while (argc > 0 && argv[0][0] == '-') {
		const char *arg = argv[0];
		argc--;
		argv++;
		if (!strcmp(arg, "-q")) {
			flags.quiet = 1;
		} else if (!strcmp(arg, "--allkeyout")) {
			flags.allkeyout = 1;
		} else if (!strcmp(arg, "--")) {
			break;
		} else {
			fprintf(err, "フラグを認識できませんでした：%s\n",
				arg);
			return 3;
		}
	}

	db_stream = xfopen("third_party/Unihan_RadicalStrokeCounts.txt", "r");

	QSORT(,	supplemental_keys, SUPKEYCNT,
		0 > strcmp(	supplemental_keys[a].c,
				supplemental_keys[b].c));
	while (fgets(line, sizeof(line), db_stream)) {
		if (res=process_rsc_line(line))	goto cleanup;
		if (res=consume_supkey(&supki))	goto cleanup;
	}

	QSORT(, sort_infos.el, sort_infos.cnt,
	      strcmp(sort_infos.el[a].c, sort_infos.el[b].c) < 0);

	fprintf(err, "%zu字の並べ替えキーを読み込み済み\n", sort_infos.cnt);

	res = check_order();

cleanup:
	XFCLOSE(db_stream);
	DESTROY_ARRAY(sort_infos);

	return res;
}
