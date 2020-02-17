#include "commands.h"
#include "test_util.h"

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("longest_rsc_block_acceptance_test");
	longest_rsc_block(NULL, 0);
	end_test_expected_content_in_file();
}
