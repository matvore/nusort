#include <stdio.h>

#include "commands.h"
#include "kanji_db.h"
#include "streams.h"

int rsc_sort_key(char const *const *argv, int argc)
{
	if (argc) {
		fputs("引数を渡さないでください。\n", err);
		return 1;
	}

	fprintf(out, "最大値: %u\n",
		kanji_db()[kanji_db_rsc_sorted()[kanji_db_nr() - 1]]
			.rsc_sort_key);

	return 0;
}