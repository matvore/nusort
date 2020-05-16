#include "commands.h"
#include "test_util.h"

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("error_for_non_number_kanji_count");
	{
		const char *argv[] = {"10a"};
		expect_fail(kanji_db_chart(argv, 1));
	}
	end_test("正の整数ではない: 10a\n");

	start_test("error_for_negative_kanji_count");
	{
		const char *argv[] = {"-2"};
		expect_fail(kanji_db_chart(argv, 1));
	}
	end_test("正の整数ではない: -2\n");

	start_test("only_1_kanji");
	{
		const char *argv[] = {"1"};
		expect_ok(kanji_db_chart(argv, 1));
	}
	end_test("日A\n");

	start_test("10_kanji");
	{
		const char *argv[] = {"10"};
		expect_ok(kanji_db_chart(argv, 1));
	}
	end_test_expected_content_in_file();

	start_test("200_kanji");
	{
		const char *argv[] = {"200"};
		expect_ok(kanji_db_chart(argv, 1));
	}
	end_test_expected_content_in_file();
}
