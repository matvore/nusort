#include "chars.h"
#include "commands.h"
#include "dict_guide.h"
#include "input_impl.h"
#include "keyboard.h"
#include "mapping.h"
#include "mapping_util.h"
#include "radicals.h"
#include "residual_stroke_count.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"
#include "windows.h"

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

static void add_rsc_range_to_cutoff_guide(
	unsigned start_rsc, unsigned end_rsc, unsigned *rad_stroke_cnt)
{
	struct radical_coverage rads = {
		.rsc_key_start = start_rsc,
		.rsc_key_end = end_rsc,
	};
	unsigned start_strokes =
		residual_stroke_count_from_rsc_sort_key(start_rsc);
	unsigned end_strokes = end_rsc == 0xffff ? 0 :
		residual_stroke_count_from_rsc_sort_key(end_rsc);
	int rad_cnt = 0;
	struct dict_guide_el *guide_el;

	while (1) {
		radical_coverage_next(&rads);
		if (radical_coverage_done(&rads))
			break;
		rad_cnt++;

		if (rads.stroke_cnt != *rad_stroke_cnt) {
			guide_el = dict_guide_add_el();
			guide_el->type = DICT_GUIDE_BUSHU_STROKE_COUNT;
			guide_el->u.stroke_count = rads.stroke_cnt;
			*rad_stroke_cnt = rads.stroke_cnt;
		}
		guide_el = dict_guide_add_el();
		guide_el->type = DICT_GUIDE_KANJI;
		guide_el->u.kanji.ki = rads.current;

		if (rad_cnt > 1) {
			guide_el = dict_guide_add_el();
			guide_el->type = DICT_GUIDE_LINE_WRAPPABLE_POINT;
			continue;
		}
		if (!start_strokes)
			continue;
		guide_el = dict_guide_add_el();
		guide_el->type = DICT_GUIDE_STROKE_COUNT;
		guide_el->u.stroke_count = start_strokes;
		guide_el = dict_guide_add_el();
		guide_el->type = DICT_GUIDE_ELLIPSIS;
	}

	if (!end_strokes || (rad_cnt == 1 && start_rsc + 1 >= end_rsc)) {
		guide_el->type = DICT_GUIDE_LINE_WRAPPABLE_POINT;
		return;
	}

	if (guide_el->type != DICT_GUIDE_ELLIPSIS) {
		/* 省略符号直前で改行をしない */
		if (guide_el->type != DICT_GUIDE_LINE_WRAPPABLE_POINT)
			guide_el = dict_guide_add_el();
		guide_el->type = DICT_GUIDE_ELLIPSIS;
	}
	guide_el = dict_guide_add_el();
	guide_el->type = DICT_GUIDE_STROKE_COUNT;
	guide_el->u.stroke_count = residual_stroke_count_from_rsc_sort_key(
		start_rsc == end_rsc ? start_rsc : end_rsc - 1);
}

static void show_cutoff_guide(struct mapping *mapping, Orig so_far_input)
{
	int ki;
	int so_far_len = strlen(so_far_input);
	Orig key = {0};
	unsigned prev_rsc_sort_key = 0, rad_stroke_cnt = 0;

	dict_guide_clear();
	memcpy(key, so_far_input, so_far_len);

	for (ki = 0; ki <= KANJI_KEY_COUNT; ki++) {
		char ch = ki == KANJI_KEY_COUNT ? 1 : KEY_INDEX_TO_CHAR_MAP[ki];
		Orig *found_key;
		struct dict_guide_el *guide_el;
		unsigned end_ki, rsc_key_end;

		key[so_far_len] = ch;

		FIND_HASHMAP_ENTRY(mapping->cutoff_map, key, found_key);
		if (strcmp(*found_key, key))
			continue;

		end_ki = *VALUE_PTR_FOR_HASH_KEY(
			mapping->cutoff_map, found_key);
		rsc_key_end = end_ki == kanji_db_nr()
			? 0xffff : kanji_db()[end_ki].rsc_sort_key;

		if (prev_rsc_sort_key)
			add_rsc_range_to_cutoff_guide(
				prev_rsc_sort_key, rsc_key_end,
				&rad_stroke_cnt);

		dict_guide_add_el()->type = DICT_GUIDE_LINE_WRAPPABLE_POINT;

		if (ki == KANJI_KEY_COUNT)
			continue;
		guide_el = dict_guide_add_el();
		guide_el->type = DICT_GUIDE_KUGIRI_INPUT_KEY;
		guide_el->u.kugiri_input_key = ch;

		prev_rsc_sort_key = rsc_key_end;
	}

	start_window(WINDOW_CUTOFF_GUIDE);
	dict_guide_show(/*include_second_line=*/0);

	finish_window();
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
		to_top_of_screen();
		if (flags->show_cutoff_guide)
			show_cutoff_guide(mapping, so_far_input);
		start_window(WINDOW_INPUT_LINE);
		if (flags->show_pending_and_converted) {
			if (converted.cnt)
				fwrite(converted.el, converted.cnt, 1, out);
			if (so_far_input[0] || did_delete_orig)
				fprintf(out, "<%s>", so_far_input);
			if (so_far_input[0] || did_delete_orig ||
			    converted.cnt || did_delete_conv)
				add_window_newline();
		}
		finish_window();
		if (flags->show_rsc_list)
			keyboard_show_rsc_list();
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
			if (ferror(in)) {
				clearerr(in);
				continue;
			}
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
