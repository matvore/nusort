#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static const char *ADOBE_JAPAN = "\tkRSAdobe_Japan1_6";

struct sort_key {
	unsigned rad: 8;
	unsigned strokes: 8;
};

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

static int process_rad_so_line(const char *line)
{
	char codepoint_str[6];
	int prefix_len;
	const char *cur;
	struct sort_key *k = sort_infos[sort_infos_nr].k;
	size_t k_nr;

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
		if (k_nr == MAX_K) {
			fprintf(stderr, "並べ替えキーが多すぎます：%s", line);
			return 13;
		}
		k->rad = rad;
		k->strokes = so;
		k_nr++;
	}

	decode_codepoint(sort_infos[sort_infos_nr++].c, codepoint_str);

	return 0;
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
	      strcmp(sort_infos[a].c, sort_infos[b].c));
	fprintf(stderr, "%ld字の並べ替えキーを読み込み済み\n", sort_infos_nr);
	return res;
}
