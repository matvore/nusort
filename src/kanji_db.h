#include <stddef.h>
#include <stdint.h>

#define CUTOFFTYPE_NONE                  0
#define CUTOFFTYPE_NEWSTROKECOUNT        1
#define CUTOFFTYPE_NEWRADICAL            2
#define CUTOFFTYPE_NEWRADICALSTROKECOUNT 3

struct kanji_entry {
	/* UTF-8ヌル終端バイト列. */
	char c[5];
	/* 漢字出現による順位 */
	uint16_t ranking;

	unsigned cutoff_type : 2;

	/* 追加する予定のあるフィールド：
	unsigned jis_suijun : 3;
	char zh_simplified[5];
	const char *zh_traditional;
	*/

	/* 部首＋画数による並べ替えキー */
	unsigned int rsc_sort_key;
};

size_t kanji_db_nr(void);
struct kanji_entry *kanji_db(void);
