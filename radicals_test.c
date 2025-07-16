#include "kanji_db.h"
#include "radicals.h"
#include "streams.h"
#include "test_util.h"

static void verify_done_is(struct radical_coverage *cov, int val)
{
	int done = radical_coverage_done(cov);
	if (val != !!done)
		fprintf(out,
			"radical_coverage_done の状態が間違っています: %d\n",
			done);
}

static void consume_radicals(struct radical_coverage *cov)
{
	while (1) {
		radical_coverage_next(cov);
		if (radical_coverage_done(cov))
			break;
		fputs(kanji_db()[cov->current].c, out);
	}
}

int main(void)
{
	set_test_source_file(__FILE__);

	while (run_test("one_sort_key_in_range", "二")) {
		unsigned key = kanji_db_lookup("些")->rsc_sort_key;
		struct radical_coverage c = {
			.rsc_key_start = key,
			.rsc_key_end = key + 1,
		};
		radical_coverage_next(&c);
		verify_done_is(&c, 0);
		verify_done_is(&c, 0);
		fputs(kanji_db()[c.current].c, out);
		radical_coverage_next(&c);
		verify_done_is(&c, 1);
		verify_done_is(&c, 1);
	}

	while (run_test("last_two_radicals", "龜龠")) {
		unsigned key = kanji_db_lookup("龠")->rsc_sort_key;
		struct radical_coverage c = {
			.rsc_key_start = key - 1,
			.rsc_key_end = key + 1,
		};

		consume_radicals(&c);
	}

	while (run_test("last_radical_with_really_big_end_key", "龠")) {
		struct radical_coverage c = {
			.rsc_key_start = kanji_db_lookup("龠")->rsc_sort_key,
			.rsc_key_end = 0xffff,
		};

		consume_radicals(&c);
	}

	while (run_test("first_three_radicals", "一二亠")) {
		struct radical_coverage c = {
			.rsc_key_start = 0,
			.rsc_key_end = kanji_db_lookup("交")->rsc_sort_key + 1,
		};

		consume_radicals(&c);
	}

	while (run_test("start_at_radical", "糸缶")) {
		struct radical_coverage c = {
			.rsc_key_start = kanji_db_lookup("糸")->rsc_sort_key,
			.rsc_key_end = kanji_db_lookup("缶")->rsc_sort_key + 1,
		};

		consume_radicals(&c);
	}

	while (run_test("stop_at_radical", "糸")) {
		struct radical_coverage c = {
			.rsc_key_start = kanji_db_lookup("糸")->rsc_sort_key,
			.rsc_key_end = kanji_db_lookup("缶")->rsc_sort_key,
		};

		consume_radicals(&c);
	}

	while (run_test("same_start_and_end", "言")) {
		unsigned key = kanji_db_lookup("調")->rsc_sort_key;
		struct radical_coverage c = {
			.rsc_key_start = key,
			.rsc_key_end = key,
		};

		consume_radicals(&c);
	}
}
