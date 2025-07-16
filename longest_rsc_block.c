#include "commands.h"
#include "kanji_db.h"
#include "streams.h"
#include "util.h"

static unsigned rsc_key(unsigned rsc_i)
{
	return kanji_from_rsc_index(rsc_i)->rsc_sort_key;
}

static void show_longest_blk(unsigned start, unsigned end)
{
	struct {
		struct kanji_entry const **el;
		size_t alloc, cnt;
	} kanjis = {0};
	unsigned i;

	fputs("字  順位\n"
	      "\n", out);
	for (i = start; i < end; i++) {
		GROW_ARRAY_BY(kanjis, 1);
		kanjis.el[kanjis.cnt - 1] = kanji_from_rsc_index(i);
	}

	QSORT(, kanjis.el, kanjis.cnt,
	      kanjis.el[a]->ranking < kanjis.el[b]->ranking);

	for (i = 0; i < kanjis.cnt; i++)
		fprintf(out, "%s %5u\n",
			kanjis.el[i]->c, kanjis.el[i]->ranking);

	fprintf(out, "\n字数: %u\n", end - start);

	DESTROY_ARRAY(kanjis);
}

int longest_rsc_block(char const *const *argv, int argc)
{
	unsigned rsc_i;
	unsigned longest_blk_size = 1;
	unsigned longest_blk_start;
	unsigned curr_block_start = 0;

	if (argc) {
		fputs("引数を渡さないでください。\n", err);
		return 174;
	}

	for (rsc_i = 1; rsc_i <= kanji_db_nr(); rsc_i++) {
		if (rsc_i < kanji_db_nr() &&
		    rsc_key(rsc_i) == rsc_key(rsc_i - 1))
			continue;
		if (longest_blk_size < rsc_i - curr_block_start) {
			longest_blk_size = rsc_i - curr_block_start;
			longest_blk_start = curr_block_start;
		}
		curr_block_start = rsc_i;
	}

	show_longest_blk(longest_blk_start,
			 longest_blk_start + longest_blk_size);

	return 0;
}
