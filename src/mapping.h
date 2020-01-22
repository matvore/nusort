#ifndef MAPPING_H
#define MAPPING_H

#include <stdio.h>

#include "romazi.h"

struct mapping_config {
	unsigned six_is_rh : 1;
	unsigned include_kanji : 1;
};

void init_mapping_config_for_cli_flags(struct mapping_config *);

/*
 * argcとargvの差す最初のフラグがマッピングに関するものの場合、CLI 引数を解析し
 * て、argcとargvを先に進めて、０ではない値を返す。フラグがローマ字フラグとし
 * て認識されない場合は０を返す。
 */
int parse_mapping_flags(
	int *argc, char const *const **argv, struct mapping_config *);

int mapping_populate(struct mapping_config const *, struct key_mapping_array *);

#endif
