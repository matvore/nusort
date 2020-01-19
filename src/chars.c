#include <string.h>

#include "chars.h"
#include "util.h"

int codepoint_range(char const *c) {
	if (strcmp(c, "ぁ") < 0)
		return CODEPOINT_RANGE_OTHER;
	if (strcmp(c, "ゖ") <= 0)
		return CODEPOINT_RANGE_HIRAGANA;
	if (strcmp(c, "ァ") < 0)
		return CODEPOINT_RANGE_OTHER;
	if (strcmp(c, "ヶ") <= 0)
		return CODEPOINT_RANGE_KATAKANA;
	if (strcmp(c, "ー") <= 0)
		return CODEPOINT_RANGE_KATAKANA_SPECIAL;
	return CODEPOINT_RANGE_OTHER;
}

int is_complete_utf8(char first_char, int size)
{
	switch (size) {
	case 1:
		return !(first_char & 0x80);
	case 2:
		return 0xc0 == (first_char & 0xe0);
	case 3:
		return 0xe0 == (first_char & 0xf0);
	case 4:
		return 1;
	default:
		DIE(0, "size が無効です");
	}
}
