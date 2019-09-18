#include "kanji_db.h"
#include "romazi.h"

#include <stdint.h>

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
	unsigned sort_each_line_by_rad_so : 1;

	uint16_t total_rank;
	uint16_t total_chars;
	uint16_t target_rank;

	size_t line_stats_nr;
	struct line_stats line_stats[KANJI_KEY_COUNT];
};

/*
Usage:
  1. memset to 0
  2. call _auto_pick_cutoff or _parse_user_cutoff
  3. call populate
*/

void kanji_distribution_auto_pick_cutoff(struct kanji_distribution *kd);

int kanji_distribution_parse_user_cutoff(
	struct kanji_distribution *kd,
	const char **argv,
	int argc);

void kanji_distribution_populate(struct kanji_distribution *kd);
