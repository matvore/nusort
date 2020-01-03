#ifndef ROMAZI_H
#define ROMAZI_H

#include <stddef.h>
#include <sys/types.h>

#include "mapping_util.h"

#define KANJI_KEY_COUNT 40
#define MAPPABLE_CHAR_COUNT (KANJI_KEY_COUNT * 2)

typedef int8_t KeyIndex;

extern const char KEY_INDEX_TO_CHAR_MAP[MAPPABLE_CHAR_COUNT];

KeyIndex char_to_key_index_or_die(char ch);

struct unused_kanji_keys {
	unsigned char count[KANJI_KEY_COUNT];
};

int is_target_non_sorted_string(const char *s);

void get_free_kanji_keys_count(struct unused_kanji_keys *u);

struct short_code_array {
	char (*el)[2];
	size_t cnt;
	size_t alloc;
};

void get_free_kanji_codes(struct short_code_array *codes);

void hiragana_to_katakana(char *conv);

struct romazi_config {
	/*
	 * 一打鍵のコードを「を」に割り当てるには、これを設定。「ヲ」のコードを
	 * 生成しない。
	 */
	char hiragana_wo_key;
	/* シフト＋数字一打鍵のコードを漢字に割り当てる。*/
	unsigned include_kanji_numerals : 1;
	/* wo=を と WO=ヲ のコードを生成する。*/
	unsigned classic_wo : 1;
};

void init_romazi_config_for_cli_flags(struct romazi_config *config);

/*
 * argcとargvの差す最初のフラグがローマ字に関するものの場合、CLI 引数を解析し
 * て、argcとargvを先にして、０ではない値を返す。フラグがローマ字フラグとして認
 * 識されない場合は０を返す。
 */
int parse_romazi_flags(
	int *argc, char const *const **argv, struct romazi_config *config);

/* init_romazi_and_return_status を呼び、失敗のときはプロセスを終了する. */
void init_romazi(struct romazi_config const *);

void get_romazi_codes(struct key_mapping_array *codes_);

#endif
