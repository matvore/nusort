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
		struct mapping_config config = {
			.include_kanji = 1,
			.ergonomic_sort = 1,
		};
		struct key_mapping_array m = {0};
		get_romazi_codes(&romazi_config, &m);
		if (!mapping_populate(&config, &m))
			DIE(0, "mapping_populate");
		keyboard_update(&m, "");
		keyboard_write(out);
		DESTROY_ARRAY(m);
	}
	end_test_expected_content_in_file();

	start_test("first_key_typed");
	{
		struct mapping_config config = {
			.include_kanji = 1,
			.ergonomic_sort = 1,
		};
		struct key_mapping_array m = {0};
		get_romazi_codes(&romazi_config, &m);
		if (!mapping_populate(&config, &m))
			DIE(0, "mapping_populate");
		keyboard_update(&m, "/");
		keyboard_write(out);
		DESTROY_ARRAY(m);
	}
	end_test_expected_content_in_file();
}
