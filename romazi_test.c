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
			fputc('\n', out);
		}
	}
}

static void print_code_length(
	struct key_mapping_array const *mapping, const char *conv)
{
	int i;
	for (i = 0; i < mapping->cnt; i++) {
		if (strcmp(mapping->el[i].conv, conv))
			continue;

		fprintf(out, "「%s」入力コードの長さ: %zu\n",
			 conv, strlen(mapping->el[i].orig));
	}
}

int main(void)
{
	set_test_source_file(__FILE__);

	while (run_test("free_kanji_keys_output", NULL)) {
		free_kanji_keys(NULL, 0);
	}

	while (run_test("free_kanji_keys_output_short_shifted_codes", NULL)) {
		char *argv[] = {"--short-shifted-codes"};
		free_kanji_keys(argv, 1);
	}

	while (run_test("hiragana_to_katakana", "")) {
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
				fprintf(err,
					 "input: %s\nexpected: %s\nactual:%s\n",
					 test_cases[i].input,
					 test_cases[i].output, scratch);
		}
	}

	while (run_test("free_kanji_keys_no_kanji_numerals", NULL)) {
		char *args[] = {"--no-kanji-nums", "--short-shifted-codes"};
		free_kanji_keys(args, 2);
	}

	while (run_test("free_kanji_keys_missing_wo_key_arg",
			"フラグを認識できませんでした：--hiragana-wo-key\n"
			 "exit code: 200\n")) {
		char *args[] = {"--hiragana-wo-key", "?"};
		/* 引数の数 (argc) が足りないため、「?」を解析しないべき。*/
		fprintf(err, "exit code: %d\n", free_kanji_keys(args, 1));
	}

	while (run_test("free_kanji_keys_wo_key_arg_too_short",
			"フラグを認識できませんでした：--hiragana-wo-key\n"
			 "exit code: 200\n")) {
		char *args[] = {"--hiragana-wo-key", ""};
		fprintf(err, "exit code: %d\n", free_kanji_keys(args, 2));
	}

	while (run_test("free_kanji_keys_wo_key_arg_too_long",
			"フラグを認識できませんでした：--hiragana-wo-key\n"
			 "exit code: 200\n")) {
		char *args[] = {"--hiragana-wo-key", "xy"};
		fprintf(err, "exit code: %d\n", free_kanji_keys(args, 2));
	}

	while (run_test("set_hiragana_wo_key", "'->を\n")) {
		struct romazi_config config = {
			.hiragana_wo_key = '\'',
		};
		struct key_mapping_array mapping = {0};

		get_romazi_codes(&config, &mapping);
		show_code_for_orig(&mapping, "'");

		DESTROY_ARRAY(mapping);
	}

	while (run_test("include_hya_hyu_hyo",
			"hya->ひゃ\nHYA->ヒャ\nhyu->ひゅ\nHYU->ヒュ\n"
			"HYO->ヒョ\nHYI->ヒィ\nHYE->ヒェ\n")) {
		struct romazi_config config = {0};
		struct key_mapping_array mapping = {0};

		get_romazi_codes(&config, &mapping);

		show_code_for_orig(&mapping, "hya");
		show_code_for_orig(&mapping, "HYA");
		show_code_for_orig(&mapping, "hyu");
		show_code_for_orig(&mapping, "HYU");
		show_code_for_orig(&mapping, "HYO");
		show_code_for_orig(&mapping, "HYI");
		show_code_for_orig(&mapping, "HYE");

		DESTROY_ARRAY(mapping);
	}

	while (run_test("include_hya_hyu_hyo",
			"dya->ぢゃ\nDYA->ヂャ\ndyu->ぢゅ\nDYU->ヂュ\ndyo->ぢょ\n"
			"DYO->ヂョ\nDYI->ヂィ\ndyi->ぢぃ\nDYE->ヂェ\n")) {
		struct romazi_config config = {0};
		struct key_mapping_array mapping = {0};

		get_romazi_codes(&config, &mapping);

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

	while (run_test("optimize_keystrokes", NULL)) {
		struct romazi_config config = { .optimize_keystrokes = 1 };
		struct key_mapping_array mapping = {0};

		get_romazi_codes(&config, &mapping);

		print_code_length(&mapping, "た");
		print_code_length(&mapping, "し");
		print_code_length(&mapping, "は");
		print_code_length(&mapping, "な");
		print_code_length(&mapping, "に");
		print_code_length(&mapping, "の");
		print_code_length(&mapping, "い");
		print_code_length(&mapping, "れ");
		print_code_length(&mapping, "る");
		print_code_length(&mapping, "と");
		print_code_length(&mapping, "タ");
		print_code_length(&mapping, "シ");
		print_code_length(&mapping, "ハ");
		print_code_length(&mapping, "ナ");
		print_code_length(&mapping, "ニ");
		print_code_length(&mapping, "ノ");
		print_code_length(&mapping, "イ");
		print_code_length(&mapping, "レ");
		print_code_length(&mapping, "ル");
		print_code_length(&mapping, "ト");

		print_code_length(&mapping, "あ");
		print_code_length(&mapping, "う");
		print_code_length(&mapping, "え");
		print_code_length(&mapping, "お");
		print_code_length(&mapping, "ア");
		print_code_length(&mapping, "ウ");
		print_code_length(&mapping, "エ");
		print_code_length(&mapping, "オ");

		expect_ok(sort_and_validate_no_conflicts(&mapping));

		DESTROY_ARRAY(mapping);
	}

	while (run_test("parse_optimize_keystroke_flag", "END 0 1\n")) {
		int argc = 1;
		char *argv[] = {"--romazi-optimize-keystrokes", "END"};
		char **argv_ptr = argv;

		struct romazi_config config = {0};
		if (!parse_romazi_flags(&argc, &argv_ptr, &config))
			fputs("解析に失敗\n", out);
		fprintf(out, "%s %d %d\n",
			 argv_ptr[0], argc, config.optimize_keystrokes);
	}

	return 0;
}
