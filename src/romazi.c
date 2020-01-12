#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chars.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

#define COMBINING_DAKUTEN "\xe3\x82\x99"

const char KEY_INDEX_TO_CHAR_MAP[MAPPABLE_CHAR_COUNT] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',

	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
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
		BUG("%d", (int) ch);

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
		BUG("char -> キーインデックスマップで見つからない： %c", ch);

	return i;
}

static struct key_mapping_array codes;

static void verify_initialized(void)
{
	if (!codes.cnt)
		BUG("must call init_romazi()");
}

static void append_mapping_auto_katakana(char const *orig, char const *conv)
{
	char *r;

	append_mapping(&codes, orig, conv);
	append_mapping(&codes, orig, conv);

	for (r = codes.el[codes.cnt - 1].orig; *r; r++) {
		if (*r >= 'a' && *r <= 'z')
			*r &= ~0x20;
	}
	hiragana_to_katakana(codes.el[codes.cnt - 1].conv);
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
		BUG("romazi_config not initialized to zero bytes");

	config->include_kanji_numerals = 1;
	config->classic_wo = 1;
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
	return 0;
}

static void append_secondary_i_retsu_mappings(char orig0, char const *kana)
{
	Orig o = {orig0, 'y'};
	Conv c = {0};

	memcpy(c, kana, 3);

	o[2] = 'a';
	memcpy(c + 3, "ゃ", 3);
	append_mapping_auto_katakana(o, c);

	o[2] = 'i';
	memcpy(c + 3, "ぃ", 3);
	append_mapping_auto_katakana(o, c);

	o[2] = 'u';
	memcpy(c + 3, "ゅ", 3);
	append_mapping_auto_katakana(o, c);

	o[2] = 'e';
	memcpy(c + 3, "ぇ", 3);
	append_mapping_auto_katakana(o, c);

	o[2] = 'o';
	memcpy(c + 3, "ょ", 3);
	append_mapping_auto_katakana(o, c);
}

static void append_optimized(
	struct romazi_config const *config,
	char const *norm_orig, char const *opt_orig, char const *conv)
{
	if (config->optimize_keystrokes)
		append_mapping_auto_katakana(opt_orig, conv);
	else
		append_mapping_auto_katakana(norm_orig, conv);
}

