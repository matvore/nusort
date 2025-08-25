#include "commands.h"
#include "flags.h"
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

static void usage(FILE *);
static void cmdhelp(const char *);

static int help(struct flagset *fs, char **argv, int argc)
{
	int res = 0;

	if (argc < 0) return 0;

	parsflag(fs, &argc, argv);

	if (argc > 1) { fprintf(err, "引数が多すぎます。\n"); res = 1; }

	if (argc)	cmdhelp(*argv);
	else		usage(stdout);

	return res;
}

static const struct {
	int (*impl)(struct flagset *fs, char **argv, int argc);

	char const *name, *shor, *usage;
} commands[] = {
	{&free_kanji_keys, "free_kanji_keys", "frkk", ""},
	{&h2k, "hira_to_kata", "h2k", ""},
	{&input, "input", "in", ""},
	{&kana_stats, "kana_stats", "ks", ""},
	{&kanji_db_chart, "kanji_db_chart", "ch", " [漢字数]"},
	{&check_kanji_db_order, "check_kanji_db_order", "or", ""},
	{&last_rank_contained, "last_rank_contained", "last", " CUTOFF_KANJI_1..CUTOFF_KANJI_n"},
	{&longest_rsc_block, "longest_rsc_block", "lb", ""},
	{&make_map, "make_map", "mm", ""},
	{&practice_set, "practice_set", "ps", ""},
	{&rsc_gaps, "rsc_gaps", "rg", ""},
	{&rsc_sort_key, "rsc_sort_key", "rk", ""},
	{&expand_rsc_keys, "expand_rsc_keys", "exk", ""},
	{&help, "help", "-h", " [コマンド名]"},
	{0},
};

static void printcategnames(FILE *stream, int ci)
{
	struct flagset fs = {
		.categ_out = stream,
		.not_collecting = 1,
	};

	commands[ci].impl(&fs, 0, -1);

	if (fs.past_first_cat) fputs("}", stream);

	destroy_flagset(&fs);
}

static void printflagdetails(FILE *s, int ci)
{
	struct flagset fs = {
		.doc_out = s,
		.not_collecting = 1,
	};

	commands[ci].impl(&fs, 0, -1);

	destroy_flagset(&fs);
}

static void cmdhelp(const char *cn)
{
	int ci;

	for (ci = 0; commands[ci].impl; ci++) {
		if (	strcmp(cn, commands[ci].name)
		&&	strcmp(cn, commands[ci].shor)
		) continue;

		fprintf(stdout, "用法: %s %s", binname, cn);

		printcategnames(stdout, ci);
		fprintf(stdout, "%s\n", commands[ci].usage);
		printflagdetails(stdout, ci);
		return;
	}

	usage(stderr);
	fprintf(stderr, "コマンド名が無効です: %s\n", cn);

	exit(1);
}

static void usage(FILE *stream)
{
	int ci;

	fprintf(stream, "USAGE: %s <command> <args...>\n", binname);

	fputs("COMMANDS:\n", stream);

	for (ci = 0; commands[ci].impl; ci++) {
		fprintf(stream,	"\t%s", commands[ci].name);

		printcategnames(stream, ci);

		fprintf(stream, "%s (or: %s)\n",
				commands[ci].usage,
				commands[ci].shor);
	}

	fputc('\n', stream);
}

int main(int argc, char **argv)
{
	int ci;
	struct flagset fs = {0};

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

	if (!argc) { usage(stdout); exit(0); }

	for (ci = 0; commands[ci].impl; ci++) {
		if (	!strcmp(commands[ci].name, *argv)
		||	!strcmp(commands[ci].shor, *argv)
		) exit(commands[ci].impl(&fs, argv+1, argc-1));
	}

	usage(stderr);

	exit(1);
}
