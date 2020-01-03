#include "commands.h"
#include "romazi.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static void show_code_for_orig(
	struct key_mapping_array const *mapping, char const *orig)
{
	int i;
	for (i = 0; i < mapping->cnt; i++) {
		struct key_mapping *m = mapping->el + i;
		if (!strcmp(orig, m->orig)) {
			print_mapping(m, out);
			xfputc('\n', out);
		}
	}
}

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

	start_test("set_hiragana_wo_key");
	{
		struct romazi_config config = {
			.hiragana_wo_key = '\'',
		};
		struct key_mapping_array mapping = {0};

		init_romazi(&config);
		get_romazi_codes(&mapping);
		show_code_for_orig(&mapping, "'");

		DESTROY_ARRAY(mapping);
	}
	end_test("'->を\n");

	start_test("include_hya_hyu_hyo");
	{
		struct romazi_config config = {0};
		struct key_mapping_array mapping = {0};

		init_romazi(&config);
		get_romazi_codes(&mapping);

		show_code_for_orig(&mapping, "hya");
		show_code_for_orig(&mapping, "HYA");
		show_code_for_orig(&mapping, "hyu");
		show_code_for_orig(&mapping, "HYU");
		show_code_for_orig(&mapping, "HYO");
		show_code_for_orig(&mapping, "HYI");
		show_code_for_orig(&mapping, "HYE");

		DESTROY_ARRAY(mapping);
	}
	end_test("hya->ひゃ\nHYA->ヒャ\nhyu->ひゅ\nHYU->ヒュ\nHYO->ヒョ\n"
		 "HYI->ヒィ\nHYE->ヒェ\n");

	start_test("include_hya_hyu_hyo");
	{
		struct romazi_config config = {0};
		struct key_mapping_array mapping = {0};

		init_romazi(&config);
		get_romazi_codes(&mapping);

		show_code_for_orig(&mapping, "dya");
		show_code_for_orig(&mapping, "DYA");
		show_code_for_orig(&mapping, "dyu");
		show_code_for_orig(&mapping, "DYU");
		show_code_for_orig(&mapping, "dyo");
		show_code_for_orig(&mapping, "DYO");
		show_code_for_orig(&mapping, "DYI");
		show_code_for_orig(&mapping, "dyi");
		show_code_for_orig(&mapping, "DYE");

		DESTROY_ARRAY(mapping);
	}
	end_test("dya->ぢゃ\nDYA->ヂャ\ndyu->ぢゅ\nDYU->ヂュ\ndyo->ぢょ\n"
		 "DYO->ヂョ\nDYI->ヂィ\ndyi->ぢぃ\nDYE->ヂェ\n");

	return 0;
}
