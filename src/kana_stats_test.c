#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	config_tests(CONFIG_TESTS_STDIN_FROM_FILE);
	set_test_source_file(__FILE__);

	start_test("empty_input");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("one_char");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("two_chars");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("hundred_chars");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("mix_2_chars");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("katakana");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("katakana_hiragana_mixed");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("katakana_percent_fractional");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	/* In UTF-8, も has 2 0x82 bytes */
	start_test("hiragana_mo");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	/* In UTF-8, 䂂 has 2 0x82 bytes as well */
	start_test("kanji_shares_two_bytes_with_hira_mo");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	/* In UTF-8, い shares the first two bytes with あ */
	start_test("hiragana_i");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	/* 䁂 shares the last two bytes with あ */
	start_test("kanji_shares_last_two_bytes_with_hira_a");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("lots_of_different_hiragana");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("lots_of_different_katakana");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("chouon_kigou");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("chouon_kigou_with_others");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("wo");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("va");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("wa_gyo_with_dakuten");
	expect_ok(kana_stats(0, 0));
	end_test_expected_content_in_file();

	return 0;
}
