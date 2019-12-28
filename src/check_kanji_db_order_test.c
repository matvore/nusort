#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

#include <stdlib.h>

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("check_kanji_db_order_test_quiet");
	{
		const char *argv[] = {"-q"};
		int exit = check_kanji_db_order(argv, 1);
		xfprintf(out, "exit: %d\n", exit);
	}
	end_test("13454字の並べ替えキーを読み込み済み\n"
		 "exit: 0\n");
}
