#include "romazi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *USAGE =
"Usage:\n"
"\tnusort check_kanji_db_order [--db-out]\n"
"\tnusort free_kanji_keys\n"
"\tnusort last_rank_contained [-s] CUTOFF_KANJI_1..CUTOFF_KANJI_n\n";

int check_kanji_db_order(const char **argv, int argc);
int print_last_rank_contained(const char **argv, int argc);

int main(int argc, const char **argv)
{
	if (argc == 2 && !strcmp(argv[1], "free_kanji_keys")) {
		print_free_kanji_keys();
		return 0;
	}
	if (argc >= 2 && !strcmp(argv[1], "check_kanji_db_order"))
		return check_kanji_db_order(argv + 2, argc - 2);
	if (argc >= 2 && !strcmp(argv[1], "last_rank_contained"))
		return print_last_rank_contained(argv + 2, argc - 2);
	fprintf(stderr, "%s", USAGE);
	return 1;
}
