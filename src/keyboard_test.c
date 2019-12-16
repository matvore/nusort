#include "commands.h"
#include "keyboard.h"
#include "mapping.h"
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

	init_romazi(&romazi_config);

	start_test(__FILE__, "default_view");
	{
		struct mapping m = {0};
		m.include_kanji = 1;
		m.ergonomic_sort = 1;
		mapping_populate(&m);
		keyboard_update(&m, "");
		keyboard_write(out);
		mapping_destroy(&m);
	}
	end_test_expected_content_in_file();

	start_test(__FILE__, "first_key_typed");
	{
		struct mapping m = {0};
		m.include_kanji = 1;
		m.ergonomic_sort = 1;
		mapping_populate(&m);
		keyboard_update(&m, "/");
		keyboard_write(out);
		mapping_destroy(&m);
	}
	end_test_expected_content_in_file();
}
