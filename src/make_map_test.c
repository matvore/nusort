#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("default_mapping");
	{
		fprintf(err, "exit code: %d\n", make_map(NULL, 0));
	}
	end_test_expected_content_in_file();

	start_test("bad_flag");
	{
		char const *argv[] = {"-1"};
		fprintf(err, "exit code: %d\n", make_map(argv, 1));
	}
	end_test("フラグを認識できませんでした：-1\nexit code: 3\n");

	start_test("bad_positional_argument");
	{
		char const *argv[] = {"asdf"};
		fprintf(err, "exit code: %d\n", make_map(argv, 1));
	}
	end_test("フラグを認識できませんでした：asdf\nexit code: 3\n");

	start_test("six_is_rh");
	{
		char const *argv[] = {"-s"};
		fprintf(err, "exit code: %d\n", make_map(argv, 1));
	}
	end_test_expected_content_in_file();

	start_test("no_kanji");
	{
		char const *argv[] = {"--no-kanji"};
		fprintf(err, "exit code: %d\n", make_map(argv, 1));
	}
	end_test_expected_content_in_file();

	start_test("no_classic_wo");
	{
		char const *argv[] = {"--no-classic-wo", "--no-kanji"};
		fprintf(err, "exit code: %d\n", make_map(argv, 2));
	}
	end_test_expected_content_in_file();

	start_test("hiragana_wo_key");
	{
		char const *argv[] = {"--hiragana-wo-key", "=", "--no-kanji"};
		fprintf(err, "exit code: %d\n", make_map(argv, 3));
	}
	end_test_expected_content_in_file();

	start_test("conflict_in_romazi_map");
	{
		char const *argv[] = {"--hiragana-wo-key", "w",
				      "--no-classic-wo"};
		fprintf(err, "exit code: %d\n", make_map(argv, 3));
	}
	end_test("コード衝突: w->を と wa->わ\n"
		 "exit code: 11\n");
	return 0;
}
