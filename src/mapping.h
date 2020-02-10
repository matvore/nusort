#ifndef MAPPING_H
#define MAPPING_H

#include <stdio.h>

#include "kanji_distribution.h"
#include "romazi.h"

struct mapping {
	unsigned six_is_rh : 1;
	unsigned include_kanji : 1;

	struct key_mapping_array arr;
	struct kanji_distribution dist;
};

void init_mapping_config_for_cli_flags(struct mapping *);

/*
 * argcとargvの差す最初のフラグがマッピングに関するものの場合、CLI 引数を解析し
 * て、argcとargvを先に進めて、０ではない値を返す。フラグがローマ字フラグとし
 * て認識されない場合は０を返す。
 */
int parse_mapping_flags(int *argc, char const *const **argv, struct mapping *);

/* エラーの時は非0を返す */
int mapping_populate(struct mapping *);

/* Returns 0 on success. */
int mapping_lazy_populate(struct mapping *, char const *key_prefix);

void destroy_mapping(struct mapping *);

#endif
