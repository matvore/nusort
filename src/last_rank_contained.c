#include "commands.h"
#include "kanji_distribution.h"
#include "util.h"

/* コマンドフラグ */
static int hide_kanji;
static int show_per_line_kanji_count;

static void print_line_stats(const struct line_stats *s)
{
	size_t i;

	xfprintf(out, "[ %s ] %c ", s->cutoff->c, s->key_ch);

	if (!hide_kanji) {
		for (i = 0; i < s->e_nr; i++)
			xfprintf(out, "%s", s->e[i]->c);
		xfprintf(out, " ");
	}

	xfprintf(out, "(%d . %d . %d",
		 s->last_char_rank, s->offset_to_target, s->cumulative_offset);
	if (show_per_line_kanji_count)
		xfprintf(out, " . %d", s->e_nr);

	xfprintf(out, ")\n");
}

static void print_stats_summary(struct kanji_distribution *kd)
{
	xfprintf(out, "各行平均位: %.1f\n",
		 (float) kd->total_rank / kd->line_stats_nr);
	xfprintf(out, "目標位:  %d\n", kd->target_rank);
	xfprintf(out, "合計漢字数:  %d\n", kd->total_chars);
}

int print_last_rank_contained(char const *const *argv, int argc)
{
	struct kanji_distribution kanji_distribution = {0};
	size_t i;

	show_per_line_kanji_count = 0;

	while (argc > 0 && argv[0][0] == '-') {
		const char *arg = argv[0];
		argc--;
		argv++;
		if (!strcmp(arg, "-s")) {
			kanji_distribution.sort_each_line_by_rad_so = 1;
		} else if (!strcmp(arg, "-k")) {
			hide_kanji = 1;
		} else if (!strcmp(arg, "-n")) {
			show_per_line_kanji_count = 1;
		} else if (!strcmp(arg, "--")) {
			break;
		} else {
			xfprintf(err,
				 "フラグを認識できませんでした：%s\n", arg);
			return 3;
		}
	}

	if (argc) {
		int res = kanji_distribution_parse_user_cutoff(
			&kanji_distribution, argv, argc);
		if (res)
			return res;
	} else {
		kanji_distribution_auto_pick_cutoff(&kanji_distribution);
	}

	kanji_distribution_populate(&kanji_distribution);

	for (i = 0; i < kanji_distribution.line_stats_nr; i++)
		print_line_stats(&kanji_distribution.line_stats[i]);
	print_stats_summary(&kanji_distribution);

	return 0;
}
