#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	set_test_source_file(__FILE__);

	while (run_test("default_mapping", NULL)) {
		char *argv[] = {"--short-shifted-codes"};
		expect_ok(make_map(argv, 1));
	}

	while (run_test("bad_flag", "フラグを認識できませんでした：-1\nexit code: 3\n")) {
		char *argv[] = {"-1"};
		fprintf(err, "exit code: %d\n", make_map(argv, 1));
	}

	while (run_test("bad_positional_argument", "フラグを認識できませんでした：asdf\nexit code: 3\n")) {
		char *argv[] = {"asdf"};
		fprintf(err, "exit code: %d\n", make_map(argv, 1));
	}

	while (run_test("six_is_rh", NULL)) {
		char *argv[] = {
			"-s", "--short-shifted-codes", "--no-kakko",
		};
		expect_ok(make_map(argv, 3));
	}

	while (run_test("no_kanji", NULL)) {
		char *argv[] = {
			"--no-kakko",
			"--no-kanji",
			"--short-shifted-codes",
		};
		expect_ok(make_map(argv, 3));
	}

	while (run_test("no_classic_wo", NULL)) {
		char *argv[] = {
			"--no-kakko",
			"--no-classic-wo",
			"--no-kanji",
			"--short-shifted-codes",
		};
		expect_ok(make_map(argv, 4));
	}

	while (run_test("hiragana_wo_key", NULL)) {
		char *argv[] = {
			"--no-kakko",
			"--hiragana-wo-key", "=",
			"--no-kanji",
			"--short-shifted-codes",
		};
		expect_ok(make_map(argv, 5));
	}

	while (run_test("conflict_in_romazi_map", "コード衝突: w->を と wa->わ\n")) {
		char *argv[] = {
			"--no-kakko",
			"--hiragana-wo-key", "w",
			"--no-classic-wo",
			"--short-shifted-codes",
		};
		expect_fail(make_map(argv, 5));
	}
	return 0;
}
