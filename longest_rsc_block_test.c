#include "commands.h"
#include "flags.h"
#include "test_util.h"

static struct flagset fs;

int main(void)
{
	set_test_source_file(__FILE__);

	while (run_test("longest_rsc_block_acceptance_test", NULL)) {
		longest_rsc_block(&fs, 0, 0);
		destroy_flagset(&fs);
	}
}
