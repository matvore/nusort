#include "commands.h"
#include "flags.h"
#include "kanji_db.h"
#include "residual_stroke_count.h"
#include "streams.h"
#include "util.h"

static long parse_count_arg(char const *arg)
{
	char *end;
	long c;

	if (!strcmp(arg, "0")) return 0;
	c = strtol(arg, &end, 10);
	if (*end || c < 1) {
		fprintf(err, "正の整数かゼロが必要です: %s\n", arg);
		return -1;
	}
	if (c > kanji_db_nr()) {
		fprintf(err, "漢字数を %u 個だけ表示できます。\n",
			kanji_db_nr());
		return kanji_db_nr();
	}
	return c;
}

int kanji_db_chart(struct flagset *fs, char **argv, int argc)
{
	unsigned ki, kscrel;
	struct {
		uint16_t ki;
		uint8_t dense_rank;
	} *sorted_k;
	long count;
	int format;
	uint32_t r;

	flagcat(fs, "kanji_db_chart");
	addflag(fs, "--chart-type", 'c', 'd',
"\n\t"	"d, t, T のどれか"
	);

	if (argc < 0) return 0;
	parsflag(fs, &argc, argv);

	if (argc && argv[0][0] == '-') badflag(argv[0]);
	format = flagval(fs, "--chart-type");
	if (!strchr("dtT", format))
		DIE(0, "--chart-type を dtT のどれか一つに設定してください。");

	switch (argc) {
	case 0:
		count = kanji_db_nr();
		break;
	case 1:
		count = parse_count_arg(argv[0]);
		if (count < 0) return 1;
		if (!count) return 0;
		break;
	default:
		fputs("引数を最大で一つだけ渡してください。\n", err);
		return 92;
	}

	sorted_k = xcalloc(kanji_db_nr(), sizeof(*sorted_k));

	for (ki = 0; ki < kanji_db_nr(); ki++)
		sorted_k[ki].ki = ki;

	QSORT(, sorted_k, kanji_db_nr(), kanji_db()[sorted_k[a].ki].ranking <
					 kanji_db()[sorted_k[b].ki].ranking);

	if (count > 1) {
		for (ki = 0, r = 0; ki < count; ki++, r += 61)
			sorted_k[ki].dense_rank = r / (count - 1);
	}

	QSORT(, sorted_k, count, kanji_db_rsc_index(kanji_db()+sorted_k[a].ki) <
				 kanji_db_rsc_index(kanji_db()+sorted_k[b].ki));

	kscrel = 0;
	for (ki = 0; ki < count; ki++) {
		struct kanji_entry const *e = kanji_db() + sorted_k[ki].ki;

		if (ki && e->cutoff_type && format == 'd') fputc('/', out);

		fputs(e->c, out);

		if (format == 'd') {
			print_base64_digit(sorted_k[ki].dense_rank);
			continue;
		}

		fprintf(out, "\t%d\t%d\t%d\n",
			kanji_db_rsc_index(e) - kscrel, e->ranking,
			residual_stroke_count(e));
		if (format == 'T') kscrel = kanji_db_rsc_index(e);
	}

	if (format == 'd') fputc('\n', out);

	FREE(sorted_k);

	return 0;
}
