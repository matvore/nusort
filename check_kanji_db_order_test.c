#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

#include <stdlib.h>

int main(void)
{
	struct flagset fs;
	int exit;

	set_test_source_file(__FILE__);

	while (run_test("check_kanji_db_order_test_quiet",
			"14190字の並べ替えキーを読み込み済み\n"
			"exit: 0\n")) {
		char *argv[] = {"-q"};
		memset(&fs, 0, sizeof fs);
		exit = check_kanji_db_order(&fs, argv, 1);
		fprintf(out, "exit: %d\n", exit);
		DESTROY_ARRAY(fs);
	}
}
