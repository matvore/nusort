#include "keyboard.h"

#include "romazi.h"
#include "util.h"

#include <errno.h>
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

void keyboard_write(FILE *stream)
{
	if (!fwrite(keyboard_bytes(), keyboard_size(), 1, stream))
		DIE(errno, "キーボードをfwriteする");
}

struct keyboard_slice {
	unsigned offset : 16;
	unsigned len : 16;
};

static struct keyboard_slice ki_to_slice(key_index_t ki)
{
	struct keyboard_slice s;

	s.offset = LINE_OFFSET[ki / 10];

	if (ki < KANJI_KEY_COUNT) {
		s.offset += (ki % 10) * 6;
		s.len = 3;
	} else {
		s.offset += (ki % 10) * 9;
		s.len = 6;
	}

	return s;
}

static void write_cell(key_index_t ki, char const *str, size_t len)
{
	struct keyboard_slice s = ki_to_slice(ki);
	char *keyboard_p = keyboard + s.offset;

	memcpy(keyboard_p, str, len);
	memset(keyboard_p + len, 0, s.len - len);
}

void keyboard_update(struct mapping const *mapping, char const *prefix)
{
	key_index_t ki;
	orig_t full_code;
	size_t missing_char_index = strlen(prefix);

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
		size_t str_bytes;

		full_code[missing_char_index] = KEY_INDEX_TO_CHAR_MAP[ki];

		BSEARCH(m, mapping->codes.el, mapping->codes.cnt,
			code_cmp(m->orig, full_code));

		if (!m)
			continue;
		str_bytes = strlen(m->conv);
		if (str_bytes < 6) {
			write_cell(ki, m->conv, str_bytes);
			continue;
		}
		if (str_bytes == 6) {
			/* 入力文字列がカナ２個です。*/
			key_index_t non_shifted = ki % KANJI_KEY_COUNT;
			key_index_t shifted = non_shifted + KANJI_KEY_COUNT;
			write_cell(shifted, m->conv, 3);
			write_cell(non_shifted, m->conv + 3, 3);
			continue;
		}

		BUG("conv が長すぎます");
	}
}
