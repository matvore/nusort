#ifndef MAPPING_UTIL_H
#define MAPPING_UTIL_H

#include <stdio.h>

#define KANJI_KEYS_ROW_0 12
#define KANJI_KEYS_ROW_1 12
#define KANJI_KEYS_ROW_2 11
#define KANJI_KEYS_ROW_3 10

#define KANJI_KEYS_ROWS_01 (KANJI_KEYS_ROW_0 + KANJI_KEYS_ROW_1)
#define KANJI_KEYS_ROWS_012 (KANJI_KEYS_ROWS_01 + KANJI_KEYS_ROW_2)
#define KANJI_KEY_COUNT (KANJI_KEYS_ROWS_012 + KANJI_KEYS_ROW_3)

#define MAPPABLE_CHAR_COUNT (KANJI_KEY_COUNT * 2)

/* ヌル終端の入力コード */
typedef char Orig[5];
typedef char Conv[7];

struct key_mapping {
	Orig orig;
	Conv conv;
};

struct key_mapping_array {
	struct key_mapping *el;
	size_t cnt;
	size_t alloc;
};

/*
 * ２つの入力コードを比較します。
 *
 * 先に長さを比較して、異なっていれば長い方を大きく標価します。
 * 長さが同じ時は strcmp でコードポイントを比較します。
 */
int code_cmp(char const *a, char const *b);

void append_mapping(
	struct key_mapping_array *, char const *orig, char const *conv);

void print_mapping(struct key_mapping const *, FILE *);

/*
 * マッピング配列で衝突がないか確認します。衝突とは、入力コードが同じか入力
 * コードの１つが別のコードのプレフィックスであることです。
 *
 * 衝突の時は err ストリームにメッセージを出力して、非０を返す。衝突がない時は０
 * 返す。
 */
int sort_and_validate_no_conflicts(struct key_mapping_array *);

/* incomplete_code を接頭辞として持つコードが存在するかどうか検出します。 */
struct key_mapping const *incomplete_code_is_prefix(
	struct key_mapping_array const *, char const *incomplete_code);
struct key_mapping const *incomplete_code_is_prefix_for_code_len(
	struct key_mapping_array const *, char const *incomplete_code, int len);

/*
 * 長さが2で一打けん目が first_key_index または長さが1でコードが
 * first_key_index のシフトされた字の入力できるかん字の中から最低の部首+画数
 * インデックスを返す。
 */
unsigned lowest_rsc_index_for_codes_with_first_key(
	struct key_mapping_array const *, int first_key_index);

/* a が b より打ちやすい場合は truthy を返す */
int ergonomic_lt(const char *a, const char *b, int six_is_rh);
int ergonomic_lt_same_first_key(
	char first_key, char second_a, char second_b, int six_is_rh);

int is_central_kanji_char(char c);

#endif
