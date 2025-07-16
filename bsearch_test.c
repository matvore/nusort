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
		long index;
		BSEARCH_INDEX(index, size, , arr[index] - 5);
		fprintf(out, "%ld ", index);

		BSEARCH_INDEX(index, size, , arr[index] - 15);
		fprintf(out, "%ld ", index);

		BSEARCH_INDEX(index, size, , arr[index] - 20);
		fprintf(out, "%ld ", index);
	}

	while (run_test("index_did_not_find", "0 3 5 ")) {
		long index;
		BSEARCH_INDEX(index, size, , arr[index] - 1);
		fprintf(out, "%ld ", ~index);

		BSEARCH_INDEX(index, size, , arr[index] - 16);
		fprintf(out, "%ld ", ~index);

		BSEARCH_INDEX(index, size, , arr[index] - 25);
		fprintf(out, "%ld ", ~index);
	}
}
