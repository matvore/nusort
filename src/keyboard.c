#include "keyboard.h"

#include "romazi.h"
#include "util.h"

#include <stdint.h>
#include <string.h>

static char const KEYBOARD[] =
	"┌──┬──┬──┬──┬──┬──╥──┬──┬──┬──┐\n"
	"│　│　│　│　│　│　║　│　│　│　│\n"	/* number row */
	"└┬─┴┬─┴┬─┴┬─┴┬─┴╥─╨┬─┴┬─┴┬─┴┬─┴┐\n"
	" │　│　│　│　│　║　│　│　│　│　│\n"	/* QWER row */
	" └┬─┴┬─┴┬─┴┬─┴┬─╨╥─┴┬─┴┬─┴┬─┴┬─┴┐\n"
	"  │　│　│　│　│　║　│　│　│　│　│\n"	/* ASDF row */
	"  └┬─┴┬─┴┬─┴┬─┴┬─╨╥─┴┬─┴┬─┴┬─┴┬─┴┐\n"
	"   │　│　│　│　│　║　│　│　│　│　│\n"	/* ZXCV row */
	"   └──┴──┴──┴──┴──╨──┴──┴──┴──┴──┘";

static uint16_t const LINE_OFFSET[] = {
	/* キーインデックス 0-39 - 非シフト */
	0x061,
	0x103,
	0x1a7,
	0x24d,

	/* キーインデックス 40-79 - シフト */
	0x003,
	0x0a4,
	0x147,
	0x1ec,
};

static char keyboard[sizeof(KEYBOARD)] = {0};

char const *keyboard_bytes(void) { return keyboard; }
size_t keyboard_size(void) { return sizeof(KEYBOARD); }

void keyboard_update(struct mapping const *mapping, char const *prefix)
{
	key_index_t ki;
	char full_code[4];
	size_t missing_char_index = strlen(prefix);

	strncpy(full_code, prefix, sizeof(full_code));

	if (!keyboard[0])
		memcpy(keyboard, KEYBOARD, sizeof(KEYBOARD));

	for (ki = 0; ki < MAPPABLE_CHAR_COUNT; ki++) {
		size_t char_offset = LINE_OFFSET[ki / 10];
		char *keyboard_p;
		size_t char_bytes;
		struct key_mapping const *m;

		if (ki < KANJI_KEY_COUNT) {
			char_offset += (ki % 10) * 6;
			char_bytes = 3;
		} else {
			char_offset += (ki % 10) * 9;
			char_bytes = 6;
		}

		full_code[missing_char_index] = KEY_INDEX_TO_CHAR_MAP[ki];

		BSEARCH(m, mapping->codes.el, mapping->codes.cnt,
			code_cmp(m->orig, full_code));

		keyboard_p = keyboard + char_offset;
		if (!m) {
			memcpy(keyboard_p, KEYBOARD + char_offset, char_bytes);
		} else {
			size_t conv_len = strlen(m->conv);
			memcpy(keyboard_p, m->conv, conv_len);
			memset(keyboard_p + conv_len, 0, char_bytes - conv_len);
		}
	}
}
