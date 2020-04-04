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

	start_test("one_sort_key_in_range");
	{
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
	end_test("二");

	start_test("last_two_radicals");
	{
		unsigned key = kanji_db_lookup("龠")->rsc_sort_key;
		struct radical_coverage c = {
			.rsc_key_start = key - 1,
			.rsc_key_end = key + 1,
		};

		consume_radicals(&c);
	}
	end_test("龜龠");

	start_test("last_radical_with_really_big_end_key");
	{
		struct radical_coverage c = {
			.rsc_key_start = kanji_db_lookup("龠")->rsc_sort_key,
			.rsc_key_end = 0xffff,
		};

		consume_radicals(&c);
	}
	end_test("龠");

	start_test("first_three_radicals");
	{
		struct radical_coverage c = {
			.rsc_key_start = 0,
			.rsc_key_end = kanji_db_lookup("交")->rsc_sort_key + 1,
		};

		consume_radicals(&c);
	}
	end_test("一二亠");

	start_test("start_at_radical");
	{
		struct radical_coverage c = {
			.rsc_key_start = kanji_db_lookup("糸")->rsc_sort_key,
			.rsc_key_end = kanji_db_lookup("缶")->rsc_sort_key + 1,
		};

		consume_radicals(&c);
	}
	end_test("糸缶");

	start_test("stop_at_radical");
	{
		struct radical_coverage c = {
			.rsc_key_start = kanji_db_lookup("糸")->rsc_sort_key,
			.rsc_key_end = kanji_db_lookup("缶")->rsc_sort_key,
		};

		consume_radicals(&c);
	}
	end_test("糸");

	start_test("same_start_and_end");
	{
		unsigned key = kanji_db_lookup("調")->rsc_sort_key;
		struct radical_coverage c = {
			.rsc_key_start = key,
			.rsc_key_end = key,
		};

		consume_radicals(&c);
	}
	end_test("言");
}
