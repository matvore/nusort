#include "util.h"

#include <string.h>

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
					memcpy(&p##b[el], pivot, \
					       sizeof(*(el))); \
				} \
			} \
			qsort_push_frame(&f, begin, p##b); \
			qsort_push_frame(&f, p##b + 1, p##a); \
		} \
		free(f.f); \
	} while(0)
