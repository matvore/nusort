#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	int arr[] = {5, 10, 15, 17, 20};
	size_t size = sizeof(arr) / sizeof(*arr);

	set_test_source_file(__FILE__);

	while (run_test("index_found_item", "0 2 4 ")) {
		ssize_t index;
		BSEARCH_INDEX(index, size, , arr[index] - 5);
		fprintf(out, "%zu ", index);

		BSEARCH_INDEX(index, size, , arr[index] - 15);
		fprintf(out, "%zu ", index);

		BSEARCH_INDEX(index, size, , arr[index] - 20);
		fprintf(out, "%zu ", index);
	}

	while (run_test("index_did_not_find", "0 3 5 ")) {
		ssize_t index;
		BSEARCH_INDEX(index, size, , arr[index] - 1);
		fprintf(out, "%zu ", ~index);

		BSEARCH_INDEX(index, size, , arr[index] - 16);
		fprintf(out, "%zu ", ~index);

		BSEARCH_INDEX(index, size, , arr[index] - 25);
		fprintf(out, "%zu ", ~index);
	}
}
