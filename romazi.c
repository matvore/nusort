#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chars.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

const char KEY_INDEX_TO_CHAR_MAP[MAPPABLE_CHAR_COUNT] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',

	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',
	'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
};

static KeyIndex char_to_key_index_map[128];

/*
 * 漢字とカナ入力で使わないキーはインデックスを持たない。そのキーの場合は、
 * -1を返す。
 */
KeyIndex char_to_key_index(char ch)
{
	KeyIndex key_index;

	if (ch >= sizeof(char_to_key_index_map) || ch <= 0)
		DIE(0, "%d", (int) ch);

	if (char_to_key_index_map[0])
		return char_to_key_index_map[(int) ch];

	memset(char_to_key_index_map, 0xff, sizeof(char_to_key_index_map));
	for (key_index = 0; key_index < MAPPABLE_CHAR_COUNT; key_index++)
		char_to_key_index_map[(int) KEY_INDEX_TO_CHAR_MAP[key_index]] =
				key_index;

	return char_to_key_index_map[(int) ch];
}

KeyIndex char_to_key_index_or_die(char ch)
{
	KeyIndex i = char_to_key_index(ch);

	if (i == -1)
		DIE(0, "char -> キーインデックスマップで見つからない： %c", ch);

	return i;
}

static void append_mapping_auto_kata(
	struct key_mapping_array *c, char const *orig, char const *conv)
{
	char *r;

	append_mapping(c, orig, conv);
	append_mapping(c, orig, conv);

	for (r = c->el[c->cnt - 1].orig; *r; r++) {
		if (*r >= 'a' && *r <= 'z')
			*r &= ~0x20;
	}
	hiragana_to_katakana(c->el[c->cnt - 1].conv);
}

void hiragana_to_katakana(char *conv)
{
	while (*conv) {
		uint16_t low_2_bytes =
			((uint16_t)conv[1] << 8) + (conv[2] & 0xff);

		low_2_bytes += 0x120;
		conv[1] = low_2_bytes >> 8;
		conv[2] = low_2_bytes;
		/* UTF-8バイト間の繰り上げ */
		if (conv[2] & 0x40) {
			conv[2] &= ~0x40;
			conv[1] += 1;
		}
		conv += 3;
	}
}

void init_romazi_config_for_cli_flags(struct romazi_config *config)
{
	if (!bytes_are_zero(config, sizeof(*config)))
		DIE(0, "romazi_config not initialized to zero bytes");

	config->include_kanji_numerals = 1;
	config->classic_wo = 1;
	config->kakko = 's';
}

int parse_romazi_flags(
	int *argc, char const *const **argv, struct romazi_config *config)
{
	if (!strcmp((*argv)[0], "--no-classic-wo")) {
		config->classic_wo = 0;
		(*argv)++;
		(*argc)--;
		return 1;
	}
	if (!strcmp((*argv)[0], "--hiragana-wo-key") && *argc > 1 &&
		   strlen((*argv)[1]) == 1) {
		config->hiragana_wo_key = (*argv)[1][0];
		*argv += 2;
		*argc -= 2;
		return 1;
	}
	if (!strcmp((*argv)[0], "--no-kanji-nums")) {
		config->include_kanji_numerals = 0;
		(*argv)++;
		(*argc)--;
		return 1;
	}
	if (!strcmp((*argv)[0], "--romazi-optimize-keystrokes")) {
		config->optimize_keystrokes = 1;
		(*argv)++;
		(*argc)--;
		return 1;
	}
	if (!strcmp((*argv)[0], "--pack-kakko")) {
		config->kakko = 'p';
		(*argv)++;
		(*argc)--;
		return 1;
	}
	if (!strcmp((*argv)[0], "--no-kakko")) {
		config->kakko = 0;
		(*argv)++;
		(*argc)--;
		return 1;
	}
	return 0;
}

static void append_secondary_i_retsu_mappings(
	struct key_mapping_array *codes, char orig0, char const *kana)
{
	Orig o = {orig0, 'y'};
	Conv c = {0};

	memcpy(c, kana, 3);

	o[2] = 'a';
	memcpy(c + 3, "ゃ", 3);
	append_mapping_auto_kata(codes, o, c);
	o[2] = 's';
	memcpy(c + 6, "う", 3);
	append_mapping_auto_kata(codes, o, c);
	c[6] = 0;

	o[2] = 'i';
	memcpy(c + 3, "ぃ", 3);
	append_mapping_auto_kata(codes, o, c);

	o[2] = 'u';
	memcpy(c + 3, "ゅ", 3);
	append_mapping_auto_kata(codes, o, c);
	o[2] = 'y';
	memcpy(c + 6, "う", 3);
	append_mapping_auto_kata(codes, o, c);
	c[6] = 0;

