#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xcalloc(size_t count, size_t size)
{
	void *res = calloc(count, size);
	if (!res) {
		fprintf(stderr, "out of memory\n");
		exit(90);
	}
	memset(res, 0, count * size);
	return res;
}

FILE *xfopen(const char *pathname, const char *mode)
{
	FILE *f = fopen(pathname, mode);
	if (!f) {
		fprintf(stderr,
			"%sを開く際にfopenが失配しました：\n\t%s\n",
			pathname, strerror(errno));
		exit(91);
	}
	return f;
}

char *xfgets(char *s, int size, FILE *stream)
{
	s = fgets(s, size, stream);
	if (!s && errno) {
		fprintf(stderr, "fgetsが失配しました:\t%s\n",
			strerror(errno));
		exit(92);
	}
	return s;
}

void xfclose(FILE *stream)
{
	if (!fclose(stream))
		return;
	fprintf(stderr, "警告: fcloseが失配しました\n:\t%s\n",
		strerror(errno));
}
