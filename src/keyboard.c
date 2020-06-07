#include "chars.h"
#include "kanji_db.h"
#include "keyboard.h"
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

static void maybe_print_preceding_space(int rsc_i)
{
	if (rsc_i && (kanji_db()[rsc_list[rsc_i    ].k].rsc_sort_key !=
		      kanji_db()[rsc_list[rsc_i - 1].k].rsc_sort_key))
		fputc(' ', out);
}

void keyboard_show_rsc_list(void)
{
	int i;

	QSORT(, rsc_list, rsc_list_nr,
	      distinct_rsc_cmp(kanji_db() + rsc_list[a].k,
			       kanji_db() + rsc_list[b].k) > 0);

	for (i = 0; i < rsc_list_nr; i++) {
		maybe_print_preceding_space(i);
		fputc(rsc_list[i].c, out);
		fputc(' ', out);
	}
	fputc('\n', out);
	for (i = 0; i < rsc_list_nr; i++) {
		maybe_print_preceding_space(i);
		fputs(kanji_db()[rsc_list[i].k].c, out);
	}
}
