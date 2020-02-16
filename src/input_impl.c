#include "chars.h"
#include "commands.h"
#include "input_impl.h"
#include "keyboard.h"
#include "mapping.h"
#include "mapping_util.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

struct {
	char *el;
	size_t cnt;
	size_t alloc;
} converted = {0};

static void append_to_converted(char const *s, int len)
{
	GROW_ARRAY_BY(converted, len);
	memcpy(converted.el + converted.cnt - len, s, len);
}

static int is_done(
	struct key_mapping_array const *mapping, Orig const so_far_input)
{
	struct key_mapping const *m;

	BSEARCH(m, mapping->el, mapping->cnt, code_cmp(m->orig, so_far_input));

	if (!m)
		return 0;

	append_to_converted(m->conv, strlen(m->conv));

	return 1;
}

int input_impl(struct mapping *mapping,
	       FILE *keyboard_out, FILE *pending_out)
{
	Orig so_far_input = {0};

	while (1) {
		int ch;
		int did_delete_orig = 0;
		int did_delete_conv = 0;
		int pressed_bs = 0;

		keyboard_update(&mapping->arr, so_far_input);
		if (keyboard_out) {
			keyboard_write(keyboard_out);
			fputc('\n', keyboard_out);
		}

		ch = fgetc(in);

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
			if (is_complete_utf8(converted.el[converted.cnt-1], 1))
				converted.cnt--;
			else if (is_complete_utf8(converted.el[converted.cnt-2],
						  2))
				converted.cnt -= 2;
			else
				converted.cnt -= 3;
			memset(converted.el + converted.cnt, 0, 3);
			did_delete_conv = 1;
		}

		while (1) {
			if (is_done(&mapping->arr, so_far_input)) {
				memset(&so_far_input, 0, sizeof(so_far_input));
				break;
			}
			if (strlen(so_far_input) == 2 && so_far_input[1] == ' ')
				mapping_lazy_populate(
					mapping, so_far_input);
			if (incomplete_code_is_prefix(&mapping->arr,
						      so_far_input))
				break;

			append_to_converted(so_far_input, 1);
			memmove(so_far_input, so_far_input + 1,
				sizeof(so_far_input) - 1);
		}

		if (!pending_out)
			continue;

		if (converted.cnt)
			fwrite(converted.el, converted.cnt, 1, pending_out);
		if (so_far_input[0] || did_delete_orig)
			fprintf(pending_out, "<%s>", so_far_input);
		if (so_far_input[0] || did_delete_orig ||
		    converted.cnt || did_delete_conv)
			fputc('\n', pending_out);
	}

	DESTROY_ARRAY(converted);

	return 0;
}
