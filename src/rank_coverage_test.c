#include "rank_coverage.h"

#include <stddef.h>
#include <stdio.h>

int main(void)
{
	size_t i;
	int res;

	rank_coverage_reset(32, 32);
	for (i = 1; i <= 31; i++)
		rank_coverage_add_kanji(i);
	res = rank_coverage_add_kanji(32);
	if (res != 0) {
		fprintf(stderr, "fail: %d\n", res);
		return 1;
	}
	return 0;
}