	o[2] = 'e';
	memcpy(c + 3, "ぇ", 3);
	append_mapping_auto_kata(codes, o, c);

	o[2] = 'o';
	memcpy(c + 3, "ょ", 3);
	append_mapping_auto_kata(codes, o, c);
	o[2] = 'p';
	memcpy(c + 6, "う", 3);
	append_mapping_auto_kata(codes, o, c);
}

static void append_optimized(
	struct romazi_config const *config, struct key_mapping_array *c,
	char const *norm_orig, char const *opt_orig, char const *conv)
{
	if (config->optimize_keystrokes)
		append_mapping_auto_kata(c, opt_orig, conv);
	else
		append_mapping_auto_kata(c, norm_orig, conv);
}

void get_romazi_codes(
	struct romazi_config const *config, struct key_mapping_array *c)
{
	append_mapping(c, "BW7", "ヴャ");
	append_mapping(c, "BW8", "ヴュ");
	append_mapping(c, "BW9", "ヴョ");
	append_mapping(c, "BWA", "ヴァ");
	append_mapping(c, "BWI", "ヴィ");
	append_mapping(c, "BWU", "ヴ");
	append_mapping(c, "BWE", "ヴェ");
	append_mapping(c, "BWO", "ヴォ");
	append_mapping(c, "TSA", "ツァ");
	append_mapping(c, "TSI", "ツィ");
	append_mapping(c, "TSE", "ツェ");
	append_mapping(c, "TSO", "ツォ");
	append_mapping(c, "THA", "テャ");
	append_mapping(c, "THI", "ティ");
	append_mapping(c, "THU", "テュ");
	append_mapping(c, "THE", "テェ");
	append_mapping(c, "THO", "テョ");
	append_mapping(c, "DHA", "デャ");
	append_mapping(c, "DHI", "ディ");
	append_mapping(c, "DHU", "デュ");
	append_mapping(c, "DHE", "デェ");
	append_mapping(c, "DHO", "デョ");
	append_mapping(c, "TWA", "トァ");
	append_mapping(c, "TWI", "トィ");
	append_mapping(c, "TWU", "トゥ");
	append_mapping(c, "TWE", "トェ");
	append_mapping(c, "TWO", "トォ");
	append_mapping(c, "DWA", "ドァ");
	append_mapping(c, "DWI", "ドィ");
	append_mapping(c, "DWU", "ドゥ");
	append_mapping(c, "DWE", "ドェ");
	append_mapping(c, "DWO", "ドォ");
	append_mapping(c, "HW7", "フャ");
	append_mapping(c, "HW8", "フュ");
	append_mapping(c, "HW9", "フョ");
	append_mapping(c, "HWA", "ファ");
	append_mapping(c, "HWI", "フィ");
	append_mapping(c, "HWE", "フェ");
	append_mapping(c, "HWO", "フォ");
	append_mapping(c, "XA",  "ァ");
	append_mapping(c, "XI",  "ィ");
	append_mapping(c, "XU",  "ゥ");
	append_mapping(c, "XE",  "ェ");
	append_mapping(c, "XO",  "ォ");
	append_mapping(c, "YE",  "イェ");
	append_mapping(c, "GHA", "ジャ");
	append_mapping(c, "GHI", "ジィ");
	append_mapping(c, "GHU", "ジュ");
	append_mapping(c, "GHE", "ジェ");
	append_mapping(c, "GHO", "ジョ");
	append_mapping(c, "GWA", "グァ");
	append_mapping(c, "GWI", "グィ");
	append_mapping(c, "GWU", "グゥ");
	append_mapping(c, "GWE", "グェ");
	append_mapping(c, "GWO", "グォ");
	append_mapping(c, "KWA", "クァ");
	append_mapping(c, "KWI", "クィ");
	append_mapping(c, "KWU", "クゥ");
	append_mapping(c, "KWE", "クェ");
	append_mapping(c, "KWO", "クォ");
	append_mapping(c, "TSU", "ツ");
	append_mapping(c, "WI",  "ウィ");
	append_mapping(c, "WE",  "ウェ");
	append_mapping(c, "WHA", "ウァ");
	append_mapping(c, "WHI", "ウィ");
	append_mapping(c, "WHU", "ウゥ");
	append_mapping(c, "WHE", "ウェ");
	append_mapping(c, "WHO", "ウォ");

	append_mapping(c, "-", "ー");

