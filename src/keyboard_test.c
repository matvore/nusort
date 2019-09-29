#include "commands.h"
#include "keyboard.h"
#include "mapping.h"
#include "test_util.h"
#include "util.h"

#include <errno.h>
#include <string.h>

static void draw_keyboard(void)
{
	if (!fwrite(keyboard_bytes(), keyboard_size(), 1, out)) {
		xfprintf(stderr, "fwriteエラー:\t%s\n", strerror(errno));
		exit(72);
	}
}

int main(void)
{
	start_test(__FILE__, "default_view");
	{
		struct mapping m = {0};
		m.ergonomic_sort = 1;
		mapping_populate(&m);
		keyboard_update(&m, "");
		draw_keyboard();
		mapping_destroy(&m);
	}
	end_test_expected_content_in_file();

	start_test(__FILE__, "first_key_typed");
	{
		struct mapping m = {0};
		m.ergonomic_sort = 1;
		mapping_populate(&m);
		keyboard_update(&m, "/");
		draw_keyboard();
		mapping_destroy(&m);
	}
	end_test_expected_content_in_file();
}
