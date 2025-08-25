#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

#include <stdlib.h>

static struct flagset fs;
static void cleanup(void) { destroy_flagset(&fs); }

int main(void)
{
	set_test_source_file(__FILE__);
	while (run_test("rsc_gaps_test", 0)) {
		int exit = rsc_gaps(&fs, 0, 0);
		fprintf(out, "exit: %d\n", exit);
		cleanup();
	}
}
