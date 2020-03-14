#include <string.h>

#include "kanji_db.h"
#include "mapping_util.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

int code_cmp(char const *a, char const *b)
{
	size_t a_len = strlen(a);
	size_t b_len = strlen(b);

	if (a_len != b_len)
		return a_len < b_len ? -1 : 1;

	return strcmp(a, b);
}

static int conflicts(struct key_mapping const *a, struct key_mapping const *b)
{
	size_t a_len = strlen(a->orig);
	size_t b_len = strlen(b->orig);

	if (strncmp(a->orig, b->orig, a_len < b_len ? a_len : b_len))
		return 0;

	fputs("コード衝突: ", err);
	print_mapping(a, err);
	fputs(" と ", err);
	print_mapping(b, err);
	fputc('\n', err);

	return 1;
}

int sort_and_validate_no_conflicts(struct key_mapping_array *arr)
{
	int i;
	int error = 0;

	QSORT(, arr->el, arr->cnt,
	      code_cmp(arr->el[a].orig, arr->el[b].orig) < 0);

	/*
	 * code_cmp は長さの異なるコードを一緒に並べ替えないので、隣接のコードを
	 * チェックするだけで不充分です。O(n^2)で全てのコードを他の全コードと対
	 * にして比較します。
	 */
	for (i = 0; i < arr->cnt; i++) {
		int j;
		for (j = i + 1; j < arr->cnt; j++) {
			if (i != j && conflicts(arr->el + i, arr->el + j))
				error = 1;
		}
	}

	return error;
}

void append_mapping(
	struct key_mapping_array *a, char const *orig, char const *conv)
{
	GROW_ARRAY_BY(*a, 1);
	strcpy(a->el[a->cnt - 1].orig, orig);
	strcpy(a->el[a->cnt - 1].conv, conv);
}

void print_mapping(struct key_mapping const *m, FILE *stream)
{
	fprintf(stream, "%s->%s", m->orig, m->conv);
}

int incomplete_code_is_prefix(
	struct key_mapping_array const *mapping, char const *incomplete_code)
{
	int try_len;

	for (try_len = strlen(incomplete_code) + 1; try_len <= sizeof(Orig) - 1;
	     try_len++) {
		int match = incomplete_code_is_prefix_for_code_len(
			mapping, incomplete_code, try_len);
		if (match)
			return 1;
	}

	return 0;
}

int incomplete_code_is_prefix_for_code_len(
	struct key_mapping_array const *mapping,
	char const *incomplete_code,
	int len)
{
	int so_far_len = strlen(incomplete_code);
	ssize_t extended_i;
	int i;
	Orig extended_input = {0};
	Orig *found;

	if (so_far_len > sizeof(Orig) - 2)
		DIE(0, "未確定コードの長さが範囲外: '%s'", incomplete_code);
	if (len < so_far_len || len > sizeof(Orig) - 1)
		DIE(0, "len が範囲外: %d (%s)", len, incomplete_code);

	memcpy(extended_input, incomplete_code, so_far_len);
	for (i = so_far_len; i < len; i++)
		extended_input[i] = 1;

	BSEARCH_INDEX(extended_i, mapping->cnt,,
		      code_cmp(mapping->el[extended_i].orig,
			       extended_input));

	if (extended_i >= 0)
		DIE(0, "一致するコードはあるはずありません：%s",
		    incomplete_code);
	extended_i = ~extended_i;

	if (extended_i >= mapping->cnt)
		return 0;

	found = &mapping->el[extended_i].orig;
	return !strncmp(*found, incomplete_code, so_far_len) &&
		strlen(*found) == len;
}

static unsigned initial_lowest_rsc_for_shifted_key_code(
	struct key_mapping_array const *m, int first_key_index)
{
	char shifted_key_char =
		KEY_INDEX_TO_CHAR_MAP[first_key_index + KANJI_KEY_COUNT];
	char find[2] = {shifted_key_char, 0};
	struct key_mapping const *e;
	struct kanji_entry const *ke;

