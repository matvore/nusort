#ifndef MAPPING_H
#define MAPPING_H

#include <stdio.h>

#include "flags.h"
#include "kanji_distribution.h"
#include "romazi.h"

struct mapping {
	struct key_mapping_array arr;
	struct kanji_distribution dist;

	/*
	 * キーがN文字の長さの場合、接頭辞がそのキーで長さが
	 * N+1 の入力コードの中で最も低い部首+画数インデックスを持つ漢字が、
	 * そのキーの該当する value です。
	 *
	 * 例えば入力マッピングが以下としましょう:
	 *	2a -> 作
	 *	2b -> 備
	 *	2c -> 仕
	 * cutoff_map では [2 -> 仕] のマッピングがあることになります。
	 *
	 * "abX" のような、シフトされたキーで終わる入力コードが "abxc" と一緒に
	 * 扱います。
	 *
	 * value が kanji_db における漢字のインデックス。
	 */
	struct {
		Orig *keys;
		uint16_t *values;
		size_t bucket_cnt;
	} cutoff_map;
};

void mapping_flags(struct flagset *);

/* エラーの時は非0を返す */
int mapping_populate(struct flagset *, struct mapping *);

/* エラーの時は非0を返す */
int mapping_lazy_populate(struct flagset *, struct mapping *, char const *key_prefix);

void destroy_mapping(struct mapping *);

#endif
