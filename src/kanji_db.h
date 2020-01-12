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

	/*
	 * 部首＋画数による並べ替えキー
	 *
	 * 部首と部首以外の画数が同じ任意の２文字はこのキーも同値です。
	 */
	uint16_t rsc_sort_key;
};

size_t kanji_db_nr(void);

/* 返された配列は codepoint によって並べ替え済みです。*/
struct kanji_entry const *kanji_db(void);

struct kanji_entry const *kanji_db_lookup(char const *kanji);

/*
 * 部首+画数順でべ替えますが、部首も画数も同一の２文字は必ず決まった順番に配置し
 * ます。
 */
void predictably_sort_by_rsc(struct kanji_entry const **, size_t count);
