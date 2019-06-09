#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xcalloc(size_t count, size_t size)
{
	void *res = calloc(count, size);
	if (!res) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}
	memset(res, 0, count * size);
	return res;
}
