#include "commands.h"
#include "test_util.h"

int main(void)
{
	set_test_source_file(__FILE__);

	while (run_test("longest_rsc_block_acceptance_test", NULL))
		longest_rsc_block(NULL, 0);
}
