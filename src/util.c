#define _GNU_SOURCE

#include "streams.h"
#include "util.h"

#include <errno.h>
#include <execinfo.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *xcalloc(size_t count, size_t size)
{
	void *res = calloc(count, size);
	if (!res)
		DIE(1, "calloc");
	return res;
}

void *xreallocarray(void *ptr, size_t count, size_t el_size)
{
	ptr = realloc(ptr, count * el_size);
	if (!ptr && count)
		DIE(1, "realloc");
	return ptr;
}

void report_fopen_failure(char const *pathname)
{
	perror("ファイルを開く際にfopenが失敗しました");
	fprintf(stderr, "ファイル名：%s\n", pathname);
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

FILE *xfdopen(int fd, char const *mode)
{
	FILE *f = fdopen(fd, mode);
	if (!f)
		DIE(1, "fdopen");
	return f;
}

void xfclose_impl(FILE *stream)
{
	if (!stream)
		return;
	if (ferror(stream))
		DIE(0, "エラービットがオン状態で fclose しようとしました");
	if (!fclose(stream))
		return;
	perror("警告: fclose が失敗しました");
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

void _Noreturn die(
	int show_errno,
	char const *file,
	long line,
	char const *format,
	...)
{
	char const *preamble = "致命的なエラー";
	va_list argp;
	void *trace_symbols[256];
	int trace_size;

	if (show_errno && errno)
		perror(preamble);
	else
		fprintf(stderr, "%s\n", preamble);

	fprintf(stderr, "\tat %s:%ld\n", file, line);

	va_start(argp, format);
	vfprintf(stderr, format, argp);
	va_end(argp);

	fputc('\n', stderr);

	trace_size =
		backtrace(trace_symbols, sizeof(trace_symbols) / sizeof(void*));
	backtrace_symbols_fd(trace_symbols + 1, trace_size - 1, STDERR_FILENO);

	exit(228);
}

int bytes_are_zero(void const *buf_, size_t s)
{
	char const *buf = buf_;
	while (s) {
		if (*buf)
			return 0;
		buf++;
		s--;
	}
	return 1;
}

size_t find_hashmap_entry_impl(
	void const *keys_, size_t key_size,
	size_t bucket_cnt, void const *target_key_)
{
	char const *target_key = target_key_;
	size_t key_hash = 0;
	size_t i;
	char const *keys = keys_;

	for (i = 0; i < key_size; i++) {
		key_hash *= 31;
		key_hash += target_key[i];
	}
	key_hash %= bucket_cnt;

	i = key_hash;
	while (1) {
		char const *bucket_ptr = keys + (key_size * i);
		if (bytes_are_zero(bucket_ptr, key_size))
			return i;
		if (!memcmp(bucket_ptr, target_key, key_size))
			return i;

		i = (i + 1) % bucket_cnt;
		if (i == key_hash)
			DIE(0, "hash map is already full");
	}
}

void print_base64_digit(int d)
{
	if (d < 26) {
		fputc('A' + d, out);
		return;
	}
	d -= 26;
	if (d < 26) {
		fputc('a' + d, out);
		return;
	}
	d -= 26;
	if (d < 10) {
		fputc('0' + d, out);
		return;
	}
	d -= 10;
	fputc(d ? '/' : '+', out);
}
