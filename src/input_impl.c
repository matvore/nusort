#include "chars.h"
#include "commands.h"
#include "input_impl.h"
#include "keyboard.h"
#include "mapping.h"
#include "mapping_util.h"
#include "radicals.h"
#include "residual_stroke_count.h"
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

static void save_with_osc52(void)
{
	size_t conv_i = 0;

	fputs("\e]52;c;", out);

	while (conv_i < converted.cnt) {
		int bits = 0, i;
		uint32_t packed = 0;

		for (i = 0; i < 3; i++) {
			packed <<= 8;

			if (conv_i >= converted.cnt)
				continue;

			packed |= (int)converted.el[conv_i++] & 0xff;
			bits += 8;
		}

		while (bits > 0) {
			print_base64_digit((packed >> (24 - 6)) & 63);
			bits -= 6;
			packed <<= 6;
		}
	}

	switch (converted.cnt % 3)
	{
	case 0:
		fputc('\a', out);
		break;
	case 1:
		fputs("==\a", out);
		break;
	case 2:
		fputs("=\a", out);
		break;
	}
}

static void show_cutoff_guide(struct mapping *mapping, Orig so_far_input)
{
	int ki;
	int so_far_len = strlen(so_far_input);
	Orig key = {0};
	struct {
		struct kanji_entry const *k;
		char key;
		struct radical_coverage rads;
	} cutoffs[KANJI_KEY_COUNT] = {0};
	int cutoffs_nr = 0;
	Orig *found_key;
	int last_line = -1;
	int fold_mode = 0;

	memcpy(key, so_far_input, so_far_len);

	for (ki = 0; ki < KANJI_KEY_COUNT; ki++) {
		char ch = KEY_INDEX_TO_CHAR_MAP[ki];

		key[so_far_len] = ch;

		FIND_HASHMAP_ENTRY(mapping->cutoff_map, key, found_key);
		if (strcmp(*found_key, key))
			continue;

		cutoffs[cutoffs_nr].k = kanji_db() + *VALUE_PTR_FOR_HASH_KEY(
			mapping->cutoff_map, found_key);
		cutoffs[cutoffs_nr].key = ch;

		if (cutoffs_nr)
			cutoffs[cutoffs_nr - 1].rads.rsc_key_end =
				cutoffs[cutoffs_nr].k->rsc_sort_key;

		cutoffs_nr++;
	}

	key[so_far_len] = 1;
	FIND_HASHMAP_ENTRY(mapping->cutoff_map, key, found_key);
	if (!strcmp(*found_key, key)) {
		unsigned end_ki =
			*VALUE_PTR_FOR_HASH_KEY(mapping->cutoff_map, found_key);
		unsigned rsc_key_end = end_ki == kanji_db_nr()
			? 0xffff : kanji_db()[end_ki].rsc_sort_key;
		cutoffs[cutoffs_nr - 1].rads.rsc_key_end = rsc_key_end;
	} else {
		return;
	}

	for (ki = cutoffs_nr - 1; ki >= 0; ki--) {
		fputc(cutoffs[ki].key, out);
		fputc(' ', out);
	}
	fputc('\n', out);

	for (ki = cutoffs_nr - 1; ki >= 0; ki--) {
		struct kanji_entry const *first_rad;

		cutoffs[ki].rads.rsc_key_start =
			cutoffs[ki].k->rsc_sort_key;
		radical_coverage_next(&cutoffs[ki].rads);

		first_rad = kanji_db() + cutoffs[ki].rads.current;
		fputs(first_rad->c, out);

		radical_coverage_next(&cutoffs[ki].rads);
	}
	fputc('\n', out);
	for (ki = cutoffs_nr - 1; ki >= 0; ki--) {
		struct kanji_entry const *k = cutoffs[ki].k;
		if (k->cutoff_type >= 2)
			fputs("  ", out);
		else
			fprintf(out, "\e[%dm%2d",
				(ki & 1) ? ANSI_BRIGHT_YELLOW_FG : ANSI_RESET,
				residual_stroke_count(k));
	}
	fprintf(out, "\e[%dm\n", ANSI_RESET);
	while (last_line) {
		int this_line = 0;
		for (ki = cutoffs_nr - 1; ki >= 0; ki--) {
			struct radical_coverage *r = &cutoffs[ki].rads;
			if (radical_coverage_done(r)) {
				if (!fold_mode)
					fputs("  ", out);
				continue;
			}
			fputs(kanji_db()[r->current].c, out);
			if (last_line == 1)
				fold_mode = 1;
			this_line++;
			radical_coverage_next(r);
		}
		if (!fold_mode)
			fputc('\n', out);
		last_line = this_line;
	}
	if (fold_mode)
		fputc('\n', out);
}

static void eat_escape_sequence(void)
{
	int c = fgetc(in);

	if (c == '[')
		/* 矢印キー */
		fgetc(in);
}

int input_impl(struct mapping *mapping, struct input_flags const *flags)
{
	Orig so_far_input = {0};
	int did_delete_orig = 0;
	int did_delete_conv = 0;

	while (1) {
		int ch;
		int pressed_bs = 0;

		keyboard_update(&mapping->arr, so_far_input);
		if (flags->show_cutoff_guide)
			show_cutoff_guide(mapping, so_far_input);
		if (flags->show_pending_and_converted) {
			if (converted.cnt)
				fwrite(converted.el, converted.cnt, 1, out);
			if (so_far_input[0] || did_delete_orig)
				fprintf(out, "<%s>", so_far_input);
			if (so_far_input[0] || did_delete_orig ||
			    converted.cnt || did_delete_conv)
				fputc('\n', out);
		}
		if (flags->show_rsc_list) {
			keyboard_show_rsc_list();
			fputc('\n', out);
		}
		if (flags->show_keyboard) {
			keyboard_write();
			fputc('\n', out);
		}

		did_delete_orig = 0;
		did_delete_conv = 0;

		ch = fgetc(in);

		switch (ch) {
		case '\e':
			eat_escape_sequence();
			continue;
		case EOF:
		case 4:
			/* EOF 又は ^D の場合は終了します。 */
			goto cleanup;
		case '\b':
		case '\x7f':
			pressed_bs = 1;
			did_delete_orig = so_far_input[0] != 0;
			break;
		case '\n':
			if (converted.alloc) {
				if (flags->save_with_osc52)
					save_with_osc52();
				converted.el[0] = 0;
				converted.cnt = 0;
			}
			continue;
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
	}

cleanup:
	DESTROY_ARRAY(converted);

	return 0;
}
