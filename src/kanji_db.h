#ifndef KANJI_H
#define KANJI_H

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
	uint16_t ranking : 14;

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

unsigned kanji_db_nr(void);

/* 返された配列は codepoint によって並べ替え済みです。*/
struct kanji_entry const *kanji_db(void);

/*
 * 返された配列は kanji_db に対するインデックスであって rsc によって並べ替え
 * 済みです。
 */
uint16_t const *kanji_db_rsc_sorted(void);

static inline struct kanji_entry const *kanji_from_rsc_index(unsigned k)
{
	return kanji_db() + kanji_db_rsc_sorted()[k];
}

/* e の kanji_db_rsc_sorted 中のインデックスを返す。*/
uint16_t kanji_db_rsc_index(struct kanji_entry const *e);

struct kanji_entry const *kanji_db_lookup(char const *kanji);

int distinct_rsc_cmp(
	struct kanji_entry const *, struct kanji_entry const *);

/*
 * 部首+画数順でべ替えますが、部首も画数も同一の２文字は必ず決まった順番に配置し
 * ます。
 */
void predictably_sort_by_rsc(struct kanji_entry const **, size_t count);

#endif
