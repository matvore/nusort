#include <stddef.h>

#define KANJI_KEY_COUNT 40
#define MAPPABLE_CHAR_COUNT 82

struct kanji_data {
	int entry[MAPPABLE_CHAR_COUNT * MAPPABLE_CHAR_COUNT];
};

extern const char KEY_INDEX_TO_CHAR_MAP[MAPPABLE_CHAR_COUNT];

size_t char_to_key_index(char ch);

struct unused_kanji_keys {
	unsigned char count[KANJI_KEY_COUNT];
};

void get_free_kanji_keys_count(struct unused_kanji_keys *u);
void print_free_kanji_keys(void);
