#include "commands.h"
#include "romazi.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	start_test(__FILE__, "free_kanji_keys_output");
	{
		free_kanji_keys(NULL, 0);
	}
	end_test_expected_content_in_file();

	start_test(__FILE__, "hiragana_to_katakana");
	{
		struct {
			Conv input;
			Conv output;
		} test_cases[] = {
			{"",		""},
			{"あ",		"ア"},
			{"きゃ",	"キャ"},
			{"じょ",	"ジョ"},
			{"っち",	"ッチ"},
			{"ゖ",		"ヶ"},
			{"ぁ",		"ァ"},
		};
		int i;

		for (i = 0; i < sizeof(test_cases) / sizeof(*test_cases); i++) {
			Conv scratch;
			memcpy(scratch, test_cases[i].input, sizeof(scratch));
			hiragana_to_katakana(scratch);
			if (strcmp(scratch, test_cases[i].output))
				xfprintf(err,
					 "input: %s\nexpected: %s\nactual:%s\n",
					 test_cases[i].input,
					 test_cases[i].output, scratch);
		}
	}
	end_test("");

	return 0;
}
