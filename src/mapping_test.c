#include <inttypes.h>
#include <stdio.h>

#include "kanji_db.h"
#include "mapping.h"
#include "mapping_util.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static void check_rank_order(
	struct key_mapping_array const *m, char const *a, char const *b)
{
	struct key_mapping const *a_m, *b_m;
	struct kanji_entry const *a_k, *b_k;

	BSEARCH(a_m, m->el, m->cnt, code_cmp(a_m->orig, a));
	if (!a_m) {
		fprintf(out, "コードが見つからない: %s", a);
		return;
	}

	BSEARCH(b_m, m->el, m->cnt, code_cmp(b_m->orig, b));
	if (!b_m) {
		fprintf(out, "コードが見つからない: %s", b);
		return;
	}

	a_k = kanji_db_lookup(a_m->conv);
	b_k = kanji_db_lookup(b_m->conv);

	if (a_k->ranking < b_k->ranking)
		return;

	fprintf(out, "%s の順位 (%"PRIu16") >= %s の順位 (%"PRIu16")\n",
		a_k->c, a_k->ranking, b_k->c, b_k->ranking);
}

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("6_is_rh_6s_is_higher_ranked_kanji_than_6k");
	{
		struct mapping m = {
			.six_is_rh = 1,
			.include_kanji = 1,
		};

		mapping_populate(&m);
		check_rank_order(&m.arr, "6s", "6k");
		destroy_mapping(&m);
	}
	end_test("");

	start_test("6_is_lh_6k_is_higher_ranked_kanji_than_6s");
	{
		struct mapping m = {
			.six_is_rh = 0,
			.include_kanji = 1,
		};

		mapping_populate(&m);
		check_rank_order(&m.arr, "6k", "6s");
		destroy_mapping(&m);
	}
	end_test("");

	start_test("6_is_lh_6o_is_higher_ranked_kanji_than_6q");
	{
		struct mapping m = {
			.six_is_rh = 0,
			.include_kanji = 1,
		};

		mapping_populate(&m);
		check_rank_order(&m.arr, "6o", "6q");
		destroy_mapping(&m);
	}
	end_test("");

	start_test("6_is_lh_k6_is_higher_ranked_kanji_than_ki");
	{
		struct mapping m = {
			.six_is_rh = 0,
			.include_kanji = 1,
		};

		mapping_populate(&m);
		check_rank_order(&m.arr, "k6", "ki");
		destroy_mapping(&m);
	}
	end_test("");
}
