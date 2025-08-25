#include "commands.h"
#include "input_impl.h"
#include "mapping.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static struct flagset fs;
static struct mapping m;

static void initflags(void)
{
	input(&fs, 0, -1);
	fs.not_collecting = 1;
}

static void cleanup(void)
{
	XFCLOSE(in);		in = stdin;
	DESTROY_ARRAY(fs);	memset(&fs, 0, sizeof fs);
	destroy_mapping(&m);	memset(&m, 0, sizeof m);
}

int main(void)
{
	set_test_source_file(__FILE__);
	config_tests(CONFIG_TESTS_IGNORE_NULL_BYTES);
	debugout = 1;

	while (run_test("first_key_k", NULL)) {
		char *argv[] = {
			"--6rh",
			"--short-shifted-codes",
			"--no-show-cutoff-guide",
			"--no-show-rsc-list",
			"--no-real-tty",
		};
		int argc = 5;

		in = open_tmp_file_containing("kj");
		expect_ok(input(&fs, argv, argc));
		cleanup();
	}

	while (run_test("long_conv_strs", NULL)) {
		char *argv[] = {
			"--6rh", "--short-shifted-codes",
			"--no-show-cutoff-guide",
			"--no-show-rsc-list",
			"--no-real-tty",
		};
		int argc = 5;
		in = open_tmp_file_containing("tya" "HWI" "DWO" "WHO" "YE");
		expect_ok(input(&fs, argv, argc));
		cleanup();
	}

	while (run_test("busy_right_pinky", NULL)) {
		char *argv[] = {
			"--busy-right-pinky", "--no-show-cutoff-guide",
			"--no-real-tty",
		};
		int argc = 3;
		in = open_tmp_file_containing("k[" "r'" "9]");
		expect_ok(input(&fs, argv, argc));
		cleanup();
	}

	while (run_test("possible_code_requires_two_more_chars", NULL)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-pending-and-converted", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "xyz", "あ");
		in = open_tmp_file_containing("x!");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("parse_romazi_flags_no_kanji_nums", NULL)) {
		char *argv[] = {
			"--no-kanji-nums", "--6rh", "--short-shifted-codes",
			"--no-show-cutoff-guide",
			"--no-show-rsc-list",
			"--no-real-tty",
		};
		int argc = 6;

		in = open_tmp_file_containing("");
		expect_ok(input(&fs, argv, argc));
		cleanup();
	}

	while (run_test("exclude_kanji", NULL)) {
		char *argv[] = {
			"--no-kanji", "--no-show-cutoff-guide",
			"--no-show-rsc-list", "--no-real-tty",
		};
		int argc = 4;

		in = open_tmp_file_containing("m");
		expect_ok(input(&fs, argv, argc));
		cleanup();
	}

	while (run_test("show_pending_conversion", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);

		append_mapping(&m.arr, "mo", "も");

		in = open_tmp_file_containing("m");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}

	while (run_test("show_already_converted", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ki", "き");

		in = open_tmp_file_containing("ki");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}

	while (run_test("accumulates_multiple_converted", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "pa", "ぱ");
		sort_and_validate_no_conflicts(&m.arr);

		in = open_tmp_file_containing("ropa");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}

	while (run_test("backspace_on_empty_line_does_nothing", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ro", "ろ");
		in = open_tmp_file_containing("\b\b\b\b");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("ascii_del_on_empty_line_does_nothing", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ro", "ろ");
		in = open_tmp_file_containing("\x7f\x7f\x7f\x7f");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("ascii_del_to_remove_pending_conversion", 0)) {
		initflags();
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ba", "ば");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("r\x7f""b");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("backspace_to_remove_pending_conversion", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ba", "ば");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("r\bb");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("backspace_to_remove_pending_conv_one_char_at_a_time", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ryo", "りょ");
		append_mapping(&m.arr, "sya", "しゃ");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("ry\b\bsya");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("backspace_to_remove_converted_char", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "wa", "わ");
		in = open_tmp_file_containing("wa\b");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("backspace_to_remove_converted_char_one_at_a_time",
			0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "wa", "わ");
		append_mapping(&m.arr, "ha", "は");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("wahaha\b\bwawa");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("invalid_prefix_leaks_out_of_pending_conv", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ma", "ま");
		in = open_tmp_file_containing("x");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("invalid_prefix_leaks_one_char_at_a_time", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ma", "ま");
		append_mapping(&m.arr, "xa", "ぁ");
		in = open_tmp_file_containing("mx");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("invalid_prefix_leaks_two_chars_at_a_time", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ma", "ま");
		append_mapping(&m.arr, "xa", "ぁ");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("x?");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("leak_invalid_prefix_then_immediately_convert", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ka", "か");
		append_mapping(&m.arr, "J", "ッ");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("kJ");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("delete_converted_ascii_char", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ka", "か");
		in = open_tmp_file_containing("kr\b");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("delete_converted_ascii_char_with_prior_kana_char", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ka", "か");
		in = open_tmp_file_containing("kakr\b");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("delete_converted_ascii_char_with_prior_kana_char_2", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ka", "か");
		in = open_tmp_file_containing("kakb\b");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("delete_converted_2_byte_char", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "dmf", "é");
		in = open_tmp_file_containing("dmf\b");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("can_input_four_key_code", 0)) {
		initflags();
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		setflag(&fs, "--recurs-ksort", 1);

		expect_ok(mapping_populate(&fs, &m));
		in = open_tmp_file_containing("1 jf");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("show_candidates_for_four_key_code", NULL)) {
		initflags();
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--no-show-pending-and-converted", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);

		append_mapping(&m.arr, "1 jf", "乱");
		append_mapping(&m.arr, "1 jd", "乾");
		append_mapping(&m.arr, "1 js", "亀");
		append_mapping(&m.arr, "d jf", "燥");
		append_mapping(&m.arr, "d jd", "爪");
		append_mapping(&m.arr, "d js", "燻");
		append_mapping(&m.arr, "9 qj", "巨");
		append_mapping(&m.arr, "9 qk", "巧");
		append_mapping(&m.arr, "9 ql", "巾");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing("1 jdd jf9 q");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("show_cutoff_guide", NULL)) {
		initflags();
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-pending-and-converted", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);

		expect_ok(mapping_populate(&fs, &m));
		in = open_tmp_file_containing("y \b\bu ");
		expect_ok(input_impl(&m, &fs));
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		cleanup();
	}

	while (run_test("show_cutoff_then_pending_and_converted_then_keyboard", NULL)) {
		initflags();
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		setflag(&fs, "--recurs-ksort", 1);
		expect_ok(mapping_populate(&fs, &m));
		in = open_tmp_file_containing("t ");
		expect_ok(input_impl(&m, &fs));
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		cleanup();
	}

	while (run_test("enter_clears_input_line", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ba", "ば");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing("ro\n");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("does_not_remember_trailing_characters_after_enter", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ba", "ば");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing("ba\nj");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("enter_before_typing_anything_does_not_crash", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ba", "ば");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing("\n");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("use_osc52_to_save_include_kanji", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-pending-and-converted", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		append_mapping(&m.arr, "a", "て");
		append_mapping(&m.arr, "b", "す");
		append_mapping(&m.arr, "c", "と");
		append_mapping(&m.arr, "d", "成");
		append_mapping(&m.arr, "e", "功");

		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing("abcde\n");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("use_osc52_to_save_with_padding", 0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-pending-and-converted", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		append_mapping(&m.arr, "a", "あ");
		append_mapping(&m.arr, "i", "い");
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ha", "は");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing(
			/* 出力が 6 bytes */
			"ai\n"
			/* 出力が 7 bytes - 要パディング */
			"aiu\n"
			/* 出力が 8 bytes - 要パディング */
			"AiroN\n"
			/* 出力が 9 bytes */
			"aroha\n"
		);
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("use_osc52_to_save_use_plus_and_slash_in_output",
			0)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-pending-and-converted", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		append_mapping(&m.arr, "a", "あ");
		append_mapping(&m.arr, "i", "い");
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ha", "は");
		append_mapping(&m.arr, "#1", "\xce\x9f");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing(
			"xy>\n"
			"jk?\n"
			"#1#1\n"
			"3ro\n"
		);
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("show_rsc_list", NULL)) {
		initflags();
		setflag(&fs, "--no-kanji", 1);
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-pending-and-converted", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-cutoff-guide", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		append_mapping(&m.arr, "yj", "肉");
		append_mapping(&m.arr, "yk", "肘");
		append_mapping(&m.arr, "yh", "漢");
		append_mapping(&m.arr, "yl", "滝");
		append_mapping(&m.arr, "y;", "方");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing("y");
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("kugiri_share_same_rsc_sort_key_mizu_and_iu", NULL)) {
		initflags();
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-pending-and-converted", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		setflag(&fs, "--recurs-ksort", 1);
		in = open_tmp_file_containing("s \b\bc ");
		expect_ok(mapping_populate(&fs, &m));
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("cutoff_guide_remains_resid_sc_3rd_key", NULL)) {
		initflags();
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		in = open_tmp_file_containing("k dk");
		expect_ok(mapping_populate(&fs, &m));
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	while (run_test("cutoff_guide_remains_recurs_ksort", NULL)) {
		initflags();
		setflag(&fs, "--recurs-ksort", 1);
		setflag(&fs, "--no-show-keyboard", 1);
		setflag(&fs, "--no-show-rsc-list", 1);
		setflag(&fs, "--no-real-tty", 1);
		setflag(&fs, "--no-save-with-osc52", 1);
		setflag(&fs, "--recurs-ksort", 1);
		in = open_tmp_file_containing("k dk");
		expect_ok(mapping_populate(&fs, &m));
		expect_ok(input_impl(&m, &fs));
		cleanup();
	}

	return 0;
}
