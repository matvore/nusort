#include "chars.h"
#include "kanji_db.h"
#include "keyboard.h"
#include "radicals.h"
#include "residual_stroke_count.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>

#define I "\000\000\000│"
#define II "\000\000\000║"
#define r "┌"
#define T "┬"
#define TT "╥"
#define L_ "└─"
#define LT "└┬"
#define vv "╨"

static char const KEYBOARD[] = ""
	  r"──"T"──"T"──"T"──"T"──"T"──"TT"──"T"──"T"──"T"──"TT"──" T"──" "┐\n"
	  I"　"I"　"I"　"I"　"I"　"I"　"II"　"I"　"I"　"I"　"II"　" I"　"  I"\n"
	  LT"─┴"T"─┴"T"─┴"T"─┴"T"─┴"TT"─╨"T"─┴"T"─┴"T"─┴"T"─"vv TT"─┴" T"─┴┐\n"
	" "I"　"I"　"I"　"I"　"I"　"II"　"I"　"I"　"I"　"I"　"  II"　" I"　"I"\n"
	" "LT"─┴"T"─┴"T"─┴"T"─┴"T"─╨"TT"─┴"T"─┴"T"─┴"T"─┴"T  "─"vv TT"─┴"T"─┘\n"
	"  "I"　"I"　"I"　"I"　"I"　"II"　"I"　"I"　"I"　"I"　"    II"　"I"\n"
	"  "LT"─┴"T"─┴"T"─┴"T"─┴"T"─╨"TT"─┴"T"─┴"T"─┴"T"─┴"T    "─"vv T"─┘\n"
	"   "I"　"I"　"I"　"I"　"I"　"II"　"I"　"I"　"I"　"I"　"      I"\n"
	"   "L_ "─┴──""┴──""┴──""┴──" vv "──┴──""┴──""┴──""┴──"      "┘";

#undef I
#undef II
#undef r
#undef T
#undef TT
#undef L_
#undef LT
#undef vv

static char keyboard[sizeof(KEYBOARD)] = {0};

static int key_index_to_cell_offset(int ki)
{
	int col, line_off;
	if (ki < KANJI_KEYS_ROW_0) {
		line_off = 0x076;
		col = ki;
	} else if (ki < KANJI_KEYS_ROWS_01) {
		line_off = 0x15d;
		col = ki - 12;
	} else if (ki < KANJI_KEYS_ROWS_012) {
		line_off = 0x243;
		col = ki - 24;
	} else if (ki < KANJI_KEY_COUNT) {
		line_off = 0x2b3 + 18 + 18 + 18 + 18 + 15 + 9 + 6;
		col = ki - 35;
	} else if (ki < KANJI_KEY_COUNT + KANJI_KEYS_ROW_0) {
		line_off = 0x003;
		col = ki - 45;
	} else if (ki < KANJI_KEY_COUNT + KANJI_KEYS_ROWS_01) {
		line_off = 0x0c5 + 18 + 18;
		col = ki - 57;
	} else if (ki < KANJI_KEY_COUNT + KANJI_KEYS_ROWS_012) {
		line_off = 0x189 + 18 + 18 + 18 + 18;
		col = ki - 69;
	} else if (ki < KANJI_KEY_COUNT * 2) {
		line_off = 0x24f + 18 + 18 + 18 + 18 + 15 + 9;
		col = ki - 80;
	} else {
		DIE(0, "%d\n", ki);
	}

	return line_off + col * 9;
}

static struct {
	/* kanji_db 配列へのインデックス */
	unsigned k;

	/* 入力コードの最後のキー */
	char c;
} rsc_list[MAPPABLE_CHAR_COUNT];

static int rsc_list_nr;

void keyboard_write(void)
{
	fwrite(keyboard, sizeof(KEYBOARD), 1, out);
}

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

	if (!keyboard[0])
		memcpy(keyboard, KEYBOARD, sizeof(KEYBOARD));

	rsc_list_nr = 0;

	/* 入力文字列を全てキーから消し、キーを空にします。*/
	for (ki = 0; ki < MAPPABLE_CHAR_COUNT; ki++) {
		int offset = key_index_to_cell_offset(ki);
		memcpy(keyboard + offset, KEYBOARD + offset, 6);
	}

	for (ki = 0; ki < MAPPABLE_CHAR_COUNT; ki++) {
		struct key_mapping const *m;
		int str_bytes;

		full_code[missing_char_index] = KEY_INDEX_TO_CHAR_MAP[ki];

		BSEARCH(m, mapping->el, mapping->cnt,
			code_cmp(m->orig, full_code));

		if (!m)
			continue;

		str_bytes = strlen(m->conv);
		if (str_bytes > 6)
			DIE(0, "conv が長すぎます(%d): %s", str_bytes, m->conv);

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

#define RSC_GUIDE_SHOW_BUSHU 0
#define RSC_GUIDE_SHOW_STROKE_COUNT 1
#define RSC_GUIDE_SHOW_INPUT_KANJI 2

static struct {
	struct {
		unsigned type;

		union {
			unsigned bushu_kanji_index;
			unsigned stroke_count;
			struct {
				unsigned ki;
				char input_c;
			} input_kanji;
		} u;
	} *el;
	size_t cnt, alloc;
} rsc_guide;

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

	GROW_ARRAY_BY(rsc_guide, 1);
	rsc_guide.el[rsc_guide.cnt - 1].type = RSC_GUIDE_SHOW_BUSHU;
	rsc_guide.el[rsc_guide.cnt - 1].u.bushu_kanji_index = last_rad;
}

