#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static struct flagset fs;

int main(void)
{
	set_test_source_file(__FILE__);

	while (run_test("default_mapping", NULL)) {
		char *argv[] = {"--short-shifted-codes"};
		expect_ok(make_map(&fs, argv, 1));
		destroy_flagset(&fs);
	}

	while (run_test("bad_flag", "フラグを認識できませんでした：-1\nexit code: 3\n")) {
		char *argv[] = {"-1"};
		fprintf(err, "exit code: %d\n", make_map(&fs, argv, 1));
		destroy_flagset(&fs);
	}

	while (run_test("bad_positional_argument", "フラグを認識できませんでした：asdf\nexit code: 3\n")) {
		char *argv[] = {"asdf"};
		fprintf(err, "exit code: %d\n", make_map(&fs, argv, 1));
		destroy_flagset(&fs);
	}

	while (run_test("six_is_rh", NULL)) {
		char *argv[] = {
			"--6rh", "--short-shifted-codes", "--kakko", "n",
		};
		expect_ok(make_map(&fs, argv, 4));
		destroy_flagset(&fs);
	}

	while (run_test("no_kanji", NULL)) {
		char *argv[] = {
			"--kakko", "n",
			"--no-kanji",
			"--short-shifted-codes",
		};
		expect_ok(make_map(&fs, argv, 4));
		destroy_flagset(&fs);
	}

	while (run_test("no_classic_wo", NULL)) {
		char *argv[] = {
			"--kakko", "n",
			"--no-classic-wo",
			"--no-kanji",
			"--short-shifted-codes",
		};
		expect_ok(make_map(&fs, argv, 5));
		destroy_flagset(&fs);
	}

	while (run_test("hiragana_wo_key", NULL)) {
		char *argv[] = {
			"--kakko", "n",
			"--hiragana-wo-key", "=",
			"--no-kanji",
			"--short-shifted-codes",
		};
		expect_ok(make_map(&fs, argv, 6));
		destroy_flagset(&fs);
	}

	while (run_test("conflict_in_romazi_map", "コード衝突: w->を と wa->わ\n")) {
		char *argv[] = {
			"--kakko", "n",
			"--hiragana-wo-key", "w",
			"--no-classic-wo",
			"--short-shifted-codes",
		};
		expect_fail(make_map(&fs, argv, 6));
		destroy_flagset(&fs);
	}
	return 0;
}
