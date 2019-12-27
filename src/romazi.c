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
static KeyIndex char_to_key_index(char ch)
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

struct {
	char const **el;
	size_t cnt;
	size_t alloc;
} target_strs;

static struct key_mapping_array codes;

static void verify_initialized(void)
{
	if (!codes.cnt)
		BUG("must call init_romazi()");
}

int is_target_non_sorted_string(const char *s)
{
	const char **e;
	if (!target_strs.cnt) {
		size_t i;

		verify_initialized();
		for (i = 0; i < codes.cnt; i++) {
			GROW_ARRAY_BY(target_strs, 1);
			target_strs.el[i] = codes.el[i].conv;
		}
		QSORT(, target_strs.el, target_strs.cnt,
		      strcmp(target_strs.el[a], target_strs.el[b]) < 0);
	}
	BSEARCH(e, target_strs.el, target_strs.cnt, strcmp(*e, s));
	return !!e;
}

struct used_bit_map {
	char m[MAPPABLE_CHAR_COUNT * MAPPABLE_CHAR_COUNT];
};

static int free_as_singleton_code(
	const struct used_bit_map *used,
	int key_code)
{
	size_t i;

	for (i = key_code * MAPPABLE_CHAR_COUNT;
			i < (key_code + 1) * MAPPABLE_CHAR_COUNT;
			i++) {
		if (used->m[i])
			return 0;
	}
	return 1;
}

static void get_free_kanji_keys(struct used_bit_map *used)
{
	size_t i;

	verify_initialized();
	memset(used, 0, sizeof(*used));

	for (i = 0; i < codes.cnt; i++) {
		ssize_t first_key_off = char_to_key_index(codes.el[i].orig[0]);

		if (first_key_off == -1)
			continue;

		first_key_off *= MAPPABLE_CHAR_COUNT;

		if (strlen(codes.el[i].orig) >= 2)
			used->m[
				first_key_off
				+ char_to_key_index(codes.el[i].orig[1])
			] = 1;
		else
			memset(used->m + first_key_off, 1, MAPPABLE_CHAR_COUNT);
	}
}

void get_free_kanji_keys_count(struct unused_kanji_keys *u)
{
	struct used_bit_map used;
	size_t key1;

	get_free_kanji_keys(&used);

	memset(u, 0, sizeof(*u));
	for (key1 = 0; key1 < KANJI_KEY_COUNT; key1++) {
		size_t key2;
		size_t shifted_key1 = key1 + MAPPABLE_CHAR_COUNT / 2;

		for (key2 = 0; key2 < KANJI_KEY_COUNT; key2++) {
			if (!used.m[key1 * MAPPABLE_CHAR_COUNT + key2])
				u->count[key1]++;
		}

		if (free_as_singleton_code(&used, shifted_key1))
			u->count[key1]++;
	}
}

void get_free_kanji_codes(struct short_code_array *codes)
{
	struct used_bit_map used;
	size_t key1;

	get_free_kanji_keys(&used);

	for (key1 = 0; key1 < KANJI_KEY_COUNT; key1++) {
		size_t key2;
		size_t shifted_key1 = key1 + MAPPABLE_CHAR_COUNT / 2;

		for (key2 = 0; key2 < KANJI_KEY_COUNT; key2++) {
			if (used.m[key1 * MAPPABLE_CHAR_COUNT + key2])
				continue;
			GROW_ARRAY_BY(*codes, 1);
			codes->el[codes->cnt - 1][0] =
				KEY_INDEX_TO_CHAR_MAP[key1];
			codes->el[codes->cnt - 1][1] =
				KEY_INDEX_TO_CHAR_MAP[key2];
		}

		if (free_as_singleton_code(&used, shifted_key1)) {
			GROW_ARRAY_BY(*codes, 1);
			codes->el[codes->cnt - 1][0] =
				KEY_INDEX_TO_CHAR_MAP[shifted_key1];
		}
	}
}

static void append_mapping(char const *orig, char const *conv)
{
	GROW_ARRAY_BY(codes, 1);
	strncpy(codes.el[codes.cnt - 1].orig, orig, sizeof(codes.el[0].orig));
	strncpy(codes.el[codes.cnt - 1].conv, conv, sizeof(codes.el[0].conv));
}

