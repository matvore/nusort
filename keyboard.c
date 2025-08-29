#include "chars.h"
#include "dict_guide.h"
#include "kanji_db.h"
#include "keyboard.h"
#include "packetized_out.h"
#include "radicals.h"
#include "residual_stroke_count.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"
#include "windows.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>

static unsigned lineoffs[9];
static char *keyboard;
static unsigned keyboardsz;

static void kbpad(int amount)
{
	if (keyboard) memset(keyboard+keyboardsz, 0, amount);
	keyboardsz += amount;
}

static void kbraw(const char *s)
{
	unsigned len = strlen(s);
	if (keyboard) memcpy(keyboard + keyboardsz, s, len);
	keyboardsz += len;
}

static void kblin(int lineno, const char *s)
{
	for (;*s;s++) {
		switch (*s) {
		case 'I': kbpad(3); kbraw("│"); break;
		case 'H': kbpad(3); kbraw("║"); break;
		case '.':
			if (lineno >= 0) lineoffs[lineno] = keyboardsz;
			break;
		default:
			if (keyboard) keyboard[keyboardsz] = *s;
			keyboardsz += 1;
		}
	}
	kbraw(termeol());
	if (lineno >= 0) kbraw("\n");
}

static void init_keyboard0(void)
{
	keyboardsz = 0;

	kblin(0, "┌.──┬──┬──┬──┬──┬──╥──┬──┬──┬──╥──┬──┐");
	kblin(1, "I.　I　I　I　I　I　H　I　I　I　H　I　I");
	kblin(2, "└┬.─┴┬─┴┬─┴┬─┴┬─┴╥─╨┬─┴┬─┴┬─┴┬─╨╥─┴┬─┴┐");
	kblin(3, " I.　I　I　I　I　H　I　I　I　I　H　I　I");
	kblin(4, " └┬.─┴┬─┴┬─┴┬─┴┬─╨╥─┴┬─┴┬─┴┬─┴┬─╨╥─┴┬─┘");
	kblin(5, "  I.　I　I　I　I　H　I　I　I　I　H　I");
	kblin(6, "  └┬.─┴┬─┴┬─┴┬─┴┬─╨╥─┴┬─┴┬─┴┬─┴┬─╨┬─┘");
	kblin(7, "   I.　I　I　I　I　H　I　I　I　I　I");
	kblin(-1, "   └──┴──┴──┴──┴──╨──┴──┴──┴──┴──┘");
}

static int key_index_to_cell_offset(int ki)
{
	int col, lineno, ks = ki - KANJI_KEY_COUNT;
	if (ki < KANJI_KEYS_ROW_0)	{ lineno = 1; col = 0	; } else
	if (ki < KANJI_KEYS_ROWS_01)	{ lineno = 3; col = 12	; } else
	if (ki < KANJI_KEYS_ROWS_012)	{ lineno = 5; col = 24	; } else
	if (ki < KANJI_KEY_COUNT)	{ lineno = 7; col = 35	; } else
	if (ks < KANJI_KEYS_ROW_0)	{ lineno = 0; col = 45	; } else
	if (ks < KANJI_KEYS_ROWS_01)	{ lineno = 2; col = 57	; } else
	if (ks < KANJI_KEYS_ROWS_012)	{ lineno = 4; col = 69	; } else
	if (ks < KANJI_KEY_COUNT)	{ lineno = 6; col = 80	; } else
	DIE(0, "%d\n", ki);

	return lineoffs[lineno] + (ki - col) * 9;
}

static struct {
	/* kanji_db 配列へのインデックス */
	unsigned k;

	/* 入力コードの最後のキー */
	char c;
} rsc_list[MAPPABLE_CHAR_COUNT];

static int rsc_list_nr;

void keyboard_write(void) { packout(keyboard, keyboardsz); }

static int needs_padding_space(char const *str, int len)
{
	if (len != 3)
		return 0;
	if (strncmp(str, "\xe2\x80", 2))
		return 0;
	if ((str[2] & 0xf8) == 0x98)
		return 1;
	return 0;
}

static void write_cell(KeyIndex ki, char const *str, int len)
{
	char *keyboard_p = key_index_to_cell_offset(ki) + keyboard;

	memcpy(keyboard_p, str, len);
	keyboard_p += len;

	if (needs_padding_space(str, len)) {
		*keyboard_p++ = ' ';
		len++;
	}
	memset(keyboard_p, 0, 6 - len);
}

