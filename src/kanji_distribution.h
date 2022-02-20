#ifndef KANJI_DISTRIBUTION_H
#define KANJI_DISTRIBUTION_H

#include <stdint.h>

#include "kanji_db.h"
#include "mapping_util.h"
#include "romazi.h"

struct line_stats {
	uint16_t last_char_rank;

	/* この行の全漢字の入力コードがこのキーから始まる */
	char key_ch;

	/* 未使用コード数 */
	uint8_t available;

	int16_t offset_to_target;
	int32_t cumulative_offset;

	/* 部首＋画数順位における、この行の全漢字より前（または同値）である。*/
	const struct kanji_entry *cutoff;

	uint8_t e_nr;
	const struct kanji_entry *e[KANJI_KEY_COUNT + 1];
};

struct kanji_distribution {
	unsigned sort_each_line_by_rsc : 1;
	unsigned short_shifted_codes : 1;
	unsigned busy_right_pinky : 1;
	unsigned allow_left_bracket_key1 : 1;

	uint16_t total_rank;
	uint16_t total_chars;
	uint16_t target_rank;

	size_t line_stats_nr;
	struct line_stats line_stats[KANJI_KEY_COUNT];

	/*
	 * 部首+画数順の範囲内の漢字だけをマッピングに入れるように [start, end)
	 * に設定します。
	 */
	uint16_t rsc_range_start, rsc_range_end;

	struct kanji_entries available;

	struct {
		char (*el)[2];
		size_t cnt;
		size_t alloc;
	} unused_kanji_origs;
};

int parse_kanji_distribution_flags(
	int *argc, char const *const **argv, struct kanji_distribution *);

/*
 * モジュールの用法:
 *   1. kanji_distribution ストラクトを０に memset する
 *   2. rsc 範囲を縮める場合 kd.start と kd.end を設定します
 *   3. kanji_distribution_set_preexisting_convs を呼び出す
 *   4. 一つを呼び出す:
 *      a. kanji_distribution_auto_pick_cutoff
 *      b. kanji_distribution_auto_pick_cutoff_exhaustive
 *      c. kanji_distribution_parse_user_cutoff
 *   5. populate を呼び出す
 *   6. kanji_distribution_destroy を呼び出す
 */

/*
 * 既存のマッピングを設定します。設定すると入力コードとダブらないようにしたり、
 * 漢字を二つめの入力コードに割り当てません。
 */
void kanji_distribution_set_preexisting_convs(
	struct kanji_distribution *, struct key_mapping_array const *,
	int block_already_used);

void kanji_distribution_auto_pick_cutoff(struct kanji_distribution *kd);

/*
 * 優先度 4000 より上の漢字とは基本漢字とします。
 * このコメントを書いている時点では優先度 4000 までの漢字がデータベースに
 * 入っているので、基本漢字のコードが変わらないように努力しましょう。
 */
#define BASIC_KANJI_MAX_PRI 4000

/*
 * 全漢字にコードが割り振られるように区切り字を選定します。
 *
 * prior_key:
 * 	行の一鍵目の前のキー。行の一鍵目を打ちやすく選定するのに使います。
 * max_basic_kanji_per_line:
 * 	一つの行に最大の基本漢字の数。これから漢字がデータベースに入ってくる
 * 	とき 、基本漢字のコードが変わらないように、行に漢字を一杯つめません。
 */
void kanji_distribution_auto_pick_cutoff_exhaustive(
	struct kanji_distribution *, char prior_key,
	unsigned max_basic_kanji_per_line, int six_is_rh);

int kanji_distribution_parse_user_cutoff(
	struct kanji_distribution *kd,
	char const *const *argv,
	int argc);

void kanji_distribution_populate(struct kanji_distribution *kd);

void kanji_distribution_destroy(struct kanji_distribution *kd);

#endif
