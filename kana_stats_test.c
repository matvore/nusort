#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	config_tests(CONFIG_TESTS_STDIN_FROM_FILE);
	set_test_source_file(__FILE__);

	while (run_test("empty_input", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("one_char", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("two_chars", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("hundred_chars", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("mix_2_chars", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("katakana", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("katakana_hiragana_mixed", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("katakana_percent_fractional", NULL))
		expect_ok(kana_stats(0, 0));

	/* In UTF-8, も has 2 0x82 bytes */
	while (run_test("hiragana_mo", NULL))
		expect_ok(kana_stats(0, 0));

	/* In UTF-8, 䂂 has 2 0x82 bytes as well */
	while (run_test("kanji_shares_two_bytes_with_hira_mo", NULL))
		expect_ok(kana_stats(0, 0));

	/* In UTF-8, い shares the first two bytes with あ */
	while (run_test("hiragana_i", NULL))
		expect_ok(kana_stats(0, 0));

	/* 䁂 shares the last two bytes with あ */
	while (run_test("kanji_shares_last_two_bytes_with_hira_a", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("lots_of_different_hiragana", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("lots_of_different_katakana", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("chouon_kigou", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("chouon_kigou_with_others", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("wo", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("va", NULL))
		expect_ok(kana_stats(0, 0));

	while (run_test("wa_gyo_with_dakuten", NULL))
		expect_ok(kana_stats(0, 0));

	return 0;
}
