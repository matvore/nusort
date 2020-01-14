#include "commands.h"
#include "kanji_distribution.h"
#include "streams.h"
#include "util.h"

/* コマンドフラグ */
static struct {
	unsigned hide_kanji : 1;
	unsigned show_per_line_kanji_count : 1;
	unsigned show_space_for_cutoff_points : 1;
} flags;

static void print_line_stats(const struct line_stats *s)
{
	size_t i;

	xfprintf(out, "[ %s ] %c ", s->cutoff->c, s->key_ch);

	if (!flags.hide_kanji) {
		for (i = 0; i < s->e_nr; i++) {
			if (flags.show_space_for_cutoff_points &&
			    i &&
			    (s->e[i-1]->rsc_sort_key != s->e[i]->rsc_sort_key))
				xfputc(' ', out);

			xfputs(s->e[i]->c, out);
		}
		xfputc(' ', out);
	}

	xfprintf(out, "(%d . %d . %d",
		 s->last_char_rank, s->offset_to_target, s->cumulative_offset);
	if (flags.show_per_line_kanji_count)
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
	struct romazi_config romazi_config = {0};
	struct key_mapping_array romazi_m = {0};

	memset(&flags, 0, sizeof(flags));
	init_romazi_config_for_cli_flags(&romazi_config);

	while (argc > 0 && argv[0][0] == '-') {
		if (!strcmp(argv[0], "-s")) {
			kanji_distribution.sort_each_line_by_rsc = 1;
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-k")) {
			flags.hide_kanji = 1;
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-n")) {
			flags.show_per_line_kanji_count = 1;
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-c")) {
			kanji_distribution.sort_each_line_by_rsc = 1;
			flags.show_space_for_cutoff_points = 1;
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "--")) {
			argc--;
			argv++;
			break;
		} else if (!parse_romazi_flags(&argc, &argv, &romazi_config)) {
			xfprintf(err,
				 "フラグを認識できませんでした：%s\n", argv[0]);
			return 3;
		}
	}

	get_romazi_codes(&romazi_config, &romazi_m);
	kanji_distribution_set_preexisting_convs(
		&kanji_distribution, &romazi_m);

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

	kanji_distribution_destroy(&kanji_distribution);
	DESTROY_ARRAY(romazi_m);

	return 0;
}
