#include "commands.h"
#include "input_impl.h"
#include "mapping.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	set_test_source_file(__FILE__);
	config_tests(CONFIG_TESTS_IGNORE_NULL_BYTES);

	while (run_test("unrecognized_argument",
			"フラグを認識できませんでした：--nonsense-arg\n"
			"exit code: 3\n")) {
		char const *argv[] = {"--nonsense-arg"};
		int argc = 1;

		fprintf(out, "exit code: %d\n",
			 input(argv, argc, /*set_raw_mode=*/0));
	}

	while (run_test("first_key_k", NULL)) {
		char const *argv[] = {
			"-s", "--short-shifted-codes",
			"--no-show-cutoff-guide",
			"--no-show-rsc-list",
		};
		int argc = 4;
		in = open_tmp_file_containing("kj");
		expect_ok(input(argv, argc, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}

	while (run_test("long_conv_strs", NULL)) {
		char const *argv[] = {
			"-s", "--short-shifted-codes",
			"--no-show-cutoff-guide",
			"--no-show-rsc-list",
		};
		int argc = 4;
		in = open_tmp_file_containing("tya" "HWI" "DWO" "WHO" "YE");
		expect_ok(input(argv, argc, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}

	while (run_test("busy_right_pinky", NULL)) {
		char const *argv[] = {
			"--busy-right-pinky", "--no-show-cutoff-guide",
		};
		int argc = 2;
		in = open_tmp_file_containing("k[" "r'" "9]");
		expect_ok(input(argv, argc, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}

	while (run_test("possible_code_requires_two_more_chars", NULL)) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_keyboard = 1,
		};
		append_mapping(&m.arr, "xyz", "あ");
		in = open_tmp_file_containing("x!");
		expect_ok(input_impl(&m, &f));
		XFCLOSE(in);
		destroy_mapping(&m);
	}

	while (run_test("parse_romazi_flags_no_kanji_nums", NULL)) {
		char const *argv[] = {
			"--no-kanji-nums", "-s", "--short-shifted-codes",
			"--no-show-cutoff-guide",
			"--no-show-rsc-list",
		};
		int argc = 5;

		in = open_tmp_file_containing("");
		expect_ok(input(argv, argc, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}

	while (run_test("exclude_kanji", NULL)) {
		char const *argv[] = {
			"--no-kanji", "--no-show-cutoff-guide",
			"--no-show-rsc-list",
		};
		int argc = 3;

		in = open_tmp_file_containing("m");
		expect_ok(input(argv, argc, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}

	while (run_test("show_pending_conversion", "<m>\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "mo", "も");

		in = open_tmp_file_containing("m");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("show_already_converted",
			"<k>\n"
			"き\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ki", "き");

		in = open_tmp_file_containing("ki");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("accumulates_multiple_converted", "<r>\nろ\nろ<p>\nろぱ\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "pa", "ぱ");
		sort_and_validate_no_conflicts(&m.arr);

		in = open_tmp_file_containing("ropa");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("backspace_on_empty_line_does_nothing", "")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		in = open_tmp_file_containing("\b\b\b\b");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("ascii_del_on_empty_line_does_nothing", "")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		in = open_tmp_file_containing("\x7f\x7f\x7f\x7f");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("ascii_del_to_remove_pending_conversion", "<r>\n<>\n<b>\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ba", "ば");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("r\x7f""b");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("backspace_to_remove_pending_conversion", "<r>\n<>\n<b>\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ba", "ば");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("r\bb");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("backspace_to_remove_pending_conv_one_char_at_a_time", "<r>\n<ry>\n<r>\n<>\n<s>\n<sy>\nしゃ\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ryo", "りょ");
		append_mapping(&m.arr, "sya", "しゃ");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("ry\b\bsya");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("backspace_to_remove_converted_char", "<w>\nわ\n\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "wa", "わ");
		in = open_tmp_file_containing("wa\b");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("backspace_to_remove_converted_char_one_at_a_time",
			"<w>\n"
			"わ\n"
			"わ<h>\n"
			"わは\n"
			"わは<h>\n"
			"わはは\n"
			"わは\n"
			"わ\n"
			"わ<w>\n"
			"わわ\n"
			"わわ<w>\n"
			"わわわ\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "wa", "わ");
		append_mapping(&m.arr, "ha", "は");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("wahaha\b\bwawa");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("invalid_prefix_leaks_out_of_pending_conv", "x\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ma", "ま");
		in = open_tmp_file_containing("x");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("invalid_prefix_leaks_one_char_at_a_time",
			"<m>\n"
			"m<x>\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ma", "ま");
		append_mapping(&m.arr, "xa", "ぁ");
		in = open_tmp_file_containing("mx");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("invalid_prefix_leaks_two_chars_at_a_time",
			"<x>\n"
			"x?\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ma", "ま");
		append_mapping(&m.arr, "xa", "ぁ");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("x?");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("leak_invalid_prefix_then_immediately_convert", "<k>\nkッ\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ka", "か");
		append_mapping(&m.arr, "J", "ッ");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("kJ");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("delete_converted_ascii_char", "<k>\nkr\nk\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ka", "か");
		in = open_tmp_file_containing("kr\b");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("delete_converted_ascii_char_with_prior_kana_char", "<k>\nか\nか<k>\nかkr\nかk\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ka", "か");
		in = open_tmp_file_containing("kakr\b");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("delete_converted_ascii_char_with_prior_kana_char_2", "<k>\nか\nか<k>\nかkb\nかk\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ka", "か");
		in = open_tmp_file_containing("kakb\b");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("delete_converted_2_byte_char", "<d>\n<dm>\né\n\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "dmf", "é");
		in = open_tmp_file_containing("dmf\b");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
	}

	while (run_test("can_input_four_key_code",
			"<1>\n"
			"<1 >\n"
			"<1 j>\n"
			"乏\n")) {
		struct mapping m = {
			.include_kanji = 1,
		};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};

		expect_ok(mapping_populate(&m));
		in = open_tmp_file_containing("1 jf");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
		XFCLOSE(in);
	}

	while (run_test("show_candidates_for_four_key_code", NULL)) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_keyboard = 1,
		};

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
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
		XFCLOSE(in);
	}

	while (run_test("show_cutoff_guide", NULL)) {
		struct mapping m = {
			.include_kanji = 1,
		};
		struct input_flags f = {
			.show_cutoff_guide = 1,
		};
		expect_ok(mapping_populate(&m));
		in = open_tmp_file_containing("y \b\bu ");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		XFCLOSE(in);
	}

	while (run_test("show_cutoff_then_pending_and_converted_then_keyboard", NULL)) {
		struct mapping m = {
			.include_kanji = 1,
		};
		struct input_flags f = {
			.show_pending_and_converted = 1,
			.show_cutoff_guide = 1,
			.show_keyboard = 1,
		};
		expect_ok(mapping_populate(&m));
		in = open_tmp_file_containing("t ");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		XFCLOSE(in);
	}

	while (run_test("enter_clears_input_line",
			"<r>\n"
			"ろ\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ba", "ば");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing("ro\n");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
		XFCLOSE(in);
	}

	while (run_test("does_not_remember_trailing_characters_after_enter",
			"<b>\n"
			"ば\n"
			"j\n")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ba", "ば");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing("ba\nj");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
		XFCLOSE(in);
	}

	while (run_test("enter before_typing_anything_does_not_crash", "")) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ba", "ば");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing("\n");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
		XFCLOSE(in);
	}

	while (run_test("use_osc52_to_save_include_kanji", "\e]52;c;44Gm44GZ44Go5oiQ5Yqf\a")) {
		struct mapping m = {0};
		struct input_flags f = {
			.save_with_osc52 = 1,
		};
		append_mapping(&m.arr, "a", "て");
		append_mapping(&m.arr, "b", "す");
		append_mapping(&m.arr, "c", "と");
		append_mapping(&m.arr, "d", "成");
		append_mapping(&m.arr, "e", "功");

		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing("abcde\n");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
		XFCLOSE(in);
	}

	while (run_test("use_osc52_to_save_with_padding",
			"\e]52;c;44GC44GE\a"
			"\e]52;c;44GC44GEdQ==\a"
			"\e]52;c;QeOBhOOCjU4=\a"
			"\e]52;c;44GC44KN44Gv\a")) {
		struct mapping m = {0};
		struct input_flags f = {
			.save_with_osc52 = 1,
		};
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
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
		XFCLOSE(in);
	}

	while (run_test("use_osc52_to_save_use_plus_and_slash_in_output",
			"\e]52;c;eHk+\a"
			"\e]52;c;ams/\a"
			"\e]52;c;zp/Onw==\a"
			"\e]52;c;M+OCjQ==\a")) {
		struct mapping m = {0};
		struct input_flags f = {
			.save_with_osc52 = 1,
		};
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
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
		XFCLOSE(in);
	}

	while (run_test("show_rsc_list", NULL)) {
		struct mapping m = {0};
		struct input_flags f = {
			.show_rsc_list = 1,
		};
		append_mapping(&m.arr, "yj", "肉");
		append_mapping(&m.arr, "yk", "肘");
		append_mapping(&m.arr, "yh", "漢");
		append_mapping(&m.arr, "yl", "滝");
		append_mapping(&m.arr, "y;", "方");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		in = open_tmp_file_containing("y");
		expect_ok(input_impl(&m, &f));
		destroy_mapping(&m);
		XFCLOSE(in);
	}

	return 0;
}
