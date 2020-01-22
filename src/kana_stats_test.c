#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	config_tests(CONFIG_TESTS_STDIN_FROM_FILE);
	set_test_source_file(__FILE__);

	start_test("empty_input");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("one_char");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("two_chars");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("hundred_chars");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("mix_2_chars");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("katakana");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("katakana_hiragana_mixed");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("katakana_percent_fractional");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	/* In UTF-8, も has 2 0x82 bytes */
	start_test("hiragana_mo");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	/* In UTF-8, 䂂 has 2 0x82 bytes as well */
	start_test("kanji_shares_two_bytes_with_hira_mo");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	/* In UTF-8, い shares the first two bytes with あ */
	start_test("hiragana_i");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	/* 䁂 shares the last two bytes with あ */
	start_test("kanji_shares_last_two_bytes_with_hira_a");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("lots_of_different_hiragana");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("lots_of_different_katakana");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("chouon_kigou");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("chouon_kigou_with_others");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("wo");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("va");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	start_test("wa_gyo_with_dakuten");
	fprintf(out, "exit code: %d\n", kana_stats(0, 0));
	end_test_expected_content_in_file();

	return 0;
}
