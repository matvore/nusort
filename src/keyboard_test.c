#include "commands.h"
#include "keyboard.h"
#include "mapping.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

#include <errno.h>
#include <string.h>

int main(void)
{
	struct romazi_config romazi_config = {
		.include_kanji_numerals = 1,
		.classic_wo = 1,
	};

	set_test_source_file(__FILE__);
	config_tests(CONFIG_TESTS_IGNORE_NULL_BYTES);

	start_test("default_view");
	{
		struct mapping m = {
			.include_kanji = 1,
			.six_is_rh = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		get_romazi_codes(&romazi_config, &m.arr);
		if (mapping_populate(&m))
			DIE(0, "mapping_populate");
		keyboard_update(&m.arr, "");
		keyboard_write(out);
		destroy_mapping(&m);
	}
	end_test_expected_content_in_file();

	start_test("first_key_typed");
	{
		struct mapping m = {
			.include_kanji = 1,
			.six_is_rh = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		get_romazi_codes(&romazi_config, &m.arr);
		if (mapping_populate(&m))
			DIE(0, "mapping_populate");
		keyboard_update(&m.arr, "/");
		keyboard_write(out);
		destroy_mapping(&m);
	}
	end_test_expected_content_in_file();

	start_test("pad_half_width_quotes");
	{
		struct key_mapping_array m = {0};

		append_mapping(&m, "d", "“");
		append_mapping(&m, "k", "”");
		expect_ok(sort_and_validate_no_conflicts(&m));
		keyboard_update(&m, "");
		keyboard_write(out);

		DESTROY_ARRAY(m);
	}
	end_test_expected_content_in_file();

	start_test("pad_half_width_single_quotes");
	{
		struct key_mapping_array m = {0};

		append_mapping(&m, "b", "‘");
		append_mapping(&m, "n", "’");
		expect_ok(sort_and_validate_no_conflicts(&m));
		keyboard_update(&m, "");
		keyboard_write(out);

		DESTROY_ARRAY(m);
	}
	end_test_expected_content_in_file();
}
