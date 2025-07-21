#ifndef ROMAZI_H
#define ROMAZI_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "mapping_util.h"

typedef int8_t KeyIndex;

extern const char KEY_INDEX_TO_CHAR_MAP[MAPPABLE_CHAR_COUNT];

KeyIndex char_to_key_index(char ch);
KeyIndex char_to_key_index_or_die(char ch);

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
	/*
	 * 頻繁に使う仮名を一打鍵で入力できるようにして、あ行の「い」以外と
	 * 「っ」と「ん」を二打鍵コードに変え、ローマ字に普段使わないキー
	 * （例えば「Q」と「V」）を一打鍵のローマ字に使います。
	 */
	unsigned optimize_keystrokes : 1;

	/*
	 * \0: 括弧がマッピングに含まれません。
	 *
	 * 's'pread: 括弧やクォートを "[?" と "]?" で入力する。"?"の打鍵は左手に
	 * なるので入力が楽です
	 *
	 * 'p'ack: 括弧やクォートを "]?" のみで入力する。めずらしい括弧はいくつか
	 * 打ちづらくなります。
	 */
	char kakko;
};

void init_romazi_config_for_cli_flags(struct romazi_config *config);

/*
 * argcとargvの差す最初のフラグがローマ字に関するものの場合、CLI 引数を解析し
 * て、argcとargvを先に進めて、０ではない値を返す。フラグがローマ字フラグとし
 * て認識されない場合は０を返す。
 */
int parse_romazi_flags(
	int *argc, char ***argv, struct romazi_config *config);

void get_romazi_codes(
	struct romazi_config const *, struct key_mapping_array *);

#endif
