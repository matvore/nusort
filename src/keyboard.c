#include "keyboard.h"

#include "chars.h"
#include "romazi.h"
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
	  r"──"T"──"T"──"T"──"T"──"T"──"TT"──"T"──"T"──"T"──""┐\n"
	  I"　"I"　"I"　"I"　"I"　"I"　"II"　"I"　"I"　"I"　" I"\n"
	  LT"─┴"T"─┴"T"─┴"T"─┴"T"─┴"TT"─╨"T"─┴"T"─┴"T"─┴"T"─""┴┐\n"
	" "I"　"I"　"I"　"I"　"I"　"II"　"I"　"I"　"I"　"I"　" I"\n"
	" "LT"─┴"T"─┴"T"─┴"T"─┴"T"─╨"TT"─┴"T"─┴"T"─┴"T"─┴"T  "─┴┐\n"
	"  "I"　"I"　"I"　"I"　"I"　"II"　"I"　"I"　"I"　"I"　" I"\n"
	"  "LT"─┴"T"─┴"T"─┴"T"─┴"T"─╨"TT"─┴"T"─┴"T"─┴"T"─┴"T  "─┴┐\n"
	"   "I"　"I"　"I"　"I"　"I"　"II"　"I"　"I"　"I"　"I"　" I"\n"
	"   "L_ "─┴──""┴──""┴──""┴──" vv "──┴──""┴──""┴──""┴──" "┘";

#undef I
#undef II
#undef r
#undef T
#undef TT
#undef L_
#undef LT
#undef vv

/* 合成用の濁点と仮名を合わせれば六バイトを必する。 */
#define NON_SHIFT_CELL_BYTES 6

static uint16_t const LINE_OFFSET[] = {
	/* キーインデックス 0-39 - 非シフト */
	0x064,
	0x127,
	0x1ec,
	0x2b3,

	/* キーインデックス 40-79 - シフト */
	0x003,
	0x0c5,
	0x189,
	0x24f,
};

static char keyboard[sizeof(KEYBOARD)] = {0};

char const *keyboard_bytes(void) { return keyboard; }
size_t keyboard_size(void) { return sizeof(KEYBOARD); }

void keyboard_write(FILE *stream)
{
	fwrite(keyboard_bytes(), keyboard_size(), 1, stream);
}

struct keyboard_slice {
	unsigned offset : 16;
	unsigned len : 16;
};

static struct keyboard_slice ki_to_slice(int ki)
{
	struct keyboard_slice s;

	s.offset = LINE_OFFSET[ki / 10];

	if (ki < KANJI_KEY_COUNT) {
		s.offset += (ki % 10) * (NON_SHIFT_CELL_BYTES + 3);
		s.len = NON_SHIFT_CELL_BYTES;
	} else {
		s.offset += (ki % 10) * 9;
		s.len = 6;
	}

	return s;
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
	struct keyboard_slice s = ki_to_slice(ki);
	char *keyboard_p = keyboard + s.offset;

	memcpy(keyboard_p, str, len);
	keyboard_p += len;

	if (needs_padding_space(str, len)) {
		*keyboard_p++ = ' ';
		len++;
	}
	memset(keyboard_p, 0, s.len - len);
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

	/* 入力文字列を全てキーから消し、キーを空にします。*/
	for (ki = 0; ki < MAPPABLE_CHAR_COUNT; ki++) {
		struct keyboard_slice s = ki_to_slice(ki);
		memcpy(keyboard + s.offset, KEYBOARD + s.offset, s.len);
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
			write_cell(ki, m->conv, str_bytes);
		}
	}
}
