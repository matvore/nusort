#include "commands.h"
#include "kanji_db.h"
#include "streams.h"
#include "util.h"

static unsigned parse_count_arg(char const *arg)
{
	char *end;
	long c = strtol(arg, &end, 10);
	if (*end || c < 1) {
		fprintf(err, "正の整数ではない: %s\n", arg);
		return 0;
	}
	if (c > kanji_db_nr()) {
		fprintf(err, "漢字数を %u 個だけ表示できます。\n",
			kanji_db_nr());
		return kanji_db_nr();
	}
	return c;
}

int kanji_db_chart(char const *const *argv, int argc)
{
	unsigned ki;
	struct {
		uint16_t ki;
		uint8_t dense_rank;
	} *sorted_k;
	long count;

	switch (argc) {
	case 0:
		count = kanji_db_nr();
		break;
	case 1:
		count = parse_count_arg(argv[0]);
		if (!count)
			return 93;
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
		uint32_t r = 0;
		for (ki = 0; ki < count; ki++, r += 61)
			sorted_k[ki].dense_rank = r / (count - 1);
	}

	QSORT(, sorted_k, count, kanji_db_rsc_index(kanji_db()+sorted_k[a].ki) <
				 kanji_db_rsc_index(kanji_db()+sorted_k[b].ki));

	for (ki = 0; ki < count; ki++) {
		struct kanji_entry const *e = kanji_db() + sorted_k[ki].ki;

		if (ki && e->cutoff_type)
			fputc('/', out);

		fputs(e->c, out);

		print_base64_digit(sorted_k[ki].dense_rank);
	}

	fputc('\n', out);

	FREE(sorted_k);

	return 0;
}
