#include "romazi.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

struct romazi_entry {
	const char *orig;
	const char *conv;
	unsigned use_as_is     : 1;
	unsigned auto_katakana : 1;
};

struct romazi_entry ROMAZI[] = {
	{"va",	"ゔぁ",	0, 1},
	{"vi",	"ゔぃ",	0, 1},
	{"vu",	"ゔ",	0, 1},
	{"ve",	"ゔぇ",	0, 1},
	{"vo",	"ゔぉ",	0, 1},
	{"vya",	"ゔゃ",	0, 1},
	{"vyi",	"ゔぃ",	0, 1},
	{"vyu",	"ゔゅ",	0, 1},
	{"vye",	"ゔぇ",	0, 1},
	{"vyo",	"ゔょ",	0, 1},
	{"q",	"っ",	1, 1},
	{"l",	"ん",	1, 1},
	{"kya",	"きゃ",	1, 1},
	{"kyi",	"きぃ",	1, 1},
	{"kyu",	"きゅ",	1, 1},
	{"kye",	"きぇ",	1, 1},
	{"kyo",	"きょ",	1, 1},
	{"gya",	"ぎゃ",	1, 1},
	{"gyi",	"ぎぃ",	1, 1},
	{"gyu",	"ぎゅ",	1, 1},
	{"gye",	"ぎぇ",	1, 1},
	{"gyo",	"ぎょ",	1, 1},
	{"sya",	"しゃ",	1, 1},
	{"syi",	"しぃ",	1, 1},
	{"syu",	"しゅ",	1, 1},
	{"sye",	"しぇ",	1, 1},
	{"syo",	"しょ",	1, 1},
	{"zya",	"じゃ",	1, 1},
	{"zyi",	"じぃ",	1, 1},
	{"zyu",	"じゅ",	1, 1},
	{"zye",	"じぇ",	1, 1},
	{"zyo",	"じょ",	1, 1},
	{"tya",	"ちゃ",	1, 1},
	{"tyi",	"ちぃ",	1, 1},
	{"tyu",	"ちゅ",	1, 1},
	{"tye",	"ちぇ",	1, 1},
	{"tyo",	"ちょ",	1, 1},
	{"dya",	"ぢゃ",	0, 1},
	{"dyi",	"ぢぃ",	0, 1},
	{"dyu",	"ぢゅ",	0, 1},
	{"dye",	"ぢぇ",	0, 1},
	{"dyo",	"ぢょ",	0, 1},
	{"tsa",	"つぁ",	0, 1},
	{"tsi",	"つぃ",	0, 1},
	{"tse",	"つぇ",	0, 1},
	{"tso",	"つぉ",	0, 1},
	{"tha",	"てゃ",	0, 1},
	{"thi",	"てぃ",	0, 1},
	{"thu",	"てゅ",	0, 1},
	{"the",	"てぇ",	0, 1},
	{"tho",	"てょ",	0, 1},
	{"dha",	"でゃ",	0, 1},
	{"dhi",	"でぃ",	0, 1},
	{"dhu",	"でゅ",	0, 1},
	{"dhe",	"でぇ",	0, 1},
	{"dho",	"でょ",	0, 1},
	{"twa",	"とぁ",	0, 1},
	{"twi",	"とぃ",	0, 1},
	{"twu",	"とぅ",	0, 1},
	{"twe",	"とぇ",	0, 1},
	{"two",	"とぉ",	0, 1},
	{"dwa",	"どぁ",	0, 1},
	{"dwi",	"どぃ",	0, 1},
	{"dwu",	"どぅ",	0, 1},
	{"dwe",	"どぇ",	0, 1},
	{"dwo",	"どぉ",	0, 1},
	{"nya",	"にゃ",	1, 1},
	{"nyi",	"にぃ",	1, 1},
	{"nyu",	"にゅ",	1, 1},
	{"nye",	"にぇ",	1, 1},
	{"nyo",	"にょ",	1, 1},
	{"hya",	"ひゃ",	1, 1},
	{"hyi",	"ひぃ",	1, 1},
	{"hyu",	"ひゅ",	1, 1},
	{"hye",	"ひぇ",	1, 1},
	{"hyo",	"ひょ",	1, 1},
	{"bya",	"びゃ",	1, 1},
	{"byi",	"びぃ",	1, 1},
	{"byu",	"びゅ",	1, 1},
	{"bye",	"びぇ",	1, 1},
	{"byo",	"びょ",	1, 1},
	{"pya",	"ぴゃ",	1, 1},
	{"pyi",	"ぴぃ",	1, 1},
	{"pyu",	"ぴゅ",	1, 1},
	{"pye",	"ぴぇ",	1, 1},
	{"pyo",	"ぴょ",	1, 1},
	{"fa",	"ふぁ",	0, 1},
	{"fi",	"ふぃ",	0, 1},
	{"fe",	"ふぇ",	0, 1},
	{"fo",	"ふぉ",	0, 1},
	{"fya",	"ふゃ",	0, 1},
	{"fyu",	"ふゅ",	0, 1},
	{"fyo",	"ふょ",	0, 1},
	{"hwa",	"ふぁ",	0, 1},
	{"hwi",	"ふぃ",	0, 1},
	{"hwe",	"ふぇ",	0, 1},
	{"hwo",	"ふぉ",	0, 1},
	{"mya",	"みゃ",	1, 1},
	{"myi",	"みぃ",	1, 1},
	{"myu",	"みゅ",	1, 1},
	{"mye",	"みぇ",	1, 1},
	{"myo",	"みょ",	1, 1},
	{"rya",	"りゃ",	1, 1},
	{"ryi",	"りぃ",	1, 1},
	{"ryu",	"りゅ",	1, 1},
	{"rye",	"りぇ",	1, 1},
	{"ryo",	"りょ",	1, 1},
	{"a",	"あ",	1, 1},
	{"i",	"い",	1, 1},
	{"u",	"う",	1, 1},
	{"e",	"え",	1, 1},
	{"o",	"お",	1, 1},
	{"xa",	"ぁ",	0, 1},
	{"xi",	"ぃ",	0, 1},
	{"xu",	"ぅ",	0, 1},
	{"xe",	"ぇ",	0, 1},
	{"xo",	"ぉ",	0, 1},
	{"xxa",	"ぁ",	1, 1},
	{"xxi",	"ぃ",	1, 1},
	{"xxu",	"ぅ",	1, 1},
	{"xxe",	"ぇ",	1, 1},
	{"xxo",	"ぉ",	1, 1},
	{"ye",	"いぇ",	0, 1},
	{"ka",	"か",	1, 1},
	{"ki",	"き",	1, 1},
	{"ku",	"く",	1, 1},
	{"ke",	"け",	1, 1},
	{"ko",	"こ",	1, 1},
	{",a",	"ゕ",	1, 1},
	{",e",	"ゖ",	1, 1},
	{"ga",	"が",	1, 1},
	{"gi",	"ぎ",	1, 1},
	{"gu",	"ぐ",	1, 1},
	{"ge",	"げ",	1, 1},
	{"go",	"ご",	1, 1},
	{"sa",	"さ",	1, 1},
	{"si",	"し",	1, 1},
	{"su",	"す",	1, 1},
	{"se",	"せ",	1, 1},
	{"so",	"そ",	1, 1},
	{"kwa",	"くぁ",	0, 1},
	{"kwi",	"くぃ",	0, 1},
	{"kwu",	"くぅ",	0, 1},
	{"kwe",	"くぇ",	0, 1},
	{"kwo",	"くぉ",	0, 1},
	{"gwa",	"ぐぁ",	0, 1},
	{"gwi",	"ぐぃ",	0, 1},
	{"gwu",	"ぐぅ",	0, 1},
	{"gwe",	"ぐぇ",	0, 1},
	{"gwo",	"ぐぉ",	0, 1},
	{"za",	"ざ",	1, 1},
	{"zi",	"じ",	1, 1},
	{"zu",	"ず",	1, 1},
	{"ze",	"ぜ",	1, 1},
	{"zo",	"ぞ",	1, 1},
	{"ja",	"じゃ",	0, 1},
	{"ji",	"じ",	0, 1},
	{"ju",	"じゅ",	0, 1},
	{"je",	"じぇ",	0, 1},
	{"jo",	"じょ",	0, 1},
	{"jya",	"じゃ",	0, 1},
	{"jyi",	"じぃ",	0, 1},
	{"jyu",	"じゅ",	0, 1},
	{"jye",	"じぇ",	0, 1},
	{"jyo",	"じょ",	0, 1},
	{"ta",	"た",	1, 1},
	{"ti",	"ち",	1, 1},
	{"tu",	"つ",	1, 1},
	{"tsu",	"つ",	0, 1},
	{"te",	"て",	1, 1},
	{"to",	"と",	1, 1},
	{"da",	"だ",	1, 1},
	{"di",	"ぢ",	1, 1},
	{"du",	"づ",	1, 1},
	{"de",	"で",	1, 1},
	{"do",	"ど",	1, 1},
	{"na",	"な",	1, 1},
	{"ni",	"に",	1, 1},
	{"nu",	"ぬ",	1, 1},
	{"ne",	"ね",	1, 1},
	{"no",	"の",	1, 1},
	{"ha",	"は",	1, 1},
	{"hi",	"ひ",	1, 1},
	{"hu",	"ふ",	1, 1},
	{"he",	"へ",	1, 1},
	{"ho",	"ほ",	1, 1},
	{"ba",	"ば",	1, 1},
	{"bi",	"び",	1, 1},
	{"bu",	"ぶ",	1, 1},
	{"be",	"べ",	1, 1},
	{"bo",	"ぼ",	1, 1},
	{"pa",	"ぱ",	1, 1},
	{"pi",	"ぴ",	1, 1},
	{"pu",	"ぷ",	1, 1},
	{"pe",	"ぺ",	1, 1},
	{"po",	"ぽ",	1, 1},
	{"ma",	"ま",	1, 1},
	{"mi",	"み",	1, 1},
	{"mu",	"む",	1, 1},
	{"me",	"め",	1, 1},
	{"mo",	"も",	1, 1},
	{"xx7",	"ゃ",	1, 1},
	{"xx8",	"ゅ",	1, 1},
	{"xx9",	"ょ",	1, 1},
	{"ya",	"や",	1, 1},
	{"yu",	"ゆ",	1, 1},
	{"yo",	"よ",	1, 1},
	{"xxz",	"ゐ",	1, 1},
	{"xxx",	"ゑ",	1, 1},
	{"ra",	"ら",	1, 1},
	{"ri",	"り",	1, 1},
	{"ru",	"る",	1, 1},
	{"re",	"れ",	1, 1},
	{"ro",	"ろ",	1, 1},
	{"xxw",	"ゎ",	1, 1},
	{"xxv",	"ゔ",	1, 1},
	{"wa",	"わ",	1, 1},
	{"wi",	"うぃ",	0, 1},
	{"we",	"うぇ",	0, 1},
	{"wo",	"を",	1, 1},
	{"wha",	"うぁ",	0, 1},
	{"whi",	"うぃ",	0, 1},
	{"whu",	"うぅ",	0, 1},
	{"whe",	"うぇ",	0, 1},
	{"who",	"うぉ",	0, 1},
	{"-",	"ー",	1, 0},
	{NULL,	NULL,	0, 0},
};