	append_optimized(config, c, "a",  ";a", "あ");
	append_mapping_auto_kata(c, "i",        "い");
	append_optimized(config, c, "u",  ";u", "う");
	append_optimized(config, c, "e",  ";e", "え");
	append_optimized(config, c, "o",  ";o", "お");
	append_mapping_auto_kata(c, "ka",       "か");
	append_mapping_auto_kata(c, "ki",       "き");
	append_mapping_auto_kata(c, "ku",       "く");
	append_mapping_auto_kata(c, "ke",       "け");
	append_mapping_auto_kata(c, "ko",       "こ");
	append_mapping_auto_kata(c, ",a",       "ゕ");
	append_mapping_auto_kata(c, ",e",       "ゖ");
	append_mapping_auto_kata(c, "ga",       "が");
	append_mapping_auto_kata(c, "gi",       "ぎ");
	append_mapping_auto_kata(c, "gu",       "ぐ");
	append_mapping_auto_kata(c, "ge",       "げ");
	append_mapping_auto_kata(c, "go",       "ご");
	append_mapping_auto_kata(c, "sa",       "さ");
	append_optimized(config, c, "si", "f",  "し");
	append_mapping_auto_kata(c, "su",       "す");
	append_mapping_auto_kata(c, "se",       "せ");
	append_mapping_auto_kata(c, "so",       "そ");
	append_mapping_auto_kata(c, "za",       "ざ");
	append_mapping_auto_kata(c, "zi",       "じ");
	append_mapping_auto_kata(c, "zu",       "ず");
	append_mapping_auto_kata(c, "ze",       "ぜ");
	append_mapping_auto_kata(c, "zo",       "ぞ");
	append_optimized(config, c, "ta", "a",  "た");
	append_mapping_auto_kata(c, "ti",       "ち");
	append_mapping_auto_kata(c, "tu",       "つ");
	append_optimized(config, c, "j",  "to", "っ");
	append_optimized(config, c, "te", "u",  "て");
	append_optimized(config, c, "to", "q",  "と");
	append_mapping_auto_kata(c, "da",       "だ");
	append_mapping_auto_kata(c, "di",       "ぢ");
	append_mapping_auto_kata(c, "du",       "づ");
	append_mapping_auto_kata(c, "de",       "で");
	append_mapping_auto_kata(c, "do",       "ど");
	append_optimized(config, c, "na", "o",  "な");
	append_optimized(config, c, "ni", "e",  "に");
	append_mapping_auto_kata(c, "nu",       "ぬ");
	append_mapping_auto_kata(c, "ne",       "ね");
	append_optimized(config, c, "no", "j",  "の");
	append_optimized(config, c, "ha", "l",  "は");
	append_mapping_auto_kata(c, "hi",       "ひ");
	append_mapping_auto_kata(c, "hu",       "ふ");
	append_mapping_auto_kata(c, "he",       "へ");
	append_mapping_auto_kata(c, "ho",       "ほ");
	append_mapping_auto_kata(c, "ba",       "ば");
	append_mapping_auto_kata(c, "bi",       "び");
	append_mapping_auto_kata(c, "bu",       "ぶ");
	append_mapping_auto_kata(c, "be",       "べ");
	append_mapping_auto_kata(c, "bo",       "ぼ");
	append_mapping_auto_kata(c, "pa",       "ぱ");
	append_mapping_auto_kata(c, "pi",       "ぴ");
	append_mapping_auto_kata(c, "pu",       "ぷ");
	append_mapping_auto_kata(c, "pe",       "ぺ");
	append_mapping_auto_kata(c, "po",       "ぽ");
	append_mapping_auto_kata(c, "ma",       "ま");
	append_mapping_auto_kata(c, "mi",       "み");
	append_mapping_auto_kata(c, "mu",       "む");
	append_mapping_auto_kata(c, "me",       "め");
	append_mapping_auto_kata(c, "mo",       "も");
	append_mapping_auto_kata(c, "ya",       "や");
	append_mapping_auto_kata(c, "yu",       "ゆ");
	append_mapping_auto_kata(c, "yo",       "よ");
	append_mapping_auto_kata(c, "ra",       "ら");
	append_mapping_auto_kata(c, "ri",       "り");
	append_optimized(config, c, "ru", "v",  "る");
	append_optimized(config, c, "re", "c",  "れ");
	append_mapping_auto_kata(c, "ro",       "ろ");
	append_mapping_auto_kata(c, "wa",       "わ");
	append_mapping_auto_kata(c, "xxl",      "を");
	append_optimized(config, c, "f",  ";n", "ん");
	append_mapping_auto_kata(c, "xxa",      "ぁ");
	append_mapping_auto_kata(c, "xxi",      "ぃ");
	append_mapping_auto_kata(c, "xxu",      "ぅ");
	append_mapping_auto_kata(c, "xxe",      "ぇ");
	append_mapping_auto_kata(c, "xxo",      "ぉ");
	append_mapping_auto_kata(c, "xx7",      "ゃ");
	append_mapping_auto_kata(c, "xx8",      "ゅ");
	append_mapping_auto_kata(c, "xx9",      "ょ");
	append_mapping_auto_kata(c, "xxd",      "ゑ");
	append_mapping_auto_kata(c, "xxk",      "ゐ");
	append_mapping_auto_kata(c, "xxw",      "ゎ");
	append_mapping_auto_kata(c, "xxv",      "ゔ");

