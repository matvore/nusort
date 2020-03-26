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

	start_test("unrecognized_argument");
	{
		char const *argv[] = {"--nonsense-arg"};
		int argc = 1;

		fprintf(out, "exit code: %d\n",
			 input(argv, argc, /*set_raw_mode=*/0));
	}
	end_test("フラグを認識できませんでした：--nonsense-arg\n"
		 "exit code: 3\n");

	start_test("first_key_k");
	{
		char const *argv[] = {
			"-s", "--short-shifted-codes",
			"--no-show-cutoff-guide",
		};
		int argc = 3;
		in = open_tmp_file_containing("kj");
		expect_ok(input(argv, argc, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}
	end_test_expected_content_in_file();

	start_test("long_conv_strs");
	{
		char const *argv[] = {
			"-s", "--short-shifted-codes",
			"--no-show-cutoff-guide",
		};
		int argc = 3;
		in = open_tmp_file_containing("tya" "HWI" "DWO" "WHO" "YE");
		expect_ok(input(argv, argc, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}
	end_test_expected_content_in_file();

	start_test("possible_code_requires_two_more_chars");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_keyboard = 1,
		};
		append_mapping(&m.arr, "xyz", "あ");
		in = open_tmp_file_containing("x!");
		expect_ok(input_impl(&m, out, &f));
		XFCLOSE(in);
		destroy_mapping(&m);
	}
	end_test_expected_content_in_file();

	start_test("parse_romazi_flags_no_kanji_nums");
	{
		char const *argv[] = {
			"--no-kanji-nums", "-s", "--short-shifted-codes",
			"--no-show-cutoff-guide",
		};
		int argc = 4;

		in = open_tmp_file_containing("");
		expect_ok(input(argv, argc, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}
	end_test_expected_content_in_file();

	start_test("exclude_kanji");
	{
		char const *argv[] = {
			"--no-kanji", "--no-show-cutoff-guide",
		};
		int argc = 2;

		in = open_tmp_file_containing("m");
		expect_ok(input(argv, argc, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}
	end_test_expected_content_in_file();

	start_test("show_pending_conversion");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "mo", "も");

		in = open_tmp_file_containing("m");
		expect_ok(input_impl(&m, out, &f));

		destroy_mapping(&m);
	}
	end_test("<m>\n");

	start_test("show_already_converted");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ki", "き");

		in = open_tmp_file_containing("ki");
		expect_ok(input_impl(&m, out, &f));

		destroy_mapping(&m);
	}
	end_test("<k>\n"
		 "き\n");

	start_test("accumulates_multiple_converted");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "pa", "ぱ");
		sort_and_validate_no_conflicts(&m.arr);

		in = open_tmp_file_containing("ropa");
		expect_ok(input_impl(&m, out, &f));

		destroy_mapping(&m);
	}
	end_test("<r>\nろ\nろ<p>\nろぱ\n");

	start_test("backspace_on_empty_line_does_nothing");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		in = open_tmp_file_containing("\b\b\b\b");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("");

	start_test("ascii_del_on_empty_line_does_nothing");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		in = open_tmp_file_containing("\x7f\x7f\x7f\x7f");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("");

	start_test("ascii_del_to_remove_pending_conversion");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ba", "ば");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("r\x7f""b");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("<r>\n<>\n<b>\n");

	start_test("backspace_to_remove_pending_conversion");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ba", "ば");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("r\bb");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("<r>\n<>\n<b>\n");

	start_test("backspace_to_remove_pending_conv_one_char_at_a_time");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ro", "ろ");
		append_mapping(&m.arr, "ryo", "りょ");
		append_mapping(&m.arr, "sya", "しゃ");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("ry\b\bsya");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("<r>\n<ry>\n<r>\n<>\n<s>\n<sy>\nしゃ\n");

	start_test("backspace_to_remove_converted_char");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "wa", "わ");
		in = open_tmp_file_containing("wa\b");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("<w>\nわ\n\n");

	start_test("backspace_to_remove_converted_char_one_at_a_time");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "wa", "わ");
		append_mapping(&m.arr, "ha", "は");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("wahaha\b\bwawa");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("<w>\n"
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
		 "わわわ\n");

	start_test("invalid_prefix_leaks_out_of_pending_conv");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ma", "ま");
		in = open_tmp_file_containing("x");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("x\n");

	start_test("invalid_prefix_leaks_one_char_at_a_time");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ma", "ま");
		append_mapping(&m.arr, "xa", "ぁ");
		in = open_tmp_file_containing("mx");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("<m>\n"
		 "m<x>\n");

	start_test("invalid_prefix_leaks_two_chars_at_a_time");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ma", "ま");
		append_mapping(&m.arr, "xa", "ぁ");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("x?");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("<x>\n"
		 "x?\n");

	start_test("leak_invalid_prefix_then_immediately_convert");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ka", "か");
		append_mapping(&m.arr, "J", "ッ");
		sort_and_validate_no_conflicts(&m.arr);
		in = open_tmp_file_containing("kJ");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("<k>\nkッ\n");

	start_test("delete_converted_ascii_char");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ka", "か");
		in = open_tmp_file_containing("kr\b");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("<k>\nkr\nk\n");

	start_test("delete_converted_ascii_char_with_prior_kana_char");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ka", "か");
		in = open_tmp_file_containing("kakr\b");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("<k>\nか\nか<k>\nかkr\nかk\n");

	start_test("delete_converted_ascii_char_with_prior_kana_char_2");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "ka", "か");
		in = open_tmp_file_containing("kakb\b");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("<k>\nか\nか<k>\nかkb\nかk\n");

	start_test("delete_converted_2_byte_char");
	{
		struct mapping m = {0};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};
		append_mapping(&m.arr, "dmf", "é");
		in = open_tmp_file_containing("dmf\b");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
	}
	end_test("<d>\n<dm>\né\n\n");

	start_test("can_input_four_key_code");
	{
		struct mapping m = {
			.include_kanji = 1,
		};
		struct input_flags f = {
			.show_pending_and_converted = 1,
		};

		expect_ok(mapping_populate(&m));
		in = open_tmp_file_containing("1 jf");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
		XFCLOSE(in);
	}
	end_test("<1>\n"
		 "<1 >\n"
		 "<1 j>\n"
		 "乏\n");

	start_test("show_candidates_for_four_key_code");
	{
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
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
		XFCLOSE(in);
	}
	end_test_expected_content_in_file();

	start_test("show_cutoff_guide");
	{
		struct mapping m = {
			.include_kanji = 1,
		};
		struct input_flags f = {
			.show_cutoff_guide = 1,
		};
		expect_ok(mapping_populate(&m));
		in = open_tmp_file_containing("y \b\bu ");
		expect_ok(input_impl(&m, out, &f));
		destroy_mapping(&m);
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		XFCLOSE(in);
	}
	end_test_expected_content_in_file();

	return 0;
}
