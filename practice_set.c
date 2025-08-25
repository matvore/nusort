#include "commands.h"
#include "flags.h"
#include "kanji_db.h"
#include "kanji_distribution.h"
#include "mapping.h"
#include "radicals.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

/* 任意のマッピングに対する漢字練習セットを生成 */
int practice_set(struct flagset *fs, char **argv, int argc)
{
	struct kanji_distribution kdist = {.sort_each_line_by_rsc = 1};
	struct key_mapping_array mapping = {0};
	int res = 0;
	unsigned this_rank, best_rank, ki;
	const char *bestk;
	struct kanji_entry const *ke;
	struct line_stats const *ls;

	romazi_flags(fs);
	mapping_flags(fs);

	if (argc < 0) return 0;

	parsflag(fs, &argc, argv);

	if (argc) badflag(*argv);

	get_romazi_codes(fs, &mapping);

	kanji_distribution_set_preexisting_convs(
		fs, &kdist, &mapping, /*block_already_used=*/1);
	kanji_distribution_auto_pick_cutoff(&kdist);
	kanji_distribution_populate(&kdist);

	bestk = NULL;
	ls = kdist.line_stats;

	for (ki = 0;; ki++) {
		ke = kanji_from_rsc_index(ki);

		if (ke->cutoff_type >= 2) {
			if (bestk) fprintf(out, "%s\n", bestk);
			bestk = NULL;
			best_rank = (unsigned) -1;
		}

		this_rank = ke->ranking;
		if (ls->e[0] == ke)
			this_rank = 0;
		else if (ls - kdist.line_stats < kdist.line_stats_nr-1 &&
			 ls[1].e[0]->rsc_sort_key == ke->rsc_sort_key+1) {
			this_rank = 0;
			ls++;
		}
		else if (ke->rsc_sort_key == largest_rsc_sort_key()) {
			fprintf(out, "%s\n", ke->c);
			break;
		}

		if (this_rank > best_rank) continue;

		if (!best_rank) fprintf(out, "%s\n", bestk);

		/* kanji_distribution 行の初めか終わりに使われている場合、
		 * best_rank == 0 */
		best_rank = this_rank;
		bestk = ke->c;
	}

	kanji_distribution_destroy(&kdist);
	DESTROY_ARRAY(mapping);

	return res;
}
