#define _GNU_SOURCE

#include "util.h"

#include <errno.h>
#include <stdarg.h>
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
	return res;
}

void *xreallocarray(void *ptr, size_t count, size_t el_size)
{
	ptr = realloc(ptr, count * el_size);
	if (!ptr && count) {
		fprintf(stderr, "out of memory\n");
		exit(90);
	}
	return ptr;
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

int xfprintf(FILE *stream, const char *format, ...)
{
	int res;

	va_list argp;
	va_start(argp, format);
	res = vfprintf(stream, format, argp);
	va_end(argp);

	if (res < 0) {
		fprintf(stderr, "fprintfエラー: %s\n", strerror(errno));
		exit(162);
	}

	return res;
}

int xasprintf(char **strp, const char *format, ...)
{
	int res;

	va_list argp;
	va_start(argp, format);
	res = vasprintf(strp, format, argp);
	va_end(argp);

	if (res < 0) {
		xfprintf(stderr, "asprintfエラー\n");
		exit(46);
	}

	return res;
}
