#include "commands.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	int arr[] = {5, 10, 15, 17, 20};
	size_t size = sizeof(arr) / sizeof(*arr);

	start_test(__FILE__, "index_found_item");
	{
		ssize_t index;
		BSEARCH_INDEX(index, size, , arr[index] - 5);
		xfprintf(out, "%ld ", index);

		BSEARCH_INDEX(index, size, , arr[index] - 15);
		xfprintf(out, "%ld ", index);

		BSEARCH_INDEX(index, size, , arr[index] - 20);
		xfprintf(out, "%ld ", index);
	}
	end_test("0 2 4 ");

	start_test(__FILE__, "index_did_not_find");
	{
		ssize_t index;
		BSEARCH_INDEX(index, size, , arr[index] - 1);
		xfprintf(out, "%ld ", ~index);

		BSEARCH_INDEX(index, size, , arr[index] - 16);
		xfprintf(out, "%ld ", ~index);

		BSEARCH_INDEX(index, size, , arr[index] - 25);
		xfprintf(out, "%ld ", ~index);
	}
	end_test("0 3 5 ");
}