const char KEY_INDEX_TO_CHAR_MAP[MAPPABLE_CHAR_COUNT] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
	'-',

	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
	'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
	'_',
};

static unsigned char char_to_key_index_map[128];

size_t char_to_key_index(char ch)
{
	size_t key_index;

	assert(ch < sizeof(char_to_key_index_map));
	assert(ch > 0);

	if (char_to_key_index_map[0])
		return char_to_key_index_map[ch];

	for (key_index = 0; key_index < MAPPABLE_CHAR_COUNT; key_index++)
		char_to_key_index_map[KEY_INDEX_TO_CHAR_MAP[key_index]] =
				key_index;

	return char_to_key_index_map[ch];
}

struct used_bit_map {
	char m[MAPPABLE_CHAR_COUNT * MAPPABLE_CHAR_COUNT];
};

static void get_free_kanji_keys(struct used_bit_map *used)
{
	struct romazi_entry *i;
	memset(used, 0, sizeof(*used));

	for (i = ROMAZI; i->orig; i++) {
		size_t char_i;
		size_t first_key_off;

		if (!i->use_as_is)
			continue;
		first_key_off = char_to_key_index(i->orig[0])
				* MAPPABLE_CHAR_COUNT;
		if (strlen(i->orig) >= 2)
			used->m[first_key_off + char_to_key_index(i->orig[1])] =
				1;
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
		for (key2 = 0; key2 < KANJI_KEY_COUNT; key2++) {
			if (!used.m[key1 * MAPPABLE_CHAR_COUNT + key2])
				u->count[key1]++;
		}
	}
}

void print_free_kanji_keys()
{
	struct used_bit_map used;
	size_t key1;

	get_free_kanji_keys(&used);

	for (key1 = 0; key1 < KANJI_KEY_COUNT; key1++) {
		size_t key2;
		for (key2 = 0; key2 < KANJI_KEY_COUNT; key2++) {
			if (used.m[key1 * MAPPABLE_CHAR_COUNT + key2])
				continue;
			printf("%c%c\n",
			       KEY_INDEX_TO_CHAR_MAP[key1],
			       KEY_INDEX_TO_CHAR_MAP[key2]);
		}
	}
}