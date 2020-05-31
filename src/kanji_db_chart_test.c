#include "commands.h"
#include "test_util.h"

int main(void)
{
	set_test_source_file(__FILE__);

	while (run_test("error_for_non_number_kanji_count", "正の整数ではない: 10a\n")) {
		const char *argv[] = {"10a"};
		expect_fail(kanji_db_chart(argv, 1));
	}

	while (run_test("error_for_negative_kanji_count", "正の整数ではない: -2\n")) {
		const char *argv[] = {"-2"};
		expect_fail(kanji_db_chart(argv, 1));
	}

	while (run_test("only_1_kanji", "日A\n")) {
		const char *argv[] = {"1"};
		expect_ok(kanji_db_chart(argv, 1));
	}

	while (run_test("10_kanji", NULL)) {
		const char *argv[] = {"10"};
		expect_ok(kanji_db_chart(argv, 1));
	}

	while (run_test("200_kanji", NULL)) {
		const char *argv[] = {"200"};
		expect_ok(kanji_db_chart(argv, 1));
	}
}
