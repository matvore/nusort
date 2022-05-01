#include "commands.h"
#include "streams.h"
#include "util.h"

#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *binname;

static int input_cmd(char const *const *argv, int argc)
{
	return input(argv, argc, /*set_raw_mode=*/1);
}

static const struct {
	int (*impl)(char const *const *argv, int argc);

	char const *name, *usage;
} commands[] = {
	{&free_kanji_keys, "free_kanji_keys", ""},
	{&h2k, "h2k", ""},
	{&input_cmd, "input", ""},
	{&kana_stats, "kana_stats", ""},
	{&kanji_db_chart, "kanji_db_chart", " [漢字数]"},
	{&check_kanji_db_order, "check_kanji_db_order", " [--db-out] [-q] [--residual-stroke-counts]"},
	{&print_last_rank_contained, "last_rank_contained", " [-c] [-s] [-k] [-n] CUTOFF_KANJI_1..CUTOFF_KANJI_n"},
	{&longest_rsc_block, "longest_rsc_block", ""},
	{&make_map, "make_map", ""},
	{&rsc_sort_key, "rsc_sort_key", ""},
	{0},
};

static void usage(FILE *stream)
{
	int ci;

	fprintf(stream, "USAGE: %s <command> <args...>\n", binname);

	fputs("COMMANDS:\n", stream);

	for (ci = 0; commands[ci].impl; ci++)
		fprintf(stream, "\t%s%s\n", commands[ci].name, commands[ci].usage);

	fputc('\n', stream);
}

int main(int argc, const char **argv)
{
	int ci;

	err = stderr;
	out = stdout;
	in = stdin;

	/* Windows で改行を特別に扱わないようにする。Linuxでは何もしない。
	 * 上記 (in, out, err) 以外のストリームでも渡せます。
	 */
#ifdef _MSC_VER
	setmode(fileno(stdin), O_BINARY);
	setmode(fileno(stdout), O_BINARY);
	setmode(fileno(stderr), O_BINARY);
#endif

	if (argc < 1) DIE(0, "argc: %d", argc);

	binname = *argv++;
	argc--;

	if (!argc
		|| !strcmp(*argv, "--help")
		|| !strcmp(*argv, "-h")
		|| !strcmp(*argv, "help")
		|| !strcmp(*argv, "-help")
	) {
		usage(stdout);
		exit(0);
	}

	for (ci = 0; commands[ci].impl; ci++) {
		if (!strcmp(commands[ci].name, *argv))
			exit(commands[ci].impl(argv+1, argc-1));
	}

	usage(stderr);

	exit(1);
}
