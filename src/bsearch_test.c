#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

int main(void)
{
	int arr[] = {5, 10, 15, 17, 20};
	size_t size = sizeof(arr) / sizeof(*arr);

	set_test_source_file(__FILE__);

	start_test("index_found_item");
	{
		ssize_t index;
		BSEARCH_INDEX(index, size, , arr[index] - 5);
		xfprintf(out, "%zu ", index);

		BSEARCH_INDEX(index, size, , arr[index] - 15);
		xfprintf(out, "%zu ", index);

		BSEARCH_INDEX(index, size, , arr[index] - 20);
		xfprintf(out, "%zu ", index);
	}
	end_test("0 2 4 ");

	start_test("index_did_not_find");
	{
		ssize_t index;
		BSEARCH_INDEX(index, size, , arr[index] - 1);
		xfprintf(out, "%zu ", ~index);

		BSEARCH_INDEX(index, size, , arr[index] - 16);
		xfprintf(out, "%zu ", ~index);

		BSEARCH_INDEX(index, size, , arr[index] - 25);
		xfprintf(out, "%zu ", ~index);
	}
	end_test("0 3 5 ");
}