void keyboard_update(
	struct key_mapping_array const *mapping, char const *prefix)
{
	KeyIndex ki;
	Orig full_code;
	int missing_char_index = strlen(prefix);

	strncpy(full_code, prefix, sizeof(full_code));

	if (!keyboard) {
		init_keyboard0();
		keyboard = xcalloc(keyboardsz, 1);
	}

	rsc_list_nr = 0;

	/* 入力文字列を全てキーから消し、キーを空にします。*/
	init_keyboard0();

	for (ki = 0; ki < MAPPABLE_CHAR_COUNT; ki++) {
		struct key_mapping const *m;
		int str_bytes;

		full_code[missing_char_index] = KEY_INDEX_TO_CHAR_MAP[ki];

		BSEARCH(m, mapping->el, mapping->cnt,
			code_cmp(m->orig, full_code));

		if (!m)
			continue;

		str_bytes = strlen(m->conv);
		if (str_bytes > 6) continue;
			//DIE(0, "conv が長すぎます(%d): %s", str_bytes, m->conv);

		if (str_bytes == 6 && (m->conv[5] !=
				       LAST_BYTE_OF_COMBINING_DAKUTEN)) {
			/* 入力文字列がカナ２個です。*/
			int non_shifted = ki % KANJI_KEY_COUNT;
			int shifted = non_shifted + KANJI_KEY_COUNT;
			write_cell(shifted, m->conv, 3);
			write_cell(non_shifted, m->conv + 3, 3);
		} else {
			struct kanji_entry const *k = kanji_db_lookup(m->conv);
			if (k && missing_char_index) {
				rsc_list[rsc_list_nr].k = k - kanji_db();
				rsc_list[rsc_list_nr].c =
					m->orig[missing_char_index];
				rsc_list_nr++;
			}
			write_cell(ki, m->conv, str_bytes);
		}
	}
}

static void maybe_add_radical_transition(int rsc_i)
{
	unsigned start, end = kanji_db()[rsc_list[rsc_i].k].rsc_sort_key + 1;
	struct radical_coverage c = {0};
	int iters_needed = rsc_i ? 2 : 1;
	unsigned last_rad;

	if (!rsc_i)
		start = end - 1;
	else
		start = kanji_db()[rsc_list[rsc_i - 1].k].rsc_sort_key;

	c.rsc_key_start = start;
	c.rsc_key_end = end;
	while (1) {
		radical_coverage_next(&c);
		if (radical_coverage_done(&c))
			break;
		last_rad = c.current;
		iters_needed--;
	}

	if (iters_needed > 0)
		return;

	dict_guide_add(DICT_GUIDE_RSC_LIST_BUSHU, 0)
		->u.rsc_list_bushu_ki = last_rad;
}

static unsigned rsc_sort_key_change(int rsc_i)
{
	unsigned prev, curr = kanji_db()[rsc_list[rsc_i].k].rsc_sort_key;
	if (!rsc_i)
		return curr;
	prev = kanji_db()[rsc_list[rsc_i - 1].k].rsc_sort_key;
	return curr != prev ? curr : 0;
}

void keyboard_show_rsc_list(int width)
{
	int i;

	QSORT(, rsc_list, rsc_list_nr,
	      distinct_rsc_cmp(kanji_db() + rsc_list[a].k,
			       kanji_db() + rsc_list[b].k) < 0);

	dict_guide_clear();

	for (i = 0; i < rsc_list_nr; i++) {
		struct dict_guide_el *guide_el;
		unsigned key_change = rsc_sort_key_change(i);

		maybe_add_radical_transition(i);

		if (key_change) {
			unsigned r = residual_stroke_count_from_rsc_sort_key(
				key_change);
			if (r) {
				dict_guide_add(DICT_GUIDE_SPACE, 0);
				dict_guide_add(DICT_GUIDE_STROKE_COUNT, 0)
					->u.stroke_count = r;
			}
		}

		guide_el = dict_guide_add(DICT_GUIDE_KANJI, 0);
		guide_el->u.kanji.ki = rsc_list[i].k;
		guide_el->u.kanji.input_c = rsc_list[i].c;
		dict_guide_add(DICT_GUIDE_LINE_WRAPPABLE_POINT, 0);
	}

	start_window(WINDOW_RSC_LIST);
	dict_guide_show(width, /*include_second_line=*/1);
	finish_window();
}
