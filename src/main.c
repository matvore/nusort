#include "romazi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *USAGE =
"Usage: \n"
"\tnusort free_kanji_keys\n"
"\tnusort last_rank_contained CUTOFF_KANJI_1..CUTOFF_KANJI_39\n";

int main(int argc, char **argv)
{
	if (argc == 2 && !strcmp(argv[1], "free_kanji_keys")) {
		print_free_kanji_keys();
		return 0;
	}
	if (argc == 41 && !strcmp(argv[1], "last_rank_contained")) {
		fprintf(stderr, "not yet implemented\n");
		return 1;
	}
	fprintf(stderr, "%s", USAGE);
	return 1;
}