	BSEARCH(e, m->el, m->cnt, code_cmp(e->orig, find));
	if (!e)
		return 0xffff;

	ke = kanji_db_lookup(e->conv);
	if (!ke)
		return 0xffff;

	return kanji_db_rsc_index(ke);
}

static unsigned first_index_to_search_for_low_rsc(
	struct key_mapping_array const *m, char key_char)
{
	char find[3] = {key_char, '\001', 0};
	long i;

	BSEARCH_INDEX(i, m->cnt, , code_cmp(m->el[i].orig, find));

	return ~i;
}

unsigned lowest_rsc_index_for_codes_with_first_key(
	struct key_mapping_array const *m, int first_key_index)
{
	unsigned lowest =
		initial_lowest_rsc_for_shifted_key_code(m, first_key_index);
	char key_char = KEY_INDEX_TO_CHAR_MAP[first_key_index];
	unsigned i = first_index_to_search_for_low_rsc(m, key_char);

	if (first_key_index < 0 || first_key_index >= KANJI_KEY_COUNT)
		DIE(0, "範囲外: %d", first_key_index);

	while (i < m->cnt && m->el[i].orig[0] == key_char) {
		struct kanji_entry const *e = kanji_db_lookup(m->el[i].conv);
		if (e) {
			unsigned curr = kanji_db_rsc_index(e);
			if (curr < lowest)
				lowest = curr;
		}
		i++;
	}

	if (lowest == 0xffff)
		DIE(0, "%c (%d) で始まる入力コードがない",
		    key_char, first_key_index);

	return lowest;
}

static int hand(char a, int six_is_rh)
{
	if (!six_is_rh && a == '6')
		return 0;
	return (char_to_key_index_or_die(a) / 5) % 2;
}

static int is_alt_hands(char first, char second, int six_is_rh)
{
	return hand(first, six_is_rh) != hand(second, six_is_rh);
}

static uint8_t COLUMN_VALUE[] = {3, 2, 1, 0, 4,    4, 0, 1, 2, 3};

static int column_value(int key_index)
{
	if (key_index == char_to_key_index_or_die('6'))
		return 5;
	return COLUMN_VALUE[key_index % 10];
}

int ergonomic_lt_same_first_key(
	char first_key, char second_a, char second_b, int six_is_rh)
{
	ssize_t sec_key_i_a, sec_key_i_b;
	ssize_t sec_key_row_a, sec_key_row_b;
	int alt_hands_a, alt_hands_b;
	int column_val_a, column_val_b;

	if (second_a == second_b)
		DIE(0, "２つのキーコードが同値であるのは疑しい: %c%c",
		    first_key, second_a);

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
	 *     e. 5TGB YHN     列
	 *     f. 6
	 */

	/* 1. prefer shorter input codes */
	if (!second_a || !second_b)
		return !second_a;

	/* 2. prefer alternating hands */
	alt_hands_a = is_alt_hands(first_key, second_a, six_is_rh);
	alt_hands_b = is_alt_hands(first_key, second_b, six_is_rh);
	if (alt_hands_a != alt_hands_b)
		return alt_hands_a;

	/* 3. sort by row of second key */
	sec_key_i_a = char_to_key_index_or_die(second_a);
	sec_key_i_b = char_to_key_index_or_die(second_b);
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
	column_val_a = column_value(sec_key_i_a);
	column_val_b = column_value(sec_key_i_b);
	if (column_val_a == column_val_b) {
		DIE(0, "列が違うはずけれど、一緒でした: %c%c と %c%c",
		    first_key, second_a, first_key, second_b);
	}
	return column_val_a < column_val_b;
}

int ergonomic_lt(const char *a, const char *b, int six_is_rh)
{
	int first_key_cmp = char_to_key_index_or_die(a[0]) % KANJI_KEY_COUNT -
		char_to_key_index_or_die(b[0]) % KANJI_KEY_COUNT;

	if (first_key_cmp)
		return first_key_cmp < 0;

	return ergonomic_lt_same_first_key(a[0], a[1], b[1], six_is_rh);
}