void init_romazi(struct romazi_config const *config)
{
	DESTROY_ARRAY(codes);

#ifdef ROMAZI_F_KEY_KATAKANA
	append_mapping(&codes, "FA",	"ファ");
	append_mapping(&codes, "FI",	"フィ");
	append_mapping(&codes, "FE",	"フェ");
	append_mapping(&codes, "FO",	"フォ");
	append_mapping(&codes, "FYA",	"フャ");
	append_mapping(&codes, "FYU",	"フュ");
	append_mapping(&codes, "FYO",	"フョ");
#endif

#ifdef ROMAZI_J_KEY_KATAKANA
	append_mapping(&codes, "JA",	"ジャ");
	append_mapping(&codes, "JI",	"ジ");
	append_mapping(&codes, "JU",	"ジュ");
	append_mapping(&codes, "JE",	"ジェ");
	append_mapping(&codes, "JO",	"ジョ");
	append_mapping(&codes, "JYA",	"ジャ");
	append_mapping(&codes, "JYI",	"ジィ");
	append_mapping(&codes, "JYU",	"ジュ");
	append_mapping(&codes, "JYE",	"ジェ");
	append_mapping(&codes, "JYO",	"ジョ");
#endif

#ifdef ROMAZI_V_KEY_KATAKANA
	append_mapping(&codes, "VA",	"ヴァ");
	append_mapping(&codes, "VI",	"ヴィ");
	append_mapping(&codes, "VU",	"ヴ");
	append_mapping(&codes, "VE",	"ヴェ");
	append_mapping(&codes, "VO",	"ヴォ");
	append_mapping(&codes, "VYA",	"ヴャ");
	append_mapping(&codes, "VYI",	"ヴィ");
	append_mapping(&codes, "VYU",	"ヴュ");
	append_mapping(&codes, "VYE",	"ヴェ");
	append_mapping(&codes, "VYO",	"ヴョ");
#endif

	append_mapping(&codes, "BW7", "ヴャ");
	append_mapping(&codes, "BW8", "ヴュ");
	append_mapping(&codes, "BW9", "ヴョ");
	append_mapping(&codes, "BWA", "ヴァ");
	append_mapping(&codes, "BWI", "ヴィ");
	append_mapping(&codes, "BWU", "ヴ");
	append_mapping(&codes, "BWE", "ヴェ");
	append_mapping(&codes, "BWO", "ヴォ");
	append_mapping(&codes, "TSA", "ツァ");
	append_mapping(&codes, "TSI", "ツィ");
	append_mapping(&codes, "TSE", "ツェ");
	append_mapping(&codes, "TSO", "ツォ");
	append_mapping(&codes, "THA", "テャ");
	append_mapping(&codes, "THI", "ティ");
	append_mapping(&codes, "THU", "テュ");
	append_mapping(&codes, "THE", "テェ");
	append_mapping(&codes, "THO", "テョ");
	append_mapping(&codes, "DHA", "デャ");
	append_mapping(&codes, "DHI", "ディ");
	append_mapping(&codes, "DHU", "デュ");
	append_mapping(&codes, "DHE", "デェ");
	append_mapping(&codes, "DHO", "デョ");
	append_mapping(&codes, "TWA", "トァ");
	append_mapping(&codes, "TWI", "トィ");
	append_mapping(&codes, "TWU", "トゥ");
	append_mapping(&codes, "TWE", "トェ");
	append_mapping(&codes, "TWO", "トォ");
	append_mapping(&codes, "DWA", "ドァ");
	append_mapping(&codes, "DWI", "ドィ");
	append_mapping(&codes, "DWU", "ドゥ");
	append_mapping(&codes, "DWE", "ドェ");
	append_mapping(&codes, "DWO", "ドォ");
	append_mapping(&codes, "HW7", "フャ");
	append_mapping(&codes, "HW8", "フュ");
	append_mapping(&codes, "HW9", "フョ");
	append_mapping(&codes, "HWA", "ファ");
	append_mapping(&codes, "HWI", "フィ");
	append_mapping(&codes, "HWE", "フェ");
	append_mapping(&codes, "HWO", "フォ");
	append_mapping(&codes, "XA",  "ァ");
	append_mapping(&codes, "XI",  "ィ");
	append_mapping(&codes, "XU",  "ゥ");
	append_mapping(&codes, "XE",  "ェ");
	append_mapping(&codes, "XO",  "ォ");
	append_mapping(&codes, "YE",  "イェ");
	append_mapping(&codes, "GHA", "ジャ");
	append_mapping(&codes, "GHI", "ジィ");
	append_mapping(&codes, "GHU", "ジュ");
	append_mapping(&codes, "GHE", "ジェ");
	append_mapping(&codes, "GHO", "ジョ");
	append_mapping(&codes, "GWA", "グァ");
	append_mapping(&codes, "GWI", "グィ");
	append_mapping(&codes, "GWU", "グゥ");
	append_mapping(&codes, "GWE", "グェ");
	append_mapping(&codes, "GWO", "グォ");
	append_mapping(&codes, "KWA", "クァ");
	append_mapping(&codes, "KWI", "クィ");
	append_mapping(&codes, "KWU", "クゥ");
	append_mapping(&codes, "KWE", "クェ");
	append_mapping(&codes, "KWO", "クォ");
	append_mapping(&codes, "TSU", "ツ");
	append_mapping(&codes, "WI",  "ウィ");
	append_mapping(&codes, "WE",  "ウェ");
	append_mapping(&codes, "WHA", "ウァ");
	append_mapping(&codes, "WHI", "ウィ");
	append_mapping(&codes, "WHU", "ウゥ");
	append_mapping(&codes, "WHE", "ウェ");
	append_mapping(&codes, "WHO", "ウォ");

	append_mapping(&codes, "-", "ー");

	append_optimized(config,     "a",  ";a", "あ");
	append_mapping_auto_katakana("i",        "い");
	append_optimized(config,     "u",  ";u", "う");
	append_optimized(config,     "e",  ";e", "え");
	append_optimized(config,     "o",  ";o", "お");
	append_mapping_auto_katakana("ka",       "か");
	append_mapping_auto_katakana("ki",       "き");
	append_mapping_auto_katakana("ku",       "く");
	append_mapping_auto_katakana("ke",       "け");
	append_mapping_auto_katakana("ko",       "こ");
	append_mapping_auto_katakana(",a",       "ゕ");
	append_mapping_auto_katakana(",e",       "ゖ");
	append_mapping_auto_katakana("ga",       "が");
	append_mapping_auto_katakana("gi",       "ぎ");
	append_mapping_auto_katakana("gu",       "ぐ");
	append_mapping_auto_katakana("ge",       "げ");
	append_mapping_auto_katakana("go",       "ご");
	append_mapping_auto_katakana("sa",       "さ");
	append_optimized(config,     "si", "f",  "し");
	append_mapping_auto_katakana("su",       "す");
	append_mapping_auto_katakana("se",       "せ");
	append_mapping_auto_katakana("so",       "そ");
	append_mapping_auto_katakana("za",       "ざ");
	append_mapping_auto_katakana("zi",       "じ");
	append_mapping_auto_katakana("zu",       "ず");
	append_mapping_auto_katakana("ze",       "ぜ");
	append_mapping_auto_katakana("zo",       "ぞ");
	append_optimized(config,     "ta", "a",  "た");
	append_mapping_auto_katakana("ti",       "ち");
	append_mapping_auto_katakana("tu",       "つ");
	append_optimized(config,     "j",  "t;", "っ");
	append_optimized(config,     "te", "u",  "て");
	append_optimized(config,     "to", "q",  "と");
	append_mapping_auto_katakana("da",       "だ");
	append_mapping_auto_katakana("di",       "ぢ");
	append_mapping_auto_katakana("du",       "づ");
	append_mapping_auto_katakana("de",       "で");
	append_mapping_auto_katakana("do",       "ど");
	append_optimized(config,     "na", "o",  "な");
	append_optimized(config,     "ni", "e",  "に");
	append_mapping_auto_katakana("nu",       "ぬ");
	append_mapping_auto_katakana("ne",       "ね");
	append_optimized(config,     "no", "j",  "の");
	append_optimized(config,     "ha", "l",  "は");
	append_mapping_auto_katakana("hi",       "ひ");
	append_mapping_auto_katakana("hu",       "ふ");
	append_mapping_auto_katakana("he",       "へ");
	append_mapping_auto_katakana("ho",       "ほ");
	append_mapping_auto_katakana("ba",       "ば");
	append_mapping_auto_katakana("bi",       "び");
	append_mapping_auto_katakana("bu",       "ぶ");
	append_mapping_auto_katakana("be",       "べ");
	append_mapping_auto_katakana("bo",       "ぼ");
	append_mapping_auto_katakana("pa",       "ぱ");
	append_mapping_auto_katakana("pi",       "ぴ");
	append_mapping_auto_katakana("pu",       "ぷ");
	append_mapping_auto_katakana("pe",       "ぺ");
	append_mapping_auto_katakana("po",       "ぽ");
	append_mapping_auto_katakana("ma",       "ま");
	append_mapping_auto_katakana("mi",       "み");
	append_mapping_auto_katakana("mu",       "む");
	append_mapping_auto_katakana("me",       "め");
	append_mapping_auto_katakana("mo",       "も");
	append_mapping_auto_katakana("ya",       "や");
	append_mapping_auto_katakana("yu",       "ゆ");
	append_mapping_auto_katakana("yo",       "よ");
	append_mapping_auto_katakana("ra",       "ら");
	append_mapping_auto_katakana("ri",       "り");
	append_optimized(config,     "ru", "v",  "る");
	append_optimized(config,     "re", "c",  "れ");
	append_mapping_auto_katakana("ro",       "ろ");
	append_mapping_auto_katakana("wa",       "わ");
	append_mapping_auto_katakana("xxl",      "を");
	append_optimized(config,     "f",  ";n", "ん");
	append_mapping_auto_katakana("xxa",      "ぁ");
	append_mapping_auto_katakana("xxi",      "ぃ");
	append_mapping_auto_katakana("xxu",      "ぅ");
	append_mapping_auto_katakana("xxe",      "ぇ");
	append_mapping_auto_katakana("xxo",      "ぉ");
	append_mapping_auto_katakana("xx7",      "ゃ");
	append_mapping_auto_katakana("xx8",      "ゅ");
	append_mapping_auto_katakana("xx9",      "ょ");
	append_mapping_auto_katakana("xxd",      "ゑ");
	append_mapping_auto_katakana("xxk",      "ゐ");
	append_mapping_auto_katakana("xxw",      "ゎ");
	append_mapping_auto_katakana("xxv",      "ゔ");

	append_secondary_i_retsu_mappings('k', "き");
	append_secondary_i_retsu_mappings('g', "ぎ");
	append_secondary_i_retsu_mappings('s', "し");
	append_secondary_i_retsu_mappings('z', "じ");
	append_secondary_i_retsu_mappings('t', "ち");
	append_secondary_i_retsu_mappings('d', "ぢ");
	append_secondary_i_retsu_mappings('n', "に");
	append_secondary_i_retsu_mappings('h', "ひ");
	append_secondary_i_retsu_mappings('b', "び");
	append_secondary_i_retsu_mappings('p', "ぴ");
	append_secondary_i_retsu_mappings('m', "み");
	append_secondary_i_retsu_mappings('r', "り");

	if (config->include_kanji_numerals) {
		append_mapping(&codes, "!", "一");
		append_mapping(&codes, "@", "二");
		append_mapping(&codes, "#", "三");
		append_mapping(&codes, "$", "四");
		append_mapping(&codes, "%", "五");
		append_mapping(&codes, "^", "六");
		append_mapping(&codes, "&", "七");
		append_mapping(&codes, "*", "八");
		append_mapping(&codes, "(", "九");
		append_mapping(&codes, ")", "十");
	}

	if (config->hiragana_wo_key) {
		Orig o = {0};
		o[0] = config->hiragana_wo_key;
		append_mapping(&codes, o, "を");
	}

	if (config->classic_wo) {
		append_mapping_auto_katakana("wo", "を");
	}

	append_mapping(&codes, "xxj", "わ" COMBINING_DAKUTEN);
	append_mapping(&codes, "xxc", "ゑ" COMBINING_DAKUTEN);
	append_mapping(&codes, "xx,", "ゐ" COMBINING_DAKUTEN);
	append_mapping(&codes, "xx.", "を" COMBINING_DAKUTEN);
	append_mapping(&codes, "XXJ", "ヷ");
	append_mapping(&codes, "XXC", "ヹ");
	append_mapping(&codes, "XX,", "ヸ");
	append_mapping(&codes, "XX.", "ヺ");
}

void get_romazi_codes(struct key_mapping_array *codes_)
{
	uint32_t i;

	verify_initialized();
	for (i = 0; i < codes.cnt; i++)
	{
		GROW_ARRAY_BY(*codes_, 1);
		memcpy(&codes_->el[codes_->cnt - 1], &codes.el[i],
		       sizeof(*codes.el));
	}
}
