#include "input_impl.h"
#include "mapping.h"

int input(char const *const *argv, int argc)
{
	struct mapping mapping = {0};
	int res;

	mapping.ergonomic_sort = 1;
	mapping_populate(&mapping);

	res = input_impl(&mapping);

	mapping_destroy(&mapping);

	return res;
}
