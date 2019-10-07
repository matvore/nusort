#include "commands.h"
#include "keyboard.h"
#include "mapping.h"
#include "romazi.h"
#include "util.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

static int is_code_prefix(
	struct mapping const *mapping,
	orig_t const so_far_input)
{
	size_t so_far_len = strlen(so_far_input);
	orig_t extended_input;

	memcpy(&extended_input, so_far_input, sizeof(orig_t));

	for (size_t try_i = so_far_len; try_i < sizeof(orig_t) - 1; try_i++) {
		ssize_t extended_i;
		extended_input[try_i] = 1;

		BSEARCH_INDEX(extended_i, mapping->codes.cnt,,
			      code_cmp(mapping->codes.el[extended_i].orig,
				       extended_input));

		if (extended_i >= 0)
			BUG("一致するコードはあるはずありません：%s",
			    so_far_input);
		extended_i = ~extended_i;

		if (extended_i >= mapping->codes.cnt)
			continue;

		if (strncmp(mapping->codes.el[extended_i].orig, so_far_input,
			    so_far_len))
			continue;

		/* so_far_input を接頭辞として持つコードが存在します。 */
		return 1;
	}

	/* so_far_input を接頭辞として持つコードはありません。 */
	return 0;
}

static int is_done(struct mapping const *mapping, orig_t const so_far_input)
{
	struct key_mapping const *m;

	if (strlen(so_far_input) == sizeof(orig_t) - 1)
		return 1;

	BSEARCH(m, mapping->codes.el, mapping->codes.cnt,
		code_cmp(m->orig, so_far_input));

	return !!m;
}

int input_impl(struct mapping const *mapping)
{
	orig_t so_far_input = {0};

	while (1) {
		int ch;

		keyboard_update(mapping, so_far_input);
		keyboard_write(out);
		xfputc('\n', out);

		ch = xfgetc(in);

		if (ch == EOF)
			break;

		so_far_input[strlen(so_far_input)] = ch;

		if (is_done(mapping, so_far_input))
			memset(&so_far_input, 0, sizeof(so_far_input));

		if (!is_code_prefix(mapping, so_far_input))
			xfputs("コードの始まりではない\n", out);
	}

	return 0;
}
