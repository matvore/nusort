#include <stddef.h>

struct kanji_entry {
	/* UTF-8ヌル終端バイト列. */
	char c[5];
	/* 漢字出現による順位 */
	unsigned short ranking;

	/* 追加する予定のあるフィールド：
	unsigned jis_suijun : 3;
	char zh_simplified[5];
	const char *zh_traditional;
	*/

	/* 部首＋画数による並べ替えキー */
	unsigned int rad_so_sort_key;
};

size_t kanji_db_nr(void);
struct kanji_entry *kanji_db(void);
