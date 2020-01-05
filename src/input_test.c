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

		xfprintf(out, "exit code: %d\n",
			 input(argv, argc, /*set_raw_mode=*/0));
	}
	end_test("フラグを認識できませんでした：--nonsense-arg\n"
		 "exit code: 3\n");

	start_test("first_key_k");
	{
		in = open_tmp_file_containing("kj");
		xfprintf(out, "exit code: %d\n",
			 input(NULL, 0, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}
	end_test_expected_content_in_file();

	start_test("impossible_code");
	{
		in = open_tmp_file_containing("kJ");
		xfprintf(out, "exit code: %d\n",
			 input(NULL, 0, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}
	end_test_expected_content_in_file();

	start_test("long_conv_strs");
	{
		in = open_tmp_file_containing("tya" "HWI" "DWO" "WHO" "YE");
		xfprintf(out, "exit code: %d\n",
			 input(NULL, 0, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}
	end_test_expected_content_in_file();

	start_test("possible_code_requires_two_more_chars");
	{
		struct key_mapping_array m = {0};

		append_mapping(&m, "xyz", "あ");
		in = open_tmp_file_containing("x!");
		xfprintf(out, "exit code: %d\n", input_impl(&m));
		XFCLOSE(in);
		DESTROY_ARRAY(m);
	}
	end_test_expected_content_in_file();

	start_test("parse_romazi_flags_no_kanji_nums");
	{
		char const *argv[] = {"--no-kanji-nums"};
		int argc = 1;

		in = open_tmp_file_containing("");
		xfprintf(out, "exit code: %d\n",
			 input(argv, argc, /*set_raw_mode=*/0));
		XFCLOSE(in);
	}
	end_test_expected_content_in_file();

	return 0;
}
