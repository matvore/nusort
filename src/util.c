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
	if (!res)
		DIE(errno, "calloc");
	return res;
}

void *xreallocarray(void *ptr, size_t count, size_t el_size)
{
	ptr = realloc(ptr, count * el_size);
	if (!ptr && count)
		DIE(errno, "realloc");
	return ptr;
}

void report_fopen_failure(char const *pathname)
{
	fprintf(stderr,
		"%s を開く際にfopenが失配しました：\n\t%s\n",
		pathname, strerror(errno));
}

FILE *xfopen(const char *pathname, const char *mode)
{
	FILE *f = fopen(pathname, mode);
	if (!f) {
		report_fopen_failure(pathname);
		exit(91);
	}
	return f;
}

char *xfgets(char *s, int size, FILE *stream)
{
	s = fgets(s, size, stream);
	if (!s && errno)
		DIE(errno, "fgets");
	return s;
}

void xfputs(char const *s, FILE *stream)
{
	int res = fputs(s, stream);
	if (res == EOF)
		DIE(errno, "fputs");
	if (res < 0)
		BUG("fputsから規定に反する戻り値");
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

	if (res < 0)
		DIE(errno, "fprintf");

	return res;
}

int xasprintf(char **strp, const char *format, ...)
{
	int res;

	va_list argp;
	va_start(argp, format);
	res = vasprintf(strp, format, argp);
	va_end(argp);

	if (res < 0)
		DIE(0, "asprintf");

	return res;
}

int xfputc(int c, FILE *stream)
{
	c = fputc(c, stream);
	if (c == EOF)
		DIE(errno, "fputc");
	return c;
}

size_t xfread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t read_size = fread(ptr, size, nmemb, stream);
	if (ferror(stream))
		DIE(errno, "fread");
	return read_size;
}
