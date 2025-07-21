#include "commands.h"
#include "kanji_db.h"
#include "radicals.h"
#include "residual_stroke_count.h"
#include "streams.h"
#include "util.h"

int rsc_gaps(char **argv, int argc)
{
	int radin = -1, rsck = 0, psc = 0xfff, csc;

	if (argc) badflag(*argv);

	for (;;) {
		if (++rsck > largest_rsc_sort_key()) break;
		csc = residual_stroke_count_from_rsc_sort_key(rsck);

		if (!csc) radin++; else {
			fprintf(out,	"%02x %02x->%02x %02x\n",
					radical_num(radin), psc, csc, csc-psc);
		}

		psc = csc;
	}

	return 0;
}
