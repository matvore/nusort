#include "commands.h"
#include "test_util.h"

int main(void)
{
	start_test(__FILE__, "free_kanji_keys_output");
	{
		free_kanji_keys(NULL, 0);
	}
	end_test_expected_content_in_file();
	return 0;
}
