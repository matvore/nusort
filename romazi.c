#include "romazi.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

struct romazi_entry {
	const char *orig;
	const char *conv;
	unsigned katakana_only : 1;
};

struct romazi_entry ROMAZI[] = {
	{"va",	"ゔぁ",	1},
	{"vi",	"ゔぃ",	1},
	{"vu",	"ゔ",	1},
	{"ve",	"ゔぇ",	1},
	{"vo",	"ゔぉ",	1},
	{"vya",	"ゔゃ",	1},
	{"vyi",	"ゔぃ",	1},
	{"vyu",	"ゔゅ",	1},
	{"vye",	"ゔぇ",	1},
	{"vyo",	"ゔょ",	1},
	{"q",	"っ",	0},
	{"l",	"ん",	0},
	{"kya",	"きゃ",	0},
	{"kyi",	"きぃ",	0},
	{"kyu",	"きゅ",	0},
	{"kye",	"きぇ",	0},
	{"kyo",	"きょ",	0},
	{"gya",	"ぎゃ",	0},
	{"gyi",	"ぎぃ",	0},
	{"gyu",	"ぎゅ",	0},
	{"gye",	"ぎぇ",	0},
	{"gyo",	"ぎょ",	0},
	{"sya",	"しゃ",	0},
	{"syi",	"しぃ",	0},
	{"syu",	"しゅ",	0},
	{"sye",	"しぇ",	0},
	{"syo",	"しょ",	0},
	{"zya",	"じゃ",	0},
	{"zyi",	"じぃ",	0},
	{"zyu",	"じゅ",	0},
	{"zye",	"じぇ",	0},
	{"zyo",	"じょ",	0},
	{"tya",	"ちゃ",	0},
	{"tyi",	"ちぃ",	0},
	{"tyu",	"ちゅ",	0},
	{"tye",	"ちぇ",	0},
	{"tyo",	"ちょ",	0},
	{"dya",	"ぢゃ",	1},
	{"dyi",	"ぢぃ",	1},
	{"dyu",	"ぢゅ",	1},
	{"dye",	"ぢぇ",	1},
	{"dyo",	"ぢょ",	1},
	{"tsa",	"つぁ",	1},
	{"tsi",	"つぃ",	1},
	{"tse",	"つぇ",	1},
	{"tso",	"つぉ",	1},
	{"tha",	"てゃ",	1},
	{"thi",	"てぃ",	1},
	{"thu",	"てゅ",	1},
	{"the",	"てぇ",	1},
	{"tho",	"てょ",	1},
	{"dha",	"でゃ",	1},
	{"dhi",	"でぃ",	1},
	{"dhu",	"でゅ",	1},
	{"dhe",	"でぇ",	1},
	{"dho",	"でょ",	1},
	{"twa",	"とぁ",	1},
	{"twi",	"とぃ",	1},
	{"twu",	"とぅ",	1},
	{"twe",	"とぇ",	1},
	{"two",	"とぉ",	1},
	{"dwa",	"どぁ",	1},
	{"dwi",	"どぃ",	1},
	{"dwu",	"どぅ",	1},
	{"dwe",	"どぇ",	1},
	{"dwo",	"どぉ",	1},
	{"nya",	"にゃ",	0},
	{"nyi",	"にぃ",	0},
	{"nyu",	"にゅ",	0},
	{"nye",	"にぇ",	0},
	{"nyo",	"にょ",	0},
	{"hya",	"ひゃ",	0},
	{"hyi",	"ひぃ",	0},
	{"hyu",	"ひゅ",	0},
	{"hye",	"ひぇ",	0},
	{"hyo",	"ひょ",	0},
	{"bya",	"びゃ",	0},
	{"byi",	"びぃ",	0},
	{"byu",	"びゅ",	0},
	{"bye",	"びぇ",	0},
	{"byo",	"びょ",	0},
	{"pya",	"ぴゃ",	0},
	{"pyi",	"ぴぃ",	0},
	{"pyu",	"ぴゅ",	0},
	{"pye",	"ぴぇ",	0},
	{"pyo",	"ぴょ",	0},
	{"fa",	"ふぁ",	1},
	{"fi",	"ふぃ",	1},
	{"fe",	"ふぇ",	1},
	{"fo",	"ふぉ",	1},
	{"fya",	"ふゃ",	1},
	{"fyu",	"ふゅ",	1},
	{"fyo",	"ふょ",	1},
	{"hwa",	"ふぁ",	1},
	{"hwi",	"ふぃ",	1},
	{"hwe",	"ふぇ",	1},
	{"hwo",	"ふぉ",	1},
	{"mya",	"みゃ",	0},
	{"myi",	"みぃ",	0},
	{"myu",	"みゅ",	0},
	{"mye",	"みぇ",	0},
	{"myo",	"みょ",	0},
	{"rya",	"りゃ",	0},
	{"ryi",	"りぃ",	0},
	{"ryu",	"りゅ",	0},
	{"rye",	"りぇ",	0},
	{"ryo",	"りょ",	0},
	{"a",	"あ",	0},
	{"i",	"い",	0},
	{"u",	"う",	0},
	{"e",	"え",	0},
	{"o",	"お",	0},
	{"xa",	"ぁ",	1},
	{"xi",	"ぃ",	1},
	{"xu",	"ぅ",	1},
	{"xe",	"ぇ",	1},
	{"xo",	"ぉ",	1},
	{"xxa",	"ぁ",	0},
	{"xxi",	"ぃ",	0},
	{"xxu",	"ぅ",	0},
	{"xxe",	"ぇ",	0},
	{"xxo",	"ぉ",	0},
	{"ye",	"いぇ",	1},
	{"ka",	"か",	0},
	{"ki",	"き",	0},
	{"ku",	"く",	0},
	{"ke",	"け",	0},
	{"ko",	"こ",	0},
	{",a",	"ヵ",	0},
	{",e",	"ヶ",	0},
	{"ga",	"が",	0},
	{"gi",	"ぎ",	0},
	{"gu",	"ぐ",	0},
	{"ge",	"げ",	0},
	{"go",	"ご",	0},
	{"sa",	"さ",	0},
	{"si",	"し",	0},
	{"su",	"す",	0},
	{"se",	"せ",	0},
	{"so",	"そ",	0},
	{"kwa",	"くぁ",	1},
	{"kwi",	"くぃ",	1},
	{"kwu",	"くぅ",	1},
	{"kwe",	"くぇ",	1},
	{"kwo",	"くぉ",	1},
	{"gwa",	"ぐぁ",	1},
	{"gwi",	"ぐぃ",	1},
	{"gwu",	"ぐぅ",	1},
	{"gwe",	"ぐぇ",	1},
	{"gwo",	"ぐぉ",	1},
	{"za",	"ざ",	0},
	{"zi",	"じ",	0},
	{"zu",	"ず",	0},
	{"ze",	"ぜ",	0},
	{"zo",	"ぞ",	0},
	{"ja",	"じゃ",	1},
	{"ji",	"じ",	1},
	{"ju",	"じゅ",	1},
	{"je",	"じぇ",	1},
	{"jo",	"じょ",	1},
	{"jya",	"じゃ",	1},
	{"jyi",	"じぃ",	1},
	{"jyu",	"じゅ",	1},
	{"jye",	"じぇ",	1},
	{"jyo",	"じょ",	1},
	{"ta",	"た",	0},
	{"ti",	"ち",	0},
	{"tu",	"つ",	0},
	{"tsu",	"つ",	1},
	{"te",	"て",	0},
	{"to",	"と",	0},
	{"da",	"だ",	0},
	{"di",	"ぢ",	0},
	{"du",	"づ",	0},
	{"de",	"で",	0},
	{"do",	"ど",	0},
	{"na",	"な",	0},
	{"ni",	"に",	0},
	{"nu",	"ぬ",	0},
	{"ne",	"ね",	0},
	{"no",	"の",	0},
	{"ha",	"は",	0},
	{"hi",	"ひ",	0},
	{"hu",	"ふ",	0},
	{"he",	"へ",	0},
	{"ho",	"ほ",	0},
	{"ba",	"ば",	0},
	{"bi",	"び",	0},
	{"bu",	"ぶ",	0},
	{"be",	"べ",	0},
	{"bo",	"ぼ",	0},
	{"pa",	"ぱ",	0},
	{"pi",	"ぴ",	0},
	{"pu",	"ぷ",	0},
	{"pe",	"ぺ",	0},
	{"po",	"ぽ",	0},
	{"ma",	"ま",	0},
	{"mi",	"み",	0},
	{"mu",	"む",	0},
	{"me",	"め",	0},
	{"mo",	"も",	0},
	{"xx7",	"ゃ",	0},
	{"xx8",	"ゅ",	0},
	{"xx9",	"ょ",	0},
	{"ya",	"や",	0},
	{"yu",	"ゆ",	0},
	{"yo",	"よ",	0},
	{"xxz",	"ゐ",	0},
	{"xxx",	"ゑ",	0},
	{"ra",	"ら",	0},
	{"ri",	"り",	0},
	{"ru",	"る",	0},
	{"re",	"れ",	0},
	{"ro",	"ろ",	0},
	{"xxw",	"ゎ",	0},
	{"xxv",	"ゔ",	0},
	{"wa",	"わ",	0},
	{"wi",	"うぃ",	1},
	{"we",	"うぇ",	1},
	{"wo",	"を",	0},
	{"wha",	"うぁ",	1},
	{"whi",	"うぃ",	1},
	{"whu",	"うぅ",	1},
	{"whe",	"うぇ",	1},
	{"who",	"うぉ",	1},
	{"-",	"ー",	0},
	{NULL,	NULL,	0},
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

void print_free_kanji_keys()
{
	char used[MAPPABLE_CHAR_COUNT * MAPPABLE_CHAR_COUNT];
	struct romazi_entry *i;
	size_t key1;

	memset(used, 0, sizeof(used));
	for (i = ROMAZI; i->orig; i++) {
		size_t char_i;
		size_t first_key_off;

		if (i->katakana_only)
			continue;
		first_key_off = char_to_key_index(i->orig[0])
				* MAPPABLE_CHAR_COUNT;
		if (strlen(i->orig) >= 2)
			used[first_key_off + char_to_key_index(i->orig[1])] = 1;
		else
			memset(used + first_key_off, 1, MAPPABLE_CHAR_COUNT);
	}

	for (key1 = 0; key1 < 40; key1++) {
		size_t key2;
		for (key2 = 0; key2 < 40; key2++) {
			if (used[key1 * MAPPABLE_CHAR_COUNT + key2])
				continue;
			printf("%c%c\n",
			       KEY_INDEX_TO_CHAR_MAP[key1],
			       KEY_INDEX_TO_CHAR_MAP[key2]);
		}
	}
}

int main(int argc, char **argv)
{
	print_free_kanji_keys();
}
