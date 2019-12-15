#include "kana_stats_util.h"
#include "test_util.h"

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("8_digit_hira_count");
	{
		print_kana_stats_header();
		print_kana_stats_line("foo", 3, 42111111, 0, 42111111);
	}
	end_test_expected_content_in_file();

	start_test("hira_count_fraction");
	{
		print_kana_stats_header();
		print_kana_stats_line("bar", 3, 42, 0, 85);
	}
	end_test_expected_content_in_file();

	start_test("round_up_right_column");
	{
		print_kana_stats_header();
		print_kana_stats_line("bar", 3, 42, 0, 87);
	}
	end_test_expected_content_in_file();

	start_test("8_digit_kata_count");
	{
		print_kana_stats_header();
		print_kana_stats_line("foo", 3, 0, 42111111, 52111111);
	}
	end_test_expected_content_in_file();

	start_test("12_digit_kata_count");
	{
		print_kana_stats_header();
		print_kana_stats_line("lots", 4, 0, 421198760099, 521111110099);
	}
	end_test_expected_content_in_file();

	start_test("12_digit_hira_count");
	{
		print_kana_stats_header();
		print_kana_stats_line("lots", 4, 421198760099, 0, 521111110099);
	}
	end_test_expected_content_in_file();

	start_test("12_digit_hira_and_kata_count");
	{
		print_kana_stats_header();
		print_kana_stats_line(
			"lots", 4, 421198760099, 100009777700, 531111110099);
	}
	end_test_expected_content_in_file();
}
