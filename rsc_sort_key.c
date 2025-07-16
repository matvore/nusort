#include <stdio.h>

#include "commands.h"
#include "kanji_db.h"
#include "residual_stroke_count.h"
#include "streams.h"

int rsc_sort_key(char const *const *argv, int argc)
{
	if (argc) {
		fputs("引数を渡さないでください。\n", err);
		return 1;
	}

	fprintf(out, "最大値: %u\n", largest_rsc_sort_key());
	fprintf(out, "最大の部首外画数: %d\n", largest_residual_stroke_count());

	return 0;
}
