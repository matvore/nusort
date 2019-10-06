#include "commands.h"
#include "test_util.h"
#include "util.h"

static struct {
	char const *name;
	char const *in;
} test_cases[] = {

{"first_key_k", "kj"},

};

int main(void)
{
	size_t test_i;
	for (test_i = 0; test_i < sizeof(test_cases) / sizeof(*test_cases);
	     test_i++) {
		char input_file[] = "/tmp/input_test_in_XXXXXX";

		start_test(__FILE__, test_cases[test_i].name);

		store_in_tmp_file(test_cases[test_i].in, input_file);

		in = xfopen(input_file, "r");
		input(0, 0);
		xfclose(in);
		in = stdin;

		end_test_expected_content_in_file();
	}
	return 0;
}
