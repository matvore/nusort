#include "mapping_util.h"

#include <stdio.h>

void keyboard_write(void);
void keyboard_update(struct key_mapping_array const *, char const *prefix);

/*
 * キーボード上の補候漢字を部首+画数順で出力する。
 *
 * 使う場合は keyboard_update の後で呼び出す。
 */
void keyboard_show_rsc_list(void);
