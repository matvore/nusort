#include "commands.h"
#include "streams.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *USAGE =
"Usage:\n"
"\tnusort check_kanji_db_order [--db-out] [-q] [--residual-stroke-counts]\n"
"\tnusort free_kanji_keys\n"
"\tnusort h2k\n"
"\tnusort input\n"
"\tnusort kana_stats\n"
"\tnusort kanji_db_chart [漢字数]\n"
"\tnusort last_rank_contained [-c] [-s] [-k] [-n] "
	"CUTOFF_KANJI_1..CUTOFF_KANJI_n\n"
"\tnusort longest_rsc_block\n"
"\tnusort make_map\n"
"\tnusort rsc_sort_key\n"
;

int main(int argc, const char **argv)
{
	err = stderr;
	out = stdout;
	in = stdin;

	if (argc >= 2 && !strcmp(argv[1], "free_kanji_keys"))
		return free_kanji_keys(argv + 2, argc - 2);
	if (argc >= 2 && !strcmp(argv[1], "h2k"))
		return h2k(argv + 2, argc - 2);
	if (argc >= 2 && !strcmp(argv[1], "input"))
		return input(argv + 2, argc - 2, /*set_raw_mode=*/1);
	if (argc >= 2 && !strcmp(argv[1], "kana_stats"))
		return kana_stats(argv + 2, argc - 2);
	if (argc >= 2 && !strcmp(argv[1], "kanji_db_chart"))
		return kanji_db_chart(argv + 2, argc - 2);
	if (argc >= 2 && !strcmp(argv[1], "check_kanji_db_order"))
		return check_kanji_db_order(argv + 2, argc - 2);
	if (argc >= 2 && !strcmp(argv[1], "last_rank_contained"))
		return print_last_rank_contained(argv + 2, argc - 2);
	if (argc >= 2 && !strcmp(argv[1], "longest_rsc_block"))
		return longest_rsc_block(argv + 2, argc - 2);
	if (argc >= 2 && !strcmp(argv[1], "make_map"))
		return make_map(argv + 2, argc - 2);
	if (argc >= 2 && !strcmp(argv[1], "rsc_sort_key"))
		return rsc_sort_key(argv + 2, argc - 2);
	fprintf(stderr, "%s", USAGE);
	return 1;
}
