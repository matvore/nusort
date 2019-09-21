#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xcalloc(size_t count, size_t size);
void *xreallocarray(void *ptr, size_t count, size_t el_size);
FILE *xfopen(const char *pathname, const char *mode);
char *xfgets(char *s, int size, FILE *stream);
void xfclose(FILE *stream);

__attribute__((format (printf, 2, 3)))
int xfprintf(FILE *stream, const char *format, ...);

__attribute__((format (printf, 2, 3)))
int xasprintf(char **strp, const char *format, ...);

int xfputc(int c, FILE *stream);
size_t xfread(void *ptr, size_t size, size_t nmemb, FILE *stream);

/*
 * GROW_ARRAY_BY
 *
 * Create a struct that looks like this:
 *
 * struct array {
 * 	TYPE *el;
 * 	size_t cnt;
 * 	size_t alloc;
 * };
 *
 * Where TYPE is any type you want. The number of elements will be kept in 'cnt'
 * and the capacity in 'alloc'.
 */
#define GROW_ARRAY_BY(array, grow_cnt) do { \
	(array).cnt += grow_cnt; \
	if ((array).alloc < (array).cnt) { \
		size_t old_alloc = (array).alloc; \
		(array).alloc *= 2; \
		if ((array).alloc < (array).cnt) \
			(array).alloc = (array).cnt; \
		(array).el = xreallocarray((array).el, (array).alloc, \
			sizeof(*(array).el)); \
		memset((array).el + old_alloc, 0, \
			((array).alloc - old_alloc) * sizeof(*(array).el)); \
	} \
} while (0)

#define DESTROY_ARRAY(array) do { \
	free((array).el); \
	(array).el = 0; \
	(array).cnt = 0; \
	(array).alloc = 0; \
} while (0)

#define FREE(ptr) do { \
	free(ptr); \
	(ptr) = NULL; \
} while (0)

struct qsort_frames {
	struct {
		size_t begin;
		size_t end;
	} *f, *end;
};

static inline void qsort_push_frame(
	struct qsort_frames *f, size_t begin, size_t end)
{
	if (end >= begin + 2) {
		f->end->begin = begin;
		f->end->end = end;
		f->end++;
	}
}

#define BUG(...) \
do { \
	xfprintf(stderr, "BUG at (%s:%d)\n", __FILE__, __LINE__); \
	xfprintf(stderr, __VA_ARGS__); \
	xfputc('\n', stderr); \
	exit(224); \
} while(0)

#define QSORT(p, el, cnt, lt) \
do { \
	struct qsort_frames f; \
	char pivot[sizeof(*(el))]; \
	f.f = f.end = xcalloc(cnt / 2, sizeof(*f.f)); \
	qsort_push_frame(&f, 0, cnt); \
	while (f.end != f.f) { \
		size_t begin, p##a, p##b; \
		f.end--; \
		p##b = begin = f.end->begin; \
		memcpy(pivot, el + begin, sizeof(*(el))); \
		for (p##a = begin + 1; p##a < f.end->end; p##a++) { \
			if (lt) { \
				(el)[p##b] = (el)[p##a]; \
				p##b++; \
				(el)[p##a] = (el)[p##b]; \
				memcpy(&p##b[el], pivot, sizeof(*(el))); \
			} \
		} \
		qsort_push_frame(&f, begin, p##b); \
		qsort_push_frame(&f, p##b + 1, p##a); \
	} \
	free(f.f); \
} while(0)

#define BSEARCH(e, el, cnt, cmp) do { \
	size_t min = 0, max = (cnt); \
	while (min != max) { \
		size_t mid = (max + min) / 2; \
		int cmp_res; \
		e = (el) + mid; \
		cmp_res  = (cmp); \
		if (!cmp_res) \
			break; \
		e = NULL; \
		if (cmp_res < 0) \
			min = mid + 1; \
		else \
			max = mid; \
	} \
} while (0)
