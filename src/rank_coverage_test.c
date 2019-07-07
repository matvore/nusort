#include "commands.h"
#include "rank_coverage.h"
#include "test_util.h"

#include <stddef.h>
#include <stdio.h>

static void add(int rank)
{
	fprintf(out, "%d ", rank_coverage_add_kanji(rank));
}

int main(void)
{
	start_test(__FILE__, "trivial");
	{
		size_t i;

		rank_coverage_reset(10, 10);
		for (i = 1; i <= 10; i++)
			add(i);
	}
	end_test("32767 32767 32767 32767 32767 32767 32767 32767 32767 0 ");

	start_test(__FILE__, "half_fits");
	{
		size_t i;
		rank_coverage_reset(2, 2);
		for (i = 1; i <= 4; i++)
			add(i);
	}
	end_test("32767 0 0 0 ");

	start_test(__FILE__, "too_much_capacity");
	{
		rank_coverage_reset(5, 3);
		add(1);
		add(4);
		add(10);
		add(12);
	}
	end_test("32767 32767 1 1 ");

	start_test(__FILE__, "too_much_capacity_reversed");
	{
		rank_coverage_reset(5, 3);
		add(12);
		add(10);
		add(4);
		add(1);
	}
	end_test("32767 32767 2 1 ");

	start_test(__FILE__, "insufficient_capacity");
	{
		rank_coverage_reset(10, 3);
		add(2);
		add(10);
		add(4);
		add(1);
	}
	end_test("32767 32767 0 -1 ");

	start_test(__FILE__, "sufficient_capacity");
	{
		rank_coverage_reset(9, 3);
		add(10);
		add(2);
		add(4);
		add(1);
	}
	end_test("32767 32767 1 0 ");
	return 0;
}
