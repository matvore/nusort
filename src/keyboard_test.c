#include "chars.h"
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

	while (run_test("default_view", NULL)) {
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
		keyboard_write();
		destroy_mapping(&m);
	}

	while (run_test("first_key_typed", NULL)) {
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
		keyboard_write();
		destroy_mapping(&m);
	}

	while (run_test("pad_half_width_quotes", NULL)) {
		struct key_mapping_array m = {0};

		append_mapping(&m, "d", "“");
		append_mapping(&m, "k", "”");
		expect_ok(sort_and_validate_no_conflicts(&m));
		keyboard_update(&m, "");
		keyboard_write();

		DESTROY_ARRAY(m);
	}

	while (run_test("pad_half_width_single_quotes", NULL)) {
		struct key_mapping_array m = {0};

		append_mapping(&m, "b", "‘");
		append_mapping(&m, "n", "’");
		expect_ok(sort_and_validate_no_conflicts(&m));
		keyboard_update(&m, "");
		keyboard_write();

		DESTROY_ARRAY(m);
	}

	while (run_test("formats_combining_dakuten", NULL)) {
		struct key_mapping_array m = {0};

		append_mapping(&m, "j", "ゑ" COMBINING_DAKUTEN);
		append_mapping(&m, "k", "う" COMBINING_DAKUTEN);
		append_mapping(&m, "l", "わ" COMBINING_DAKUTEN);
		append_mapping(&m, ";", "を" COMBINING_DAKUTEN);
		expect_ok(sort_and_validate_no_conflicts(&m));
		keyboard_update(&m, "");
		keyboard_write();

		DESTROY_ARRAY(m);
	}

	while (run_test("rsc_list_basic", NULL)) {
		struct key_mapping_array m = {0};

		append_mapping(&m, "xj", "作");
		append_mapping(&m, "xk", "準");
		append_mapping(&m, "xl", "巾");
		append_mapping(&m, "x;", "方");
		expect_ok(sort_and_validate_no_conflicts(&m));
		keyboard_update(&m, "x");
		keyboard_show_rsc_list();

		DESTROY_ARRAY(m);
	}

	while (run_test("rsc_list_grouping", NULL)) {
		struct key_mapping_array m = {0};

		append_mapping(&m, "yj", "作");
		append_mapping(&m, "yk", "準");
		append_mapping(&m, "yh", "漢");
		append_mapping(&m, "yl", "滝");
		append_mapping(&m, "y;", "方");
		expect_ok(sort_and_validate_no_conflicts(&m));
		keyboard_update(&m, "y");
		keyboard_show_rsc_list();

		DESTROY_ARRAY(m);
	}

	while (run_test("extra_keys_on_right", NULL)) {
		struct key_mapping_array m = {0};

		append_mapping(&m, "k]", "作");
		append_mapping(&m, "k'", "準");
		append_mapping(&m, "k=", "漢");
		append_mapping(&m, "k+", "滝");
		append_mapping(&m, "k\"", "方");
		append_mapping(&m, "k}", "わ" COMBINING_DAKUTEN);
		expect_ok(sort_and_validate_no_conflicts(&m));
		keyboard_update(&m, "k");
		keyboard_write();

		DESTROY_ARRAY(m);
	}

	while (run_test("some_radicals_skipped_in_rsc_guide", NULL)) {
		struct key_mapping_array m = {0};

		expect_ok(sort_and_validate_no_conflicts(&m));

		append_mapping(&m, "/a", "龍");
		append_mapping(&m, "/b", "黒");
		append_mapping(&m, "/c", "麻");
		append_mapping(&m, "/d", "麗");
		append_mapping(&m, "/e", "魅");
		append_mapping(&m, "/f", "髪");
		append_mapping(&m, "/g", "飯");
		keyboard_update(&m, "/");
		keyboard_show_rsc_list();

		DESTROY_ARRAY(m);
	}
}
