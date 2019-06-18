#include "non_sorted_codes.h"
#include "romazi.h"

#include <string.h>

struct {
	char kanji[5];
	char key;
} NON_SORTED[] = {
	{"一", '!'},
	{"二", '@'},
	{"三", '#'},
	{"四", '$'},
	{"五", '%'},
	{"六", '^'},
	{"七", '&'},
	{"八", '*'},
	{"九", '('},
	{"十", ')'},
};

#define NON_SORTED_NR (sizeof(NON_SORTED) / sizeof(*NON_SORTED))

int is_non_sorted_kanji(const char *k)
{
	size_t i;
	for (i = 0; i < NON_SORTED_NR; i++)
		if (!strcmp(k, NON_SORTED[i].kanji))
			return 1;
	return 0;
}

int is_non_sorted_code(size_t key_index)
{
	size_t i;
	for (i = 0; i < NON_SORTED_NR; i++)
		if (key_index == char_to_key_index(NON_SORTED[i].key))
			return 1;
	return 0;
}
