#include <string.h>

#include "mapping.h"

#include "kanji_distribution.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

static int is_alt_hands(char const *a)
{
	return (char_to_key_index_or_die(a[0]) / 5) % 2 !=
	       (char_to_key_index_or_die(a[1]) / 5) % 2;
}

uint8_t COLUMN_VALUE[] = {3, 2, 1, 0, 4,	4, 0, 1, 2, 3};

static int ergonomic_lt(const char *a, const char *b)
{
	ssize_t fir_key_i_a = char_to_key_index_or_die(a[0]) % KANJI_KEY_COUNT;
	ssize_t fir_key_i_b = char_to_key_index_or_die(b[0]) % KANJI_KEY_COUNT;
	ssize_t sec_key_i_a, sec_key_i_b;
	ssize_t sec_key_row_a, sec_key_row_b;
	int alt_hands_a, alt_hands_b;
	int column_val_a, column_val_b;

	if (!strncmp(a, b, 2))
		BUG("２つのキーコードが同値であるのは疑しい: %.2s", a);

	if (fir_key_i_a != fir_key_i_b)
		return fir_key_i_a < fir_key_i_b;

	/*
	 * 並べ替え基順 (先頭に近いほど基順が優先されます。)
	 *  1. コードの長さ (シフトされた一文字が二文字のコードより打ちやすいと
	 *     します。)
	 *  2. 両手を使うコードが片手コードより打ちやすい。
	 *  3. ホームポジションに近いほど打ちやすい：
	 *     a. ASDF(ホーム) 行
	 *     b. QWER         行
	 *     c. ZXCV         行
	 *     d. 1234         行
	 *  4. ２鍵目のキーを押す指が強いほど打ちやすいけれど、人差指を左右にの
	 *     びる列はもっとも打ちづらい。
	 *     a. 4RFV 7UJM    列
	 *     b. 3EDC 8IK,    列
	 *     c. 2WSX 9OL.    列
	 *     d. 1QAZ 0P;/    列
	 *     e. 5TGB 6YHN    列
	 */

	/* 1. prefer shorter input codes */
	if (!a[1] || !b[1]) {
		if (!a[1] && !b[1])
			BUG("最初のキーが同じのに、２個のコード両方が１打鍵: "
			    "%.2s %.2s?", a, b);
		return !a[1];
	}

	/* 2. prefer alternating hands */
	alt_hands_a = is_alt_hands(a);
	alt_hands_b = is_alt_hands(b);
	if (alt_hands_a != alt_hands_b)
		return alt_hands_a;

	/* 3. sort by row of second key */
	sec_key_i_a = char_to_key_index_or_die(a[1]);
	sec_key_i_b = char_to_key_index_or_die(b[1]);
	sec_key_row_a = sec_key_i_a / 10;
	sec_key_row_b = sec_key_i_b / 10;
	if (sec_key_row_a != sec_key_row_b) {
		/* row 0 is worst */
		if (!sec_key_row_a || !sec_key_row_b)
			return !sec_key_row_b;
		/* row 3 is next worst */
		if (sec_key_row_a == 3 || sec_key_row_b == 3)
			return sec_key_row_b == 3;
		return sec_key_row_b == 1;
	}

	/* 4. sort by column */
	column_val_a = COLUMN_VALUE[sec_key_i_a % 10];
	column_val_b = COLUMN_VALUE[sec_key_i_b % 10];
	if (column_val_a == column_val_b) {
		BUG("列が違うはずけれど、一緒でした: %.2s と %.2s", a, b);
	}
	return column_val_a < column_val_b;
}

static void get_kanji_codes(struct key_mapping_array *m, int ergonomic_sort)
{
	size_t free_code;
	size_t codes_consumed[KANJI_KEY_COUNT] = {0};

	struct kanji_distribution kd = {0};
	struct short_code_array free_kanji_codes = {0};

	kanji_distribution_auto_pick_cutoff(&kd);
	kanji_distribution_populate(&kd);
	get_free_kanji_codes(&free_kanji_codes);

	/*
	 * この時点で、free_kanji_codesが既に１打鍵目を共有するコード群ごとに
	 * 使用頻度に基づいて並べ替え済みです。
	 * 更にコードを打ちやすい順に並べ替えれば、以降の論理の流れでよく使う漢
	 * 字は打ちやすいコードに割り当てられます。
	 */
	if (ergonomic_sort)
		QSORT(, free_kanji_codes.el, free_kanji_codes.cnt,
		      ergonomic_lt(free_kanji_codes.el[a],
				   free_kanji_codes.el[b]));

	for (free_code = 0; free_code < free_kanji_codes.cnt; free_code++) {
		char first_key_ch = free_kanji_codes.el[free_code][0];
		ssize_t first_key_i = char_to_key_index_or_die(first_key_ch);
		struct line_stats *line_stats;
		size_t next_code_i;

		/*
		 * 大文字を小文字にかえ、シフト文字を非シフトにかえる。
		 * (例 [: -> ;] [< -> ,])
		 */
		first_key_i %= KANJI_KEY_COUNT;
		first_key_ch = KEY_INDEX_TO_CHAR_MAP[first_key_i];

		BSEARCH(line_stats, kd.line_stats, kd.line_stats_nr,
			char_to_key_index_or_die(line_stats->key_ch) -
				first_key_i);

		if (line_stats == NULL)
			BUG("kanji_distributionで行が見つかりません：%zd",
			    first_key_i);

		next_code_i = codes_consumed[first_key_i]++;
		if (next_code_i >= line_stats->e_nr)
			BUG("kanji_distributionの１打鍵目が'%zd'のコード数が"
			    "足りません", first_key_i);
		GROW_ARRAY_BY(*m, 1);
		memcpy(m->el[m->cnt - 1].orig, free_kanji_codes.el[free_code],
		       2);
		strncpy(m->el[m->cnt - 1].conv, line_stats->e[next_code_i]->c,
			sizeof(m->el->conv));
	}

	DESTROY_ARRAY(free_kanji_codes);
}

void init_mapping_config_for_cli_flags(struct mapping_config *config)
{
	if (!bytes_are_zero(config, sizeof(*config)))
		BUG("mapping_config not initialized to zero bytes");
	config->include_kanji = 1;
}

int parse_mapping_flags(
	int *argc, char const *const **argv, struct mapping_config *config)
{
	if (!strcmp((*argv)[0], "-s")) {
		config->ergonomic_sort = 1;
		(*argv)++;
		(*argc)--;
		return 1;
	}
	if (!strcmp((*argv)[0], "--no-kanji")) {
		config->include_kanji = 0;
		(*argv)++;
		(*argc)--;
		return 1;
	}
	return 0;
}

int mapping_populate(
	struct mapping_config const *config, struct key_mapping_array *mapping)
{
	get_romazi_codes(mapping);
	if (config->include_kanji)
		get_kanji_codes(mapping, config->ergonomic_sort);

	return sort_and_validate_no_conflicts(mapping);
}
