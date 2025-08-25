#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static void dostats(void)
{
	struct flagset fs = {0};
	expect_ok(kana_stats(&fs, 0, 0));
	DESTROY_ARRAY(fs);
}

int main(void)
{
	config_tests(CONFIG_TESTS_STDIN_FROM_FILE);
	set_test_source_file(__FILE__);

	while (run_test("empty_input", 0)) dostats();

	while (run_test("one_char", 0)) dostats();

	while (run_test("two_chars", 0)) dostats();

	while (run_test("hundred_chars", 0)) dostats();

	while (run_test("mix_2_chars", 0)) dostats();

	while (run_test("katakana", 0)) dostats();

	while (run_test("katakana_hiragana_mixed", 0)) dostats();

	while (run_test("katakana_percent_fractional", 0)) dostats();

	/* In UTF-8, も has 2 0x82 bytes */
	while (run_test("hiragana_mo", 0)) dostats();

	/* In UTF-8, 䂂 has 2 0x82 bytes as well */
	while (run_test("kanji_shares_two_bytes_with_hira_mo", 0)) dostats();

	/* In UTF-8, い shares the first two bytes with あ */
	while (run_test("hiragana_i", 0)) dostats();

	/* 䁂 shares the last two bytes with あ */
	while (run_test("kanji_shares_last_two_bytes_with_hira_a", 0))
		dostats();

	while (run_test("lots_of_different_hiragana", 0)) dostats();

	while (run_test("lots_of_different_katakana", 0)) dostats();

	while (run_test("chouon_kigou", 0)) dostats();

	while (run_test("chouon_kigou_with_others", 0)) dostats();

	while (run_test("wo", 0)) dostats();

	while (run_test("va", 0)) dostats();

	while (run_test("wa_gyo_with_dakuten", 0)) dostats();

	return 0;
}
