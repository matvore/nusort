#include <stddef.h>
#include <stdio.h>
#include <string.h>

void *xcalloc(size_t count, size_t size);
void *xreallocarray(void *ptr, size_t count, size_t size);
FILE *xfopen(const char *pathname, const char *mode);
char *xfgets(char *s, int size, FILE *stream);
void xfclose(FILE *stream);
int xfprintf(FILE *stream, const char *format, ...);

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
