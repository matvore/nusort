#ifndef ROMAZI_H
#define ROMAZI_H

#include <stddef.h>
#include <sys/types.h>

#define KANJI_KEY_COUNT 40
#define MAPPABLE_CHAR_COUNT (KANJI_KEY_COUNT * 2)

typedef int8_t key_index_t;

extern const char KEY_INDEX_TO_CHAR_MAP[MAPPABLE_CHAR_COUNT];

ssize_t char_to_key_index_or_die(char ch);

struct unused_kanji_keys {
	unsigned char count[KANJI_KEY_COUNT];
};

int is_target_non_sorted_string(const char *s);

void get_free_kanji_keys_count(struct unused_kanji_keys *u);

struct short_code_array {
	char (*el)[2];
	size_t cnt;
	size_t alloc;
};

void get_free_kanji_codes(struct short_code_array *codes);

/* ヌル終端の入力コード */
typedef char orig_t[4];

struct key_mapping {
	orig_t orig;
	char conv[7];
};

struct key_mapping_array {
	struct key_mapping *el;
	size_t cnt;
	size_t alloc;
};

void get_romazi_codes(struct key_mapping_array *codes);

#endif
