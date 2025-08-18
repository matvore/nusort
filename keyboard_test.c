#include "chars.h"
#include "commands.h"
#include "keyboard.h"
#include "mapping.h"
#include "packetized_out.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

#include <errno.h>
#include <string.h>

static struct mapping m;

static void cleanup(void)
{
	flush_packet();
	destroy_mapping(&m);
}

int main(void)
{
	struct romazi_config romazi_config = {
		.include_kanji_numerals = 1,
		.classic_wo = 1,
	};

	set_test_source_file(__FILE__);
	debugout = 1;
	config_tests(CONFIG_TESTS_IGNORE_NULL_BYTES);

	while (run_test("default_view", NULL)) {
		m.include_kanji = 1,
		m.six_is_rh = 1,
		m.dist.short_shifted_codes = 1;

		get_romazi_codes(&romazi_config, &m.arr);
		if (mapping_populate(&m))
			DIE(0, "mapping_populate");
		keyboard_update(&m.arr, "");
		keyboard_write();
		cleanup();
	}

	while (run_test("first_key_typed", NULL)) {
		m.include_kanji = 1;
		m.six_is_rh = 1;
		m.dist.short_shifted_codes = 1;
		get_romazi_codes(&romazi_config, &m.arr);
		if (mapping_populate(&m))
			DIE(0, "mapping_populate");
		keyboard_update(&m.arr, "/");
		keyboard_write();
		cleanup();
	}

	while (run_test("pad_half_width_quotes", NULL)) {
		append_mapping(&m.arr, "d", "“");
		append_mapping(&m.arr, "k", "”");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		keyboard_update(&m.arr, "");
		keyboard_write();

		cleanup();
	}

	while (run_test("pad_half_width_single_quotes", NULL)) {
		append_mapping(&m.arr, "b", "‘");
		append_mapping(&m.arr, "n", "’");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		keyboard_update(&m.arr, "");
		keyboard_write();

		cleanup();
	}

	while (run_test("formats_combining_dakuten", NULL)) {
		append_mapping(&m.arr, "j", "ゑ" COMBINING_DAKUTEN);
		append_mapping(&m.arr, "k", "う" COMBINING_DAKUTEN);
		append_mapping(&m.arr, "l", "わ" COMBINING_DAKUTEN);
		append_mapping(&m.arr, ";", "を" COMBINING_DAKUTEN);
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		keyboard_update(&m.arr, "");
		keyboard_write();

		cleanup();
	}

	while (run_test("rsc_list_basic", NULL)) {
		append_mapping(&m.arr, "xj", "作");
		append_mapping(&m.arr, "xk", "準");
		append_mapping(&m.arr, "xl", "巾");
		append_mapping(&m.arr, "x;", "方");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		keyboard_update(&m.arr, "x");
		keyboard_show_rsc_list();

		cleanup();
	}

	while (run_test("rsc_list_grouping", NULL)) {
		append_mapping(&m.arr, "yj", "作");
		append_mapping(&m.arr, "yk", "準");
		append_mapping(&m.arr, "yh", "漢");
		append_mapping(&m.arr, "yl", "滝");
		append_mapping(&m.arr, "y;", "方");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		keyboard_update(&m.arr, "y");
		keyboard_show_rsc_list();

		cleanup();
	}

	while (run_test("extra_keys_on_right", NULL)) {
		append_mapping(&m.arr, "k]", "作");
		append_mapping(&m.arr, "k'", "準");
		append_mapping(&m.arr, "k=", "漢");
		append_mapping(&m.arr, "k+", "滝");
		append_mapping(&m.arr, "k\"", "方");
		append_mapping(&m.arr, "k}", "わ" COMBINING_DAKUTEN);
		expect_ok(sort_and_validate_no_conflicts(&m.arr));
		keyboard_update(&m.arr, "k");
		keyboard_write();

		cleanup();
	}

	while (run_test("some_radicals_skipped_in_rsc_guide", NULL)) {
		append_mapping(&m.arr, "/a", "龍");
		append_mapping(&m.arr, "/b", "黒");
		append_mapping(&m.arr, "/c", "麻");
		append_mapping(&m.arr, "/d", "麗");
		append_mapping(&m.arr, "/e", "魅");
		append_mapping(&m.arr, "/f", "髪");
		append_mapping(&m.arr, "/g", "飯");
		expect_ok(sort_and_validate_no_conflicts(&m.arr));

		keyboard_update(&m.arr, "/");
		keyboard_show_rsc_list();

		cleanup();
	}
}
