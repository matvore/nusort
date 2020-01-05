#include "commands.h"
#include "input_impl.h"
#include "keyboard.h"
#include "mapping.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

static int is_code_prefix(
	struct key_mapping_array const *mapping,
	Orig const so_far_input)
{
	size_t so_far_len = strlen(so_far_input);
	Orig extended_input;

	memcpy(&extended_input, so_far_input, sizeof(Orig));

	for (size_t try_i = so_far_len; try_i < sizeof(Orig) - 1; try_i++) {
		ssize_t extended_i;
		extended_input[try_i] = 1;

		BSEARCH_INDEX(extended_i, mapping->cnt,,
			      code_cmp(mapping->el[extended_i].orig,
				       extended_input));

		if (extended_i >= 0)
			BUG("一致するコードはあるはずありません：%s",
			    so_far_input);
		extended_i = ~extended_i;

		if (extended_i >= mapping->cnt)
			continue;

		if (strncmp(mapping->el[extended_i].orig, so_far_input,
			    so_far_len))
			continue;

		/* so_far_input を接頭辞として持つコードが存在します。 */
		return 1;
	}

	/* so_far_input を接頭辞として持つコードはありません。 */
	return 0;
}

struct {
	char *el;
	size_t cnt;
	size_t alloc;
} converted = {0};

static int is_done(
	struct key_mapping_array const *mapping, Orig const so_far_input)
{
	struct key_mapping const *m;
	size_t orig_converted_len = converted.cnt;

	BSEARCH(m, mapping->el, mapping->cnt, code_cmp(m->orig, so_far_input));

	if (!m)
		return 0;

	GROW_ARRAY_BY(converted, strlen(m->conv));

	memcpy(converted.el + orig_converted_len, m->conv, strlen(m->conv));

	return 1;
}

int input_impl(struct key_mapping_array const *mapping,
	       FILE *keyboard_out, FILE *pending_out)
{
	Orig so_far_input = {0};

	while (1) {
		int ch;
		int did_delete_orig = 0;
		int did_delete_conv = 0;
		int pressed_bs = 0;

		keyboard_update(mapping, so_far_input);
		if (keyboard_out) {
			keyboard_write(keyboard_out);
			xfputc('\n', keyboard_out);
		}

		ch = xfgetc(in);

		/* EOF、 ^D 又は Escape の場合は終了します。 */
		if (ch == EOF || ch == 4 || ch == '\e')
			break;

		if (ch == '\b' || ch == 0x7f) {
			pressed_bs = 1;
			did_delete_orig = so_far_input[0] != 0;
		}

		if (did_delete_orig) {
			so_far_input[strlen(so_far_input) - 1] = 0;
		} else if (!pressed_bs) {
			so_far_input[strlen(so_far_input)] = ch;
		} else if (converted.cnt) {
			converted.cnt -= 3;
			memset(converted.el + converted.cnt, 0, 3);
			did_delete_conv = 1;
		}

		if (is_done(mapping, so_far_input))
			memset(&so_far_input, 0, sizeof(so_far_input));

		if (pending_out) {
			if (converted.cnt)
				xfwrite(converted.el, converted.cnt,
					pending_out);
			if (so_far_input[0] || did_delete_orig)
				xfprintf(pending_out, "<%s>", so_far_input);
			if (so_far_input[0] || did_delete_orig ||
			    converted.cnt || did_delete_conv)
				xfputc('\n', pending_out);
		}

		if (!is_code_prefix(mapping, so_far_input))
			xfputs("コードの始まりではない\n", out);
	}

	DESTROY_ARRAY(converted);

	return 0;
}
