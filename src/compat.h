#include <string.h>

struct qsort_frame {
	size_t begin;
	size_t end;
};

static inline void qsort_advance_pivot(
	char *el, size_t a, size_t *b, void *pivot_buff, size_t width)
{
	void *a_el = el + a * width;
	memcpy(el + *b * width, a_el, width);
	(*b)++;
	memcpy(a_el, el + *b * width, width);
	memcpy(el + *b * width, pivot_buff, width);
}

#include <stdio.h>
#define QSORT(prefix, el, cnt, lt) \
	do { \
		struct qsort_frame f[10000] = {{0, cnt}}; \
		size_t f_last = 0; \
		char pivot_buff[sizeof(*(el))]; \
		while (1) { \
			struct qsort_frame *cf = f + f_last; \
			size_t prefix##a, prefix##b = cf->begin; \
			if (cf->end <= prefix##b) { \
				if (!f_last) \
					break; \
				f_last--; \
				continue; \
			} \
			memcpy(pivot_buff, el + prefix##b, sizeof(*(el))); \
			for (prefix##a = prefix##b + 1; prefix##a < cf->end; \
					prefix##a++) { \
				if (lt) \
					qsort_advance_pivot( \
						(char *)(el), \
						prefix##a, &prefix##b, \
						pivot_buff, sizeof(*(el))); \
			} \
			f[f_last + 1].begin = cf->begin; \
			f[f_last + 1].end = prefix##b; \
			f[f_last].begin = prefix##b + 1; \
			f_last++; \
		} \
	} while(0)
