#include "rank_coverage.h"
#include "util.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static int target_rank;
static int keys_unfilled;

static uint8_t *ranks;
static size_t ranks_nr;

/* ranksビット配列へのインデックス  */
static int position;

/*
 * positionとtarget_rankの間、漢字の数
 * 正の場合、positionがtarget_rankより高い
 * 負の場合、positionがtarget_rankより低い
 */
static int offset;

void rank_coverage_reset(int target_rank_, int key_capacity)
{
	target_rank = target_rank_;
	keys_unfilled = key_capacity;

	free(ranks);
	ranks_nr = 0;
}

static int rank_set(int rank) { return ranks[rank / 8] & (1 << (rank % 8)); }

static int count_in_range(int start_rank, int end_rank)
{
	int count = 0;
	while (start_rank < end_rank)
		count += rank_set(start_rank++) ? 1 : 0;
	return count;
}

int rank_coverage_add_kanji(int rank)
{
	size_t required_ranks_nr = rank / 8 + 1;
	size_t byte_i = rank / 8;
	uint8_t bit_mask = 1 << (rank % 8);

	if (required_ranks_nr > ranks_nr) {
		ranks = xreallocarray(ranks, required_ranks_nr, 1);
		memset(ranks + ranks_nr, 0, required_ranks_nr - ranks_nr);
		ranks_nr = required_ranks_nr;
	} else if (ranks[byte_i] & bit_mask) {
		fprintf(stderr, "順位がすでに追加されている: %d\n", rank);
		exit(142);
	}
	ranks[byte_i] |= bit_mask;
	if (keys_unfilled > 0) {
		if (--keys_unfilled)
			return 0x7fff;

		for (rank = ranks_nr * 8 - 1; !rank_set(rank); rank--) {}
		position = rank;

		offset = count_in_range(target_rank, position);
		return offset;
	}

	if (position > rank) {
		do {
			position--;
		} while (!rank_set(position));
		offset--;
	} else if (rank <= target_rank) {
		offset--;
	}

	return offset;
}
