#ifndef MAPPING_H
#define MAPPING_H

#include <stdio.h>

#include "kanji_distribution.h"
#include "romazi.h"

struct mapping {
	unsigned six_is_rh : 1;
	unsigned include_kanji : 1;

	/*
	 * 三打鍵以上のコードの生成を影響します。
	 * オンの場合: 三打鍵目が部首外画数に該当する
	 * オフの場合: 三打鍵目が部首続いて部首外画数 (rsc) の順番によって決まる
	 */
	unsigned resid_sc_3rd_key : 1;

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

void init_mapping_config_for_cli_flags(struct mapping *);

/*
 * argcとargvの差す最初のフラグがマッピングに関するものの場合、CLI 引数を解析し
 * て、argcとargvを先に進めて、０ではない値を返す。フラグがローマ字フラグとし
 * て認識されない場合は０を返す。
 */
int parse_mapping_flags(int *argc, char ***argv, struct mapping *);

/* エラーの時は非0を返す */
int mapping_populate(struct mapping *);

/* エラーの時は非0を返す */
int mapping_lazy_populate(struct mapping *, char const *key_prefix);

void destroy_mapping(struct mapping *);

#endif
