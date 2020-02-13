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
		};
		get_romazi_codes(&romazi_config, &m.arr);
		if (mapping_populate(&m))
			DIE(0, "mapping_populate");
		keyboard_update(&m.arr, "/");
		keyboard_write(out);
		destroy_mapping(&m);
	}
	end_test_expected_content_in_file();
}
