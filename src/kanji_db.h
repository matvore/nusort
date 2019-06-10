#include <stddef.h>

struct kanji_entry {
	/* UTF-8ヌル終端バイト列. */
	char c[5];
	/* 漢字出現による順位 */
	unsigned short ranking;
	/* 部首＋画数による並べ替えキー */
	unsigned int rad_so_sort_key;
};

int print_last_rank_contained(const char **argv, int argc);
