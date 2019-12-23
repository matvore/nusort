#include "chars.h"
#include "commands.h"
#include "romazi.h"
#include "util.h"

static int is_complete_utf8(char first_char, int size)
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
		BUG("size が無効です");
	}
}

int h2k(char const *const *argv, int argc)
{
	char buf[4] = {0};
	int size = 0;

	if (argc) {
		xfputs("引数を渡さないでください。\n", err);
		return 92;
	}

	while (1) {
		int c = xfgetc(in);

		if (c == -1)
			break;

		buf[size++] = c;
		if (is_complete_utf8(buf[0], size)) {
			if (codepoint_range(buf) == CODEPOINT_RANGE_HIRAGANA)
				hiragana_to_katakana(buf);
			xfwrite(buf, size, out);
			size = 0;
			memset(buf, 0, sizeof(buf));
		}
	}

	return 0;
}
