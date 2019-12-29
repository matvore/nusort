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
	set_test_source_file(__FILE__);

	start_test("trivial");
	{
		size_t i;

		rank_coverage_reset(10, 10);
		for (i = 1; i <= 10; i++)
			add(i);
	}
	end_test("32767 32767 32767 32767 32767 32767 32767 32767 32767 0 ");

	start_test("half_fits");
	{
		size_t i;
		rank_coverage_reset(2, 2);
		for (i = 1; i <= 4; i++)
			add(i);
	}
	end_test("32767 0 0 0 ");

	start_test("too_much_capacity");
	{
		rank_coverage_reset(5, 3);
		add(1);
		add(4);
		add(10);
		add(12);
	}
	end_test("32767 32767 1 1 ");

	start_test("too_much_capacity_reversed");
	{
		rank_coverage_reset(5, 3);
		add(12);
		add(10);
		add(4);
		add(1);
	}
	end_test("32767 32767 2 1 ");

	start_test("insufficient_capacity");
	{
		rank_coverage_reset(10, 3);
		add(2);
		add(10);
		add(4);
		add(1);
	}
	end_test("32767 32767 0 -1 ");

	start_test("sufficient_capacity");
	{
		rank_coverage_reset(9, 3);
		add(10);
		add(2);
		add(4);
		add(1);
	}
	end_test("32767 32767 1 0 ");

	start_test("first_key");
	{
		rank_coverage_reset(1261, 40);
		add(1547);	add(2651);	add(3868);	add(839);
		add(348);	 add(852);	add(4052);	add(1620);
		add(346);	add(848);	add(20);	add(48);
		add(677);	add(3825);	add(2325);	add(1351);
		add(744);	add(486);	add(1169);	add(14);
		add(161);	add(132);	add(2096);	add(1452);
		add(3374);	add(1609);	add(194);	add(3508);
		add(3760);	add(2190);	add(1523);	add(224);
		add(3841);	add(1278);	add(3066);	add(662);
		add(828);	add(2821);	add(1108);	add(1663);
		add(6);		add(599);	add(2884);	add(902);
		add(358);	add(1117);	add(1301);	add(1558);
		add(860);	add(538);	add(2553);	add(1434);
		add(2263);	add(1142);	add(324);	add(275);
		add(1421);	add(2); 	add(10);	add(728);
	}
	end_test("32767 32767 32767 32767 32767 32767 32767 32767 32767 32767 "
		 "32767 32767 32767 32767 32767 32767 32767 32767 32767 32767 "
		 "32767 32767 32767 32767 32767 32767 32767 32767 32767 32767 "
		 "32767 32767 32767 32767 32767 32767 32767 32767 32767 21 20 "
		 "19 19 18 17 16 16 16 15 14 14 14 14 13 12 11 11 10 9 8 ");

	return 0;
}
