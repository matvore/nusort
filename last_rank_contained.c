#include "commands.h"
#include "flags.h"
#include "kanji_distribution.h"
#include "streams.h"
#include "util.h"

/* コマンドフラグ */
#define SORT_EACH_LINE_BY_RSC	"-s"
#define HIDE_KANJI		"-k"
#define SHOW_PER_LINE_KANJI_CNT	"-n"
#define SHOW_SPACE_FOR_CUTOFF	"-c"

static void print_line_stats(struct flagset *fs, const struct line_stats *s)
{
	size_t i;
	char sep = flagval(fs, "--tsv") ? '\t' : ' ';

	if (flagval(fs, "--tsv"))
		fprintf(out, "%s\t%c", s->cutoff->c, s->key_ch);
	else
		fprintf(out, "[ %s ] %c", s->cutoff->c, s->key_ch);
	fputc(sep, out);

	if (!flagval(fs, HIDE_KANJI)) {
		for (i = 0; i < s->e_nr; i++) {
			if (flagval(fs, SHOW_SPACE_FOR_CUTOFF) &&
			    i &&
			    (s->e[i-1]->rsc_sort_key != s->e[i]->rsc_sort_key))
				fputc(' ', out);

			fputs(s->e[i]->c, out);
		}
		fputc(sep, out);
	}

	if (!flagval(fs, "--tsv")) fputc('(', out);

	fprintf(out, "%d . %d . %d",
		 s->last_char_rank, s->offset_to_target, s->cumulative_offset);
	if (flagval(fs, SHOW_PER_LINE_KANJI_CNT))
		fprintf(out, " . %d", s->e_nr);

	if (!flagval(fs, "--tsv")) fputc(')', out);

	fputc('\n', out);
}

static void print_stats_summary(struct kanji_distribution *kd)
{
	fprintf(out, "各行平均位: %.1f\n",
		 (float) kd->total_rank / kd->line_stats_nr);
	fprintf(out, "目標位:  %d\n", kd->target_rank);
	fprintf(out, "合計漢字数:  %d\n", kd->total_chars);
}

int last_rank_contained(struct flagset *fs, char **argv, int argc)
{
	struct kanji_distribution kanji_distribution = {0};
	size_t i;
	struct key_mapping_array romazi_m = {0};
	int res = 0;

	flagcat(fs, "print_last_rank_contained");
	addflag(fs, SORT_EACH_LINE_BY_RSC, 'b', 0, "");
	addflag(fs, HIDE_KANJI, 'b', 0, "");
	addflag(fs, SHOW_PER_LINE_KANJI_CNT, 'b', 0, "");
	addflag(fs, SHOW_SPACE_FOR_CUTOFF, 'b', 0, "");
	addflag(fs, "--tsv", 'b', 0, "");

	romazi_flags(fs);
	kanji_distribution_flags(fs);

	if (argc < 0) return 0;

	parsflag(fs, &argc, argv);

	if (	flagval(fs, SORT_EACH_LINE_BY_RSC)
	||	flagval(fs, SHOW_SPACE_FOR_CUTOFF)
	||	flagval(fs, "--tsv")
	) kanji_distribution.sort_each_line_by_rsc = 1;

	get_romazi_codes(fs, &romazi_m);
	kanji_distribution_set_preexisting_convs(
		fs, &kanji_distribution, &romazi_m, 1);

	if (argc) {
		res = kanji_distribution_parse_user_cutoff(
			&kanji_distribution, argv, argc);
		if (res) goto cleanup;
	} else {
		kanji_distribution_auto_pick_cutoff(&kanji_distribution);
	}

	kanji_distribution_populate(&kanji_distribution);

	for (i = 0; i < kanji_distribution.line_stats_nr; i++)
		print_line_stats(fs, &kanji_distribution.line_stats[i]);
	print_stats_summary(&kanji_distribution);

cleanup:
	kanji_distribution_destroy(&kanji_distribution);
	DESTROY_ARRAY(romazi_m);

	return res;
}