static unsigned rsc_sort_key_change(int rsc_i)
{
	unsigned prev, curr = kanji_db()[rsc_list[rsc_i].k].rsc_sort_key;
	if (!rsc_i)
		return curr;
	prev = kanji_db()[rsc_list[rsc_i - 1].k].rsc_sort_key;
	return curr != prev ? curr : 0;
}

static void print_kanji_line(int i)
{
	switch (rsc_guide.el[i].type) {
	case RSC_GUIDE_SHOW_BUSHU:
		fprintf(out, "\e[%d;%dm%s\e[%dm",
			ANSI_BRIGHT_MAGENTA_FG, ANSI_BOLD,
			kanji_db()[rsc_guide.el[i].u.bushu_kanji_index].c,
			ANSI_RESET);
		break;
	case RSC_GUIDE_SHOW_STROKE_COUNT:
		fprintf(out, "\e[%dm%d \e[%dm",
			ANSI_BRIGHT_YELLOW_FG,
			rsc_guide.el[i].u.stroke_count,
			ANSI_RESET);
		break;
	case RSC_GUIDE_SHOW_INPUT_KANJI:
		fputs(kanji_db()[rsc_guide.el[i].u.input_kanji.ki].c, out);
		break;
	default:
		DIE(0, "");
	}
}

static void print_key_line(int i)
{
	switch (rsc_guide.el[i].type) {
	case RSC_GUIDE_SHOW_BUSHU:
		fprintf(out, "\e[%d;%dm部\e[%dm", ANSI_BRIGHT_MAGENTA_FG,
			ANSI_BOLD, ANSI_RESET);
		break;
	case RSC_GUIDE_SHOW_STROKE_COUNT:
		fputs("  ", out);
		if (rsc_guide.el[i].u.stroke_count >= 10)
			fputc(' ', out);
		break;
	case RSC_GUIDE_SHOW_INPUT_KANJI:
		fprintf(out, "%c ",
			rsc_guide.el[i].u.input_kanji.input_c);
		break;
	default:
		DIE(0, "");
	}
}

#define RSC_LIST_WRAP_WIDTH 60

void keyboard_show_rsc_list(void)
{
	int i, amount_printed = 0;

	QSORT(, rsc_list, rsc_list_nr,
	      distinct_rsc_cmp(kanji_db() + rsc_list[a].k,
			       kanji_db() + rsc_list[b].k) < 0);
	rsc_guide.cnt = 0;

	for (i = 0; i < rsc_list_nr; i++) {
		unsigned key_change = rsc_sort_key_change(i);

		maybe_add_radical_transition(i);

		if (key_change) {
			unsigned r = residual_stroke_count_from_rsc_sort_key(
				key_change);
			if (r) {
				GROW_ARRAY_BY(rsc_guide, 1);
				rsc_guide.el[rsc_guide.cnt - 1].type =
					RSC_GUIDE_SHOW_STROKE_COUNT;
				rsc_guide.el[rsc_guide.cnt - 1].u.stroke_count =
					r;
			}
		}

		GROW_ARRAY_BY(rsc_guide, 1);
		rsc_guide.el[rsc_guide.cnt - 1].type =
			RSC_GUIDE_SHOW_INPUT_KANJI;
		rsc_guide.el[rsc_guide.cnt - 1].u.input_kanji.ki =
			rsc_list[i].k;
		rsc_guide.el[rsc_guide.cnt - 1].u.input_kanji.input_c =
			rsc_list[i].c;
	}

	while (amount_printed < rsc_guide.cnt) {
		int remaining_width = RSC_LIST_WRAP_WIDTH, last_fitting_padding;
		unsigned cursor = amount_printed, last_fitting_chunk = 0;

		while (cursor < rsc_guide.cnt &&
		       (remaining_width >= 2 || !last_fitting_chunk)) {
			switch (rsc_guide.el[cursor].type) {
			case RSC_GUIDE_SHOW_BUSHU:
				remaining_width -= 2;
				break;
			case RSC_GUIDE_SHOW_INPUT_KANJI:
				remaining_width -= 2;
				last_fitting_chunk = cursor;
				last_fitting_padding = remaining_width;
				break;
			case RSC_GUIDE_SHOW_STROKE_COUNT:
				remaining_width -= 2;
				if (rsc_guide.el[cursor].u.stroke_count >= 10)
					remaining_width -= 1;
				break;
			default:
				DIE(0, "");
			}

			cursor++;
		}

		if (amount_printed)
			fputc('\n', out);
		for (i = 0; i < last_fitting_padding; i++)
			fputc(' ', out);
		for (i = last_fitting_chunk; i >= amount_printed; i--)
			print_kanji_line(i);
		fputc('\n', out);

		for (i = 0; i < last_fitting_padding; i++)
			fputc(' ', out);
		for (i = last_fitting_chunk; i >= amount_printed; i--)
			print_key_line(i);

		amount_printed = last_fitting_chunk + 1;
	}
}
