#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static struct {
	char const *name;
	char const *in;
	char const *out;
} test_cases[] = {
	{"empty", "", ""},
	{"non_kana", "abc ABC", "abc ABC"},
	{"one_hiragana", "ぁ", "ァ"},
	{"non_kana_non_ascii", "日", "日"},
	{"2byte_utf8_char", "ïゖ", "ïヶ"},
	{"2byte_utf8_char_two_kana", "ïゖààが", "ïヶààガ"},
	{"4byte_utf8_char_two_kana", "𠮟ゖ𪎌わ", "𠮟ヶ𪎌ワ"},
};

int main(void)
{
	int test_i;

	set_test_source_file(__FILE__);

	for (test_i = 0; test_i < sizeof(test_cases) / sizeof(*test_cases);
	     test_i++) {
		int exit_code;

		in = open_tmp_file_containing(test_cases[test_i].in);

		start_test(test_cases[test_i].name);
		exit_code = h2k(0, 0);
		if (exit_code)
			fprintf(out, "non-zero exit: %d\n", exit_code);
		end_test(test_cases[test_i].out);

		XFCLOSE(in);
		in = stdin;
	}

	return 0;
}
