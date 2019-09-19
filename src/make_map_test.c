#include "commands.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	start_test(__FILE__, "default_mapping");
	{
		xfprintf(err, "exit code: %d\n", make_map(NULL, 0));
	}
	end_test_expected_content_in_file();
	return 0;
}
