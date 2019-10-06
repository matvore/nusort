#include "commands.h"
#include "keyboard.h"
#include "mapping.h"
#include "romazi.h"
#include "util.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

static int is_done(struct mapping *mapping, orig_t const so_far_input)
{
	struct key_mapping const *m;

	if (strlen(so_far_input) == sizeof(orig_t) - 1)
		return 1;

	BSEARCH(m, mapping->codes.el, mapping->codes.cnt,
		code_cmp(m->orig, so_far_input));

	return !!m;
}

int input(char const *const *argv, int argc)
{
	struct mapping mapping = {0};
	orig_t so_far_input = {0};

	mapping.ergonomic_sort = 1;
	mapping_populate(&mapping);

	while (1) {
		int ch;

		keyboard_update(&mapping, so_far_input);
		keyboard_write(out);
		xfputc('\n', out);

		ch = xfgetc(in);

		if (ch == EOF)
			break;

		so_far_input[strlen(so_far_input)] = ch;

		if (is_done(&mapping, so_far_input))
			memset(&so_far_input, 0, sizeof(so_far_input));
	}

	mapping_destroy(&mapping);

	return 0;
}
