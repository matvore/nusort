#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kanji_db.h"
#include "util.h"

static const char *ADOBE_JAPAN = "\tkRSAdobe_Japan1_6";

struct sort_key {
	unsigned rad: 8;
	unsigned strokes: 8;
};

static int sort_key_cmp(struct sort_key *a, struct sort_key *b)
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

static struct sort_info sort_infos[20000];
static size_t sort_infos_nr;

void set_utf8_byte(char *d, unsigned char c) { *d = 0x80 | (c & 0x3f); }

static void decode_codepoint(char *d, const char *codepoint_str)
{
	long codepoint = strtol(codepoint_str, NULL, 16);
	if (codepoint < 0x0800) {
		fprintf(stderr, "コードポイントが対象外: %ld\n", codepoint);
		exit(20);
	}
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
	struct sort_info *si = &sort_infos[sort_infos_nr];
	for (i = 0; i < MAX_K; i++) {
		if (!si->k[i].rad) {
			si->k[i].rad = rad;
			si->k[i].strokes = so;
			return 1;
		}
	}

	fprintf(stderr, "並べ替えキーが多すぎます\n");
	return 0;
}

static struct {
	char c[5];
	unsigned rad : 8;
	unsigned strokes : 8;
} supplemental_keys[] = {
	{"屠", 0x2c, 0x09},
	{"斎", 0x43, 0x07},
	{"蒸", 0x56, 0x09},
	{"萬", 0x72, 0x07},
	{"采", 0xa5, 0x00},
	{"舎", 0x87, 0x02},
	{"舗", 0x87, 0x09},
};

static int process_rad_so_line(const char *line)
{
	char codepoint_str[6];
	int prefix_len;
	const char *cur;
	char *raw_ch = sort_infos[sort_infos_nr].c;
	size_t i;

	if (line[0] == '#' || line[0] == '\n')
		return 0;
	switch (sscanf(line, "U+%5[0-9A-F]%n", codepoint_str, &prefix_len)) {
	case 0:
		fprintf(stderr,
			"警告: 行の形式が間違っています：%s\n",
			line);
		return 0;
	case 1:
		break;
	default:
		fprintf(stderr, "予期していないエラー: %s\n",
			strerror(errno));
		return 10;
	}

	cur = line + prefix_len;
	if (strncmp(ADOBE_JAPAN, cur, strlen(ADOBE_JAPAN)))
		return 0;

	cur += strlen(ADOBE_JAPAN);
	while (1) {
		int rad, so, len;
		if (!*cur || *cur == '\n')
			break;

		switch (sscanf(cur, " %*1[CV]+%*d+%d.%*d.%d%n",
			       &rad, &so, &len)) {
		case 0:
		case 1:
			fprintf(stderr, "行の形式が間違っています: (列 %d)\n%s",
				(int)(cur - line), line);
			return 11;
		case 2:
			break;
		default:
			fprintf(stderr, "予期していないエラー: %s\n",
				strerror(errno));
			return 12;
		}
		cur += len;

		/* # 一画の部首を全て同一の部首として見なす。 */
		if (rad <= 6)
			rad = 1;
		/* 人と儿と入と八を同一の部首として見なす。*/
		if (rad >= 9 && rad <= 12)
			rad = 9;
		/* 匚と匸を同一の部首として見なす。*/
		if (rad == 23)
			rad = 22;
		/* 日と曰を同一の部首と見なす。*/
		if (rad == 73)
			rad = 72;

		if (!add_key(rad, so))
			return 13;
	}

	decode_codepoint(raw_ch, codepoint_str);
	for (i = 0; i < sizeof(supplemental_keys) / sizeof(*supplemental_keys);
	     i++) {
		if (!strcmp(supplemental_keys[i].c, raw_ch) &&
		    !add_key(supplemental_keys[i].rad,
			     supplemental_keys[i].strokes))
			return 14;
	}

	sort_infos_nr++;
	return 0;
}

static int check_order(void)
{
	struct kanji_entry const **k = xcalloc(kanji_db_nr(), sizeof(*k));
	size_t i;
	struct sort_key key = {0, 0};
	int res = 0;

	for (i = 0; i < kanji_db_nr(); i++)
		k[i] = kanji_db() + i;

	QSORT(, k, kanji_db_nr(),
	      k[a]->rad_so_sort_key < k[b]->rad_so_sort_key);

	for (i = 0; i < kanji_db_nr(); i++) {
		struct sort_key smallest_matching = {0xff, 0xff};
		struct sort_info *si;
		size_t ki;

		BSEARCH(si, sort_infos, sort_infos_nr, strcmp(si->c, k[i]->c));

		if (!si) {
			fprintf(stderr,
				"Unihanで並べ替えキーが見つかり"
				"ませんでした: %s\n",
				k[i]->c);
			res = 30;
			break;
		}

		for (ki = 0; ki < MAX_K && si->k[ki].rad; ki++) {
			if (sort_key_cmp(&si->k[ki], &key) < 0)
				continue;
			if (sort_key_cmp(&si->k[ki], &smallest_matching) < 0)
				smallest_matching = si->k[ki];
		}
		if (smallest_matching.rad == 0xff) {
			fprintf(stderr, "err: %s\n", k[i]->c);
			res = 31;
			break;
		}
		key = smallest_matching;
		printf("%s\t%02x%02x\n",
		       k[i]->c, (int) key.rad, (int) key.strokes);
	}

	free(k);

	return res;
}

int check_kanji_db_order(void)
{
	char rad_so_db_path[512];
	int size = snprintf(
		rad_so_db_path, sizeof(rad_so_db_path),
		"%s/Desktop/Unihan/Unihan_RadicalStrokeCounts.txt",
		getenv("HOME"));
	FILE *db_stream = NULL;
	int res = 0;
	char line[512];

	if (size >= sizeof(rad_so_db_path)) {
		fprintf(stderr, "path too long\n");
		return 1;
	}
	if (!(db_stream = fopen(rad_so_db_path, "r"))) {
		fprintf(stderr, "could not open %s\n:\t%s\n",
			rad_so_db_path, strerror(errno));
		return 2;
	}
	while (fgets(line, sizeof(line), db_stream)) {
		int res = process_rad_so_line(line);
		if (res)
			break;
	}
	if (errno) {
		fprintf(stderr, "error reading %s:\t%s\n", rad_so_db_path,
			strerror(errno));
		res = 3;
	}
	if (fclose(db_stream)) {
		fprintf(stderr,
			"WARNING: did not close %s properly\n:\t%s\n",
			rad_so_db_path, strerror(errno));
	}

	QSORT(, sort_infos, sort_infos_nr,
	      strcmp(sort_infos[a].c, sort_infos[b].c) < 0);
	fprintf(stderr, "%ld字の並べ替えキーを読み込み済み\n", sort_infos_nr);

	if (!res)
		res = check_order();

	return res;
}
