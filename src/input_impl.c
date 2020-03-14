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

static void show_cutoff_guide(
	FILE *out, struct mapping *mapping, Orig so_far_input)
{
	int key_index;
	int so_far_len = strlen(so_far_input);
	Orig key = {0};
	char second_line[KANJI_KEY_COUNT * 2 + 1];
	int second_line_len = 0;

	memcpy(key, so_far_input, so_far_len);

	for (key_index = 0; key_index < KANJI_KEY_COUNT; key_index++) {
		char ch = KEY_INDEX_TO_CHAR_MAP[key_index];
		Orig *found_key;
		struct kanji_entry const *k;

		key[so_far_len] = ch;

		FIND_HASHMAP_ENTRY(mapping->cutoff_map, key, found_key);
		if (strcmp(*found_key, key))
			continue;

		k = kanji_db() + *VALUE_PTR_FOR_HASH_KEY(
			mapping->cutoff_map, found_key);
		fprintf(out, "%s", k->c);
		second_line[second_line_len] = ch;
		second_line[second_line_len + 1] = ' ';
		second_line_len += 2;
	}
	second_line[second_line_len] = 0;
	fputc('\n', out);
	fputs(second_line, out);
	fputc('\n', out);
}

int input_impl(
	struct mapping *mapping, FILE *out, struct input_flags const *flags)
{
	Orig so_far_input = {0};

	while (1) {
		int ch;
		int did_delete_orig = 0;
		int did_delete_conv = 0;
		int pressed_bs = 0;

		keyboard_update(&mapping->arr, so_far_input);
		if (flags->show_cutoff_guide)
			show_cutoff_guide(out, mapping, so_far_input);
		if (flags->show_keyboard) {
			keyboard_write(out);
			fputc('\n', out);
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

		if (!flags->show_pending_and_converted)
			continue;

		if (converted.cnt)
			fwrite(converted.el, converted.cnt, 1, out);
		if (so_far_input[0] || did_delete_orig)
			fprintf(out, "<%s>", so_far_input);
		if (so_far_input[0] || did_delete_orig ||
		    converted.cnt || did_delete_conv)
			fputc('\n', out);
	}

	DESTROY_ARRAY(converted);

	return 0;
}