	append_secondary_i_retsu_mappings(c, 'k', "き");
	append_secondary_i_retsu_mappings(c, 'g', "ぎ");
	append_secondary_i_retsu_mappings(c, 's', "し");
	append_secondary_i_retsu_mappings(c, 'z', "じ");
	append_secondary_i_retsu_mappings(c, 't', "ち");
	append_secondary_i_retsu_mappings(c, 'd', "ぢ");
	append_secondary_i_retsu_mappings(c, 'n', "に");
	append_secondary_i_retsu_mappings(c, 'h', "ひ");
	append_secondary_i_retsu_mappings(c, 'b', "び");
	append_secondary_i_retsu_mappings(c, 'p', "ぴ");
	append_secondary_i_retsu_mappings(c, 'm', "み");
	append_secondary_i_retsu_mappings(c, 'r', "り");

	if (config->include_kanji_numerals) {
		append_mapping(c, "!", "一");
		append_mapping(c, "@", "二");
		append_mapping(c, "#", "三");
		append_mapping(c, "$", "四");
		append_mapping(c, "%", "五");
		append_mapping(c, "^", "六");
		append_mapping(c, "&", "七");
		append_mapping(c, "*", "八");
		append_mapping(c, "(", "九");
		append_mapping(c, ")", "十");
	}

	if (config->hiragana_wo_key) {
		Orig o = {0};
		o[0] = config->hiragana_wo_key;
		append_mapping(c, o, "を");
	}

	if (config->classic_wo) {
		append_mapping_auto_kata(c, "wo", "を");
	}

	append_mapping(c, "xxj", "わ" COMBINING_DAKUTEN);
	append_mapping(c, "xxc", "ゑ" COMBINING_DAKUTEN);
	append_mapping(c, "xx,", "ゐ" COMBINING_DAKUTEN);
	append_mapping(c, "xx.", "を" COMBINING_DAKUTEN);
	append_mapping(c, "XXJ", "ヷ");
	append_mapping(c, "XXC", "ヹ");
	append_mapping(c, "XX,", "ヸ");
	append_mapping(c, "XX.", "ヺ");

	switch (config->kakko) {
	case 0: break;
	case 'p':
		append_mapping(c, "]1", "‘");
		append_mapping(c, "]2", "’");
		append_mapping(c, "]3", "“");
		append_mapping(c, "]4", "”");
		append_mapping(c, "]q", "〈");
		append_mapping(c, "]w", "〉");
		append_mapping(c, "]e", "《");
		append_mapping(c, "]r", "》");
		append_mapping(c, "]a", "「");
		append_mapping(c, "]s", "」");
		append_mapping(c, "]d", "『");
		append_mapping(c, "]f", "』");
		append_mapping(c, "]z", "【");
		append_mapping(c, "]x", "】");
		append_mapping(c, "]c", "〔");
		append_mapping(c, "]v", "〕");
		append_mapping(c, "]y", "〖");
		append_mapping(c, "]u", "〗");
		append_mapping(c, "]h", "〘");
		append_mapping(c, "]j", "〙");
		append_mapping(c, "]n", "〝");
		append_mapping(c, "]m", "〟");
		append_mapping(c, "]7", "｟");
		append_mapping(c, "]8", "｠");
		break;
	case 's':
		append_mapping(c, "[q", "‘");
		append_mapping(c, "]q", "’");
		append_mapping(c, "[a", "“");
		append_mapping(c, "]a", "”");
		append_mapping(c, "[e", "〈");
		append_mapping(c, "]e", "〉");
		append_mapping(c, "[r", "《");
		append_mapping(c, "]r", "》");
		append_mapping(c, "[s", "「");
		append_mapping(c, "]s", "」");
		append_mapping(c, "[d", "『");
		append_mapping(c, "]d", "』");
		append_mapping(c, "[f", "【");
		append_mapping(c, "]f", "】");
		append_mapping(c, "[w", "〔");
		append_mapping(c, "]w", "〕");
		append_mapping(c, "[v", "〖");
		append_mapping(c, "]v", "〗");
		append_mapping(c, "[c", "〘");
		append_mapping(c, "]c", "〙");
		append_mapping(c, "[g", "〝");
		append_mapping(c, "]g", "〟");
		append_mapping(c, "[x", "｟");
		append_mapping(c, "]x", "｠");
		break;
	default:
		DIE(0, "括弧の入力設定が無効です: %d (%c)",
		    (int) config->kakko, config->kakko);
	}
}
