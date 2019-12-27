#include "commands.h"
#include "romazi.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("free_kanji_keys_output");
	{
		free_kanji_keys(NULL, 0);
	}
	end_test_expected_content_in_file();

	start_test("hiragana_to_katakana");
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

	start_test("free_kanji_keys_no_kanji_numerals");
	{
		char const *args[] = {"--no-kanji-nums"};
		free_kanji_keys(args, 1);
	}
	end_test_expected_content_in_file();

	start_test("free_kanji_keys_missing_wo_key_arg");
	{
		char const *args[] = {"--hiragana-wo-key", "?"};
		/* 引数の数 (argc) が足りないため、「?」を解析しないべき。*/
		xfprintf(err, "exit code: %d\n", free_kanji_keys(args, 1));
	}
	end_test("フラグを認識できませんでした：--hiragana-wo-key\n"
		 "exit code: 200\n");

	start_test("free_kanji_keys_wo_key_arg_too_short");
	{
		char const *args[] = {"--hiragana-wo-key", ""};
		xfprintf(err, "exit code: %d\n", free_kanji_keys(args, 2));
	}
	end_test("フラグを認識できませんでした：--hiragana-wo-key\n"
		 "exit code: 200\n");

	start_test("free_kanji_keys_wo_key_arg_too_long");
	{
		char const *args[] = {"--hiragana-wo-key", "xy"};
		xfprintf(err, "exit code: %d\n", free_kanji_keys(args, 2));
	}
	end_test("フラグを認識できませんでした：--hiragana-wo-key\n"
		 "exit code: 200\n");

	start_test("init_returns_1_on_success");
	{
		struct romazi_config config = {
			.hiragana_wo_key = '\'',
		};
		xfprintf(err, "status: %d",
			 init_romazi_and_return_status(&config));
	}
	end_test("status: 1");

	start_test("do_not_allow_overlap_with_one_wo_key");
	{
		struct romazi_config config = {
			.hiragana_wo_key = 'k',
		};
		xfprintf(err, "status: %d",
			 init_romazi_and_return_status(&config));
	}
	end_test("ダブっている入力コードがあります。\nstatus: 0");

	start_test("do_not_allow_overlap_with_one_wo_key_2");
	{
		struct romazi_config config = {
			.hiragana_wo_key = 'x',
		};
		xfprintf(err, "status: %d",
			 init_romazi_and_return_status(&config));
	}
	end_test("ダブっている入力コードがあります。\nstatus: 0");

	start_test("do_not_allow_overlap_with_one_wo_key_3");
	{
		struct romazi_config config = {
			.hiragana_wo_key = 'p',
		};
		xfprintf(err, "status: %d",
			 init_romazi_and_return_status(&config));
	}
	end_test("ダブっている入力コードがあります。\nstatus: 0");

	return 0;
}