static void append_mapping_auto_katakana(char const *orig, char const *conv)
{
	char *r;

	append_mapping(orig, conv);
	append_mapping(orig, conv);

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

int init_romazi_and_return_status(struct romazi_config const *config)
{
	int32_t i;

	DESTROY_ARRAY(codes);
	DESTROY_ARRAY(target_strs);

#ifdef ROMAZI_F_KEY_KATAKANA
	append_mapping("FA",	"ファ");
	append_mapping("FI",	"フィ");
	append_mapping("FE",	"フェ");
	append_mapping("FO",	"フォ");
	append_mapping("FYA",	"フャ");
	append_mapping("FYU",	"フュ");
	append_mapping("FYO",	"フョ");
#endif

#ifdef ROMAZI_J_KEY_KATAKANA
	append_mapping("JA",	"ジャ");
	append_mapping("JI",	"ジ");
	append_mapping("JU",	"ジュ");
	append_mapping("JE",	"ジェ");
	append_mapping("JO",	"ジョ");
	append_mapping("JYA",	"ジャ");
	append_mapping("JYI",	"ジィ");
	append_mapping("JYU",	"ジュ");
	append_mapping("JYE",	"ジェ");
	append_mapping("JYO",	"ジョ");
#endif

#ifdef ROMAZI_V_KEY_KATAKANA
	append_mapping("VA",	"ヴァ");
	append_mapping("VI",	"ヴィ");
	append_mapping("VU",	"ヴ");
	append_mapping("VE",	"ヴェ");
	append_mapping("VO",	"ヴォ");
	append_mapping("VYA",	"ヴャ");
	append_mapping("VYI",	"ヴィ");
	append_mapping("VYU",	"ヴュ");
	append_mapping("VYE",	"ヴェ");
	append_mapping("VYO",	"ヴョ");
#endif

	append_mapping("BW7", "ヴャ");
	append_mapping("BW8", "ヴュ");
	append_mapping("BW9", "ヴョ");
	append_mapping("BWA", "ヴァ");
	append_mapping("BWI", "ヴィ");
	append_mapping("BWU", "ヴ");
	append_mapping("BWE", "ヴェ");
	append_mapping("BWO", "ヴォ");
	append_mapping("DYA", "ヂャ");
	append_mapping("DYI", "ヂィ");
	append_mapping("DYU", "ヂュ");
	append_mapping("DYE", "ヂェ");
	append_mapping("DYO", "ヂョ");
	append_mapping("TSA", "ツァ");
	append_mapping("TSI", "ツィ");
	append_mapping("TSE", "ツェ");
	append_mapping("TSO", "ツォ");
	append_mapping("THA", "テャ");
	append_mapping("THI", "ティ");
	append_mapping("THU", "テュ");
	append_mapping("THE", "テェ");
	append_mapping("THO", "テョ");
	append_mapping("DHA", "デャ");
	append_mapping("DHI", "ディ");
	append_mapping("DHU", "デュ");
	append_mapping("DHE", "デェ");
	append_mapping("DHO", "デョ");
	append_mapping("TWA", "トァ");
	append_mapping("TWI", "トィ");
	append_mapping("TWU", "トゥ");
	append_mapping("TWE", "トェ");
	append_mapping("TWO", "トォ");
	append_mapping("DWA", "ドァ");
	append_mapping("DWI", "ドィ");
	append_mapping("DWU", "ドゥ");
	append_mapping("DWE", "ドェ");
	append_mapping("DWO", "ドォ");
	append_mapping("HW7", "フャ");
	append_mapping("HW8", "フュ");
	append_mapping("HW9", "フョ");
	append_mapping("HWA", "ファ");
	append_mapping("HWI", "フィ");
	append_mapping("HWE", "フェ");
	append_mapping("HWO", "フォ");
	append_mapping("HYA", "ヒャ");
	append_mapping("HYI", "ヒィ");
	append_mapping("HYU", "ヒュ");
	append_mapping("HYE", "ヒェ");
	append_mapping("HYO", "ヒョ");
	append_mapping("XA",  "ァ");
	append_mapping("XI",  "ィ");
	append_mapping("XU",  "ゥ");
	append_mapping("XE",  "ェ");
	append_mapping("XO",  "ォ");
	append_mapping("YE",  "イェ");
	append_mapping("GHA", "ジャ");
	append_mapping("GHI", "ジィ");
	append_mapping("GHU", "ジュ");
	append_mapping("GHE", "ジェ");
	append_mapping("GHO", "ジョ");
	append_mapping("GWA", "グァ");
	append_mapping("GWI", "グィ");
	append_mapping("GWU", "グゥ");
	append_mapping("GWE", "グェ");
	append_mapping("GWO", "グォ");
	append_mapping("KWA", "クァ");
	append_mapping("KWI", "クィ");
	append_mapping("KWU", "クゥ");
	append_mapping("KWE", "クェ");
	append_mapping("KWO", "クォ");
	append_mapping("TSU", "ツ");
	append_mapping("WI",  "ウィ");
	append_mapping("WE",  "ウェ");
	append_mapping("WHA", "ウァ");
	append_mapping("WHI", "ウィ");
	append_mapping("WHU", "ウゥ");
	append_mapping("WHE", "ウェ");
	append_mapping("WHO", "ウォ");

	append_mapping("-", "ー");

	append_mapping_auto_katakana("j",   "っ");
	append_mapping_auto_katakana("f",   "ん");
	append_mapping_auto_katakana("a",   "あ");
	append_mapping_auto_katakana("i",   "い");
	append_mapping_auto_katakana("u",   "う");
	append_mapping_auto_katakana("e",   "え");
	append_mapping_auto_katakana("o",   "お");
	append_mapping_auto_katakana("ka",  "か");
	append_mapping_auto_katakana("ki",  "き");
	append_mapping_auto_katakana("ku",  "く");
	append_mapping_auto_katakana("ke",  "け");
	append_mapping_auto_katakana("ko",  "こ");
	append_mapping_auto_katakana(",a",  "ゕ");
	append_mapping_auto_katakana(",e",  "ゖ");
	append_mapping_auto_katakana("ga",  "が");
	append_mapping_auto_katakana("gi",  "ぎ");
	append_mapping_auto_katakana("gu",  "ぐ");
	append_mapping_auto_katakana("ge",  "げ");
	append_mapping_auto_katakana("go",  "ご");
	append_mapping_auto_katakana("sa",  "さ");
	append_mapping_auto_katakana("si",  "し");
	append_mapping_auto_katakana("su",  "す");
	append_mapping_auto_katakana("se",  "せ");
	append_mapping_auto_katakana("so",  "そ");
	append_mapping_auto_katakana("za",  "ざ");
	append_mapping_auto_katakana("zi",  "じ");
	append_mapping_auto_katakana("zu",  "ず");
	append_mapping_auto_katakana("ze",  "ぜ");
	append_mapping_auto_katakana("zo",  "ぞ");
	append_mapping_auto_katakana("ta",  "た");
	append_mapping_auto_katakana("ti",  "ち");
	append_mapping_auto_katakana("tu",  "つ");
	append_mapping_auto_katakana("te",  "て");
	append_mapping_auto_katakana("to",  "と");
	append_mapping_auto_katakana("da",  "だ");
	append_mapping_auto_katakana("di",  "ぢ");
	append_mapping_auto_katakana("du",  "づ");
	append_mapping_auto_katakana("de",  "で");
	append_mapping_auto_katakana("do",  "ど");
	append_mapping_auto_katakana("na",  "な");
	append_mapping_auto_katakana("ni",  "に");
	append_mapping_auto_katakana("nu",  "ぬ");
	append_mapping_auto_katakana("ne",  "ね");
	append_mapping_auto_katakana("no",  "の");
	append_mapping_auto_katakana("ha",  "は");
	append_mapping_auto_katakana("hi",  "ひ");
	append_mapping_auto_katakana("hu",  "ふ");
	append_mapping_auto_katakana("he",  "へ");
	append_mapping_auto_katakana("ho",  "ほ");
	append_mapping_auto_katakana("ba",  "ば");
	append_mapping_auto_katakana("bi",  "び");
	append_mapping_auto_katakana("bu",  "ぶ");
	append_mapping_auto_katakana("be",  "べ");
	append_mapping_auto_katakana("bo",  "ぼ");
	append_mapping_auto_katakana("pa",  "ぱ");
	append_mapping_auto_katakana("pi",  "ぴ");
	append_mapping_auto_katakana("pu",  "ぷ");
	append_mapping_auto_katakana("pe",  "ぺ");
	append_mapping_auto_katakana("po",  "ぽ");
	append_mapping_auto_katakana("ma",  "ま");
	append_mapping_auto_katakana("mi",  "み");
	append_mapping_auto_katakana("mu",  "む");
	append_mapping_auto_katakana("me",  "め");
	append_mapping_auto_katakana("mo",  "も");
	append_mapping_auto_katakana("ya",  "や");
	append_mapping_auto_katakana("yu",  "ゆ");
	append_mapping_auto_katakana("yo",  "よ");
	append_mapping_auto_katakana("ra",  "ら");
	append_mapping_auto_katakana("ri",  "り");
	append_mapping_auto_katakana("ru",  "る");
	append_mapping_auto_katakana("re",  "れ");
	append_mapping_auto_katakana("ro",  "ろ");
	append_mapping_auto_katakana("wa",  "わ");
	append_mapping_auto_katakana("xxa", "ぁ");
	append_mapping_auto_katakana("xxi", "ぃ");
	append_mapping_auto_katakana("xxu", "ぅ");
	append_mapping_auto_katakana("xxe", "ぇ");
	append_mapping_auto_katakana("xxo", "ぉ");
	append_mapping_auto_katakana("xx7", "ゃ");
	append_mapping_auto_katakana("xx8", "ゅ");
	append_mapping_auto_katakana("xx9", "ょ");
	append_mapping_auto_katakana("xxd", "ゑ");
	append_mapping_auto_katakana("xxk", "ゐ");
	append_mapping_auto_katakana("xxl", "を");
	append_mapping_auto_katakana("xxw", "ゎ");
	append_mapping_auto_katakana("xxv", "ゔ");

	append_secondary_i_retsu_mappings('k', "き");
	append_secondary_i_retsu_mappings('g', "ぎ");
	append_secondary_i_retsu_mappings('s', "し");
	append_secondary_i_retsu_mappings('z', "じ");
	append_secondary_i_retsu_mappings('t', "ち");
	append_secondary_i_retsu_mappings('n', "に");
	append_secondary_i_retsu_mappings('b', "び");
	append_secondary_i_retsu_mappings('p', "ぴ");
	append_secondary_i_retsu_mappings('m', "み");
	append_secondary_i_retsu_mappings('r', "り");

	if (config->include_kanji_numerals) {
		append_mapping("!", "一");
		append_mapping("@", "二");
		append_mapping("#", "三");
		append_mapping("$", "四");
		append_mapping("%", "五");
		append_mapping("^", "六");
		append_mapping("&", "七");
		append_mapping("*", "八");
		append_mapping("(", "九");
		append_mapping(")", "十");
	}

	if (config->hiragana_wo_key) {
		Orig o = {0};
		o[0] = config->hiragana_wo_key;
		append_mapping(o, "を");
	}

	if (config->classic_wo) {
		append_mapping_auto_katakana("wo", "を");
	}

	append_mapping("xxj", "わ" COMBINING_DAKUTEN);
	append_mapping("xxc", "ゑ" COMBINING_DAKUTEN);
	append_mapping("xx,", "ゐ" COMBINING_DAKUTEN);
	append_mapping("xx.", "を" COMBINING_DAKUTEN);
	append_mapping("XXJ", "ヷ");
	append_mapping("XXC", "ヹ");
	append_mapping("XX,", "ヸ");
	append_mapping("XX.", "ヺ");

	for (i = 0; i < codes.cnt; i++) {
		if (codes.el[i].orig[0] == config->hiragana_wo_key &&
		    strcmp("を", codes.el[i].conv)) {
			xfputs("ダブっている入力コードがあります。\n", err);
			return 0;
		}
	}

	return 1;
}

void init_romazi(struct romazi_config const *config)
{
	if (!init_romazi_and_return_status(config))
		DIE(0, "ローマ字入力コードの生成に失敗しました。");
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
