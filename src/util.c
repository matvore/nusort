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
	xfprintf(stderr, "ファイル名：%s\n", pathname);
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

int xfgetc(FILE *stream)
{
	int c;
	if (ferror(stream) != 0)
		BUG("fgetc の前にファイルエラーが未処理です");
	c = fgetc(stream);
	if (ferror(stream) != 0)
		DIE(0, "fgetc");
	return c;
}

char *xfgets(char *s, int size, FILE *stream)
{
	s = fgets(s, size, stream);
	if (!s && errno)
		DIE(1, "fgets");
	return s;
}

void xfputs(char const *s, FILE *stream)
{
	int res = fputs(s, stream);
	if (res == EOF)
		DIE(1, "fputs");
	if (res < 0)
		BUG("fputsから規定に反する戻り値");
}

void xfwrite(void const *buf, size_t size, FILE *stream)
{
	errno = 0;
	if (!fwrite(buf, size, 1, stream))
		DIE(1, "fwrite");
}

void xfclose_impl(FILE *stream)
{
	if (!stream || !fclose(stream))
		return;
	perror("警告: fclose が失敗しました");
}

int xfprintf(FILE *stream, const char *format, ...)
{
	int res;

	va_list argp;
	va_start(argp, format);
	res = vfprintf(stream, format, argp);
	va_end(argp);

	if (res < 0)
		DIE(1, "fprintf");

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
		DIE(1, "fputc");
	return c;
}

size_t xfread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t read_size = fread(ptr, size, nmemb, stream);
	if (ferror(stream))
		DIE(1, "fread");
	return read_size;
}

/*
 * xfprintf, xfputc などが DIE を呼び出すことができるため、DIE の実装では使えま
 * せん。
 */
void _Noreturn die(
	int show_errno,
	char const *file,
	long line,
	char const *format,
	...)
{
	char const *preamble = "致命的なエラー";
	va_list argp;

	if (show_errno && errno)
		perror(preamble);
	else
		fprintf(stderr, "%s\n", preamble);

	fprintf(stderr, "\tat %s:%ld\n", file, line);

	va_start(argp, format);
	vfprintf(stderr, format, argp);
	va_end(argp);

	fputc('\n', stderr);

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
			BUG("hash map is already full");
	}
}
