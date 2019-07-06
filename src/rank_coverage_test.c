#include "commands.h"
#include "rank_coverage.h"
#include "test_util.h"

#include <stddef.h>
#include <stdio.h>

int main(void)
{
	start_test(__FILE__, "trivial");
	{
		size_t i;

		rank_coverage_reset(32, 32);
		for (i = 1; i <= 31; i++)
			rank_coverage_add_kanji(i);
		fprintf(out, "%d", rank_coverage_add_kanji(32));
	}
	end_test("0");

	start_test(__FILE__, "half_fits");
	{
		size_t i;
		rank_coverage_reset(2, 2);
		for (i = 1; i <= 4; i++)
			fprintf(out, "%d ", rank_coverage_add_kanji(i));
	}
	end_test("32767 0 0 0 ");

	return 0;
}
