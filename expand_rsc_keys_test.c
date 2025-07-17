#include "commands.h"
#include "streams.h"
#include "test_util.h"

int main(void)
{
	set_test_source_file(__FILE__);
	while (run_test("expand_rsc_keys", 0)) {
		char const *const args[] = {"-1", 0};
		fprintf(out, "err: %d\n", expand_rsc_keys(args, 1));
	}
}
