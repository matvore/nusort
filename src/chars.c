#include <string.h>

#include "chars.h"

int codepoint_range(char const *c) {
	if (strcmp(c, "ぁ") < 0)
		return CODEPOINT_RANGE_OTHER;
	if (strcmp(c, "ゖ") <= 0)
		return CODEPOINT_RANGE_HIRAGANA;
	if (strcmp(c, "ァ") < 0)
		return CODEPOINT_RANGE_OTHER;
	if (strcmp(c, "ヶ") <= 0)
		return CODEPOINT_RANGE_KATAKANA;
	return CODEPOINT_RANGE_OTHER;
}
