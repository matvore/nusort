#include "chars.h"
#include "commands.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

int h2k(char **argv, int argc)
{
	char buf[4] = {0};
	int size = 0;

	if (argc) {
		fputs("引数を渡さないでください。\n", err);
		return 92;
	}

	while (1) {
		int c = fgetc(in);

		if (c == -1)
			break;

		buf[size++] = c;
		if (is_complete_utf8(buf[0], size)) {
			if (codepoint_range(buf) == CODEPOINT_RANGE_HIRAGANA)
				hiragana_to_katakana(buf);
			fwrite(buf, size, 1, out);
			size = 0;
			memset(buf, 0, sizeof(buf));
		}
	}

	return 0;
}
