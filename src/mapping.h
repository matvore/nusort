#ifndef MAPPING_H
#define MAPPING_H

#include "romazi.h"

struct mapping {
	struct key_mapping_array codes;
	unsigned ergonomic_sort : 1;
	unsigned include_kanji : 1;
};

/*
 * ２つの入力コードを比較します。
 *
 * 先に長さを比較して、異なっていれば長い方を大きく標価します。
 * 長さが同じ時は strcmp でコードポイントを比較します。
 */
int code_cmp(char const *a, char const *b);

void mapping_populate(struct mapping *mapping);
void mapping_destroy(struct mapping *mapping);

#endif
