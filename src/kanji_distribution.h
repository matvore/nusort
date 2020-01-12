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

	uint16_t total_rank;
	uint16_t total_chars;
	uint16_t target_rank;

	size_t line_stats_nr;
	struct line_stats line_stats[KANJI_KEY_COUNT];

	struct {
		struct kanji_entry const **el;
		size_t cnt;
		size_t alloc;
	} available;

	uint8_t unused_kanji_orig_cnts[KANJI_KEY_COUNT];
	struct {
		char (*el)[2];
		size_t cnt;
		size_t alloc;
	} unused_kanji_origs;
};

/*
Usage:
  1. memset to 0
  2. kanji_distribution_set_preexisting_convs を呼び出す
  3. kanji_distribution_auto_pick_cutoff 又は _parse_user_cutoff を呼び出す
  4. populate を呼び出す
  5. kanji_distribution_destroy を呼び出す
*/

/*
 * 既存のマッピングを設定します。設定すると入力コードとダブらないようにしたり、
 * 漢字を二つめの入力コードに割り当てません。
 */
void kanji_distribution_set_preexisting_convs(
	struct kanji_distribution *, struct key_mapping_array const *);

void kanji_distribution_auto_pick_cutoff(struct kanji_distribution *kd);

int kanji_distribution_parse_user_cutoff(
	struct kanji_distribution *kd,
	char const *const *argv,
	int argc);

void kanji_distribution_populate(struct kanji_distribution *kd);

void kanji_distribution_destroy(struct kanji_distribution *kd);
