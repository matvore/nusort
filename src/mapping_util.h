#include <stdio.h>

/* ヌル終端の入力コード */
typedef char Orig[4];
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
 * 衝突の時は err ストリームにメッセージを出力して、０を返す。衝突がない時は０
 * ではない値を返す。
 */
int sort_and_validate_no_conflicts(struct key_mapping_array *);
