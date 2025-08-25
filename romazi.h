#ifndef ROMAZI_H
#define ROMAZI_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "flags.h"
#include "mapping_util.h"

typedef int8_t KeyIndex;

extern const char KEY_INDEX_TO_CHAR_MAP[MAPPABLE_CHAR_COUNT];

KeyIndex char_to_key_index(char ch);
KeyIndex char_to_key_index_or_die(char ch);

void hiragana_to_katakana(char *conv);

void romazi_flags(struct flagset *fs);

void get_romazi_codes(struct flagset *fs, struct key_mapping_array *);

#endif
