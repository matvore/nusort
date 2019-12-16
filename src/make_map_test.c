#include "commands.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	start_test(__FILE__, "default_mapping");
	{
		xfprintf(err, "exit code: %d\n", make_map(NULL, 0));
	}
	end_test_expected_content_in_file();

	start_test(__FILE__, "bad_flag");
	{
		char const *argv[] = {"-1"};
		xfprintf(err, "exit code: %d\n", make_map(argv, 1));
	}
	end_test("フラグを認識できませんでした：-1\nexit code: 3\n");

	start_test(__FILE__, "bad_positional_argument");
	{
		char const *argv[] = {"asdf"};
		xfprintf(err, "exit code: %d\n", make_map(argv, 1));
	}
	end_test("フラグを認識できませんでした：asdf\nexit code: 3\n");

	start_test(__FILE__, "ergonomic_sort");
	{
		char const *argv[] = {"-s"};
		xfprintf(err, "exit code: %d\n", make_map(argv, 1));
	}
	end_test_expected_content_in_file();

	start_test(__FILE__, "no_classic_wo");
	{
		char const *argv[] = {"--no-classic-wo"};
		xfprintf(err, "exit code: %d\n", make_map(argv, 1));
	}
	end_test_expected_content_in_file();

	start_test(__FILE__, "hiragana_wo_key");
	{
		char const *argv[] = {"--hiragana-wo-key", "="};
		xfprintf(err, "exit code: %d\n", make_map(argv, 2));
	}
	end_test_expected_content_in_file();

	start_test(__FILE__, "no_kanji");
	{
		char const *argv[] = {"--no-kanji"};
		xfprintf(err, "exit code: %d\n", make_map(argv, 1));
	}
	end_test_expected_content_in_file();
	return 0;
}
