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

	/*
	 * kanji_db.c で書いてある位置
	 *
	 * distinct_rsc_sort_keyを共有する２文字が存在しません。
	 *
	 * a.rsc_sort_key > b.rsc_sort_key
	 * だと
	 * a.distinct_rsc_sort_key > b.distinct_rsc_sort_key
	 * がうかがえます。
	 */
	uint16_t distinct_rsc_sort_key;
};

size_t kanji_db_nr(void);

/* 返された配列は codepoint によって並べ替え済みです。*/
struct kanji_entry const *kanji_db(void);
