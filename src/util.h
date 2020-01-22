#include <stddef.h>
#include <stdio.h>
#include <stdnoreturn.h>
#include <stdlib.h>
#include <string.h>

void *xcalloc(size_t count, size_t size);
void *xreallocarray(void *ptr, size_t count, size_t el_size);

void report_fopen_failure(char const *pathname);
FILE *xfopen(const char *pathname, const char *mode);
FILE *xfdopen(int fd, char const *mode);

__attribute__((format (printf, 2, 3)))
int xasprintf(char **strp, const char *format, ...);

__attribute__((format (printf, 4, 5)))
void _Noreturn die(
	int show_errno, char const *file, long line, char const *format, ...);

#define DIE(show_err, ...) die(show_err, __FILE__, __LINE__, __VA_ARGS__);

void xfclose_impl(FILE *);
#define XFCLOSE(s) do { \
	xfclose_impl(s); \
	(s) = NULL; \
} while (0)

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
				/* \
				 * = 演算子の代わりにmemcpyを使えば el の要素 \
				 * それぞれが固定配でも el をも並べ替えられま \
				 * す。 \
				*/ \
				memcpy(&p##b[el], &p##a[el], sizeof(*(el))); \
				p##b++; \
				memcpy(&p##a[el], &p##b[el], sizeof(*(el))); \
				memcpy(&p##b[el], pivot, sizeof(*(el))); \
			} \
		} \
		qsort_push_frame(&f, begin, p##b); \
		qsort_push_frame(&f, p##b + 1, p##a); \
	} \
	free(f.f); \
} while(0)

#define BSEARCH_INDEX(index, cnt, cmp_prelude, cmp) do { \
	size_t min = 0, max = (cnt); \
	int cmp_res; \
	while (min != max) { \
		(index) = (max + min) / 2; \
		cmp_prelude; \
		cmp_res = (cmp); \
		if (!cmp_res) \
			break; \
		if (cmp_res < 0) \
			min = (index) + 1; \
		else \
			max = (index); \
	} \
	if (cmp_res > 0) \
		(index) = ~(index); \
	else if (cmp_res < 0) \
		(index) = ~((index) + 1); \
} while (0)

#define BSEARCH(e, el, cnt, cmp) do { \
	ssize_t index; \
	BSEARCH_INDEX(index, cnt, (e) = (el) + (index), cmp); \
	if (index < 0) \
		(e) = 0; \
} while (0)

/*
 * ごく単純なハッシュマップ
 *
 * キーがバイト列として扱います。２個のキーのバイトが一致していればキーが同一
 * として見なします。キーがポインターであればまたはポインターを格納するstruct
 * の場合でもそのポインターの差すメモリーがfreeされません。キーの全バイトが０
 * の時はバケツが空と見なします。このハッシュマップは固定サイズの文字列に向い
 * ています。
 *
 * ハッシュマップを初期化後にバケツ数を変更できません。
 *
 * ハッシュマップを格納するstructは下記の様に宣言されます：
 *
 * struct hashmap {
 * 	KEY_TYPE *keys;
 * 	VAL_TYPE *values;
 * 	size_t bucket_cnt;
 * };
 */

/* ハッシュマップを初期化して、バケツを bc 個作ります。 */
#define INIT_HASHMAP(hashmap, bc) do { \
	if ((hashmap).keys || (hashmap).values || (hashmap).bucket_cnt) \
		DIE(0, "hashmapのフィールドが０に設定されていない"); \
	(hashmap).bucket_cnt = (bc); \
	(hashmap).keys = \
		xcalloc((hashmap).bucket_cnt, sizeof((hashmap).keys)); \
	(hashmap).values = \
		xcalloc((hashmap).bucket_cnt, sizeof((hashmap).values)); \
} while (0)

/*
 * キーがマップに入っていれば、そのバケツを見つけます。
 * マップに入っていなければ、そのキーの書き込めるバケツを見つけます。
 */
#define FIND_HASHMAP_ENTRY(hashmap, key, found_key, found_value) do { \
	size_t index = find_hashmap_entry_impl( \
		(hashmap).keys, sizeof (hashmap).keys[0], \
		(hashmap).bucket_cnt, &(key)); \
	(found_key) = (hashmap).keys + index; \
	(found_value) = (hashmap).values + index; \
} while (0)

#define DESTROY_HASHMAP(hashmap) do { \
	FREE((hashmap).keys); \
	FREE((hashmap).values); \
	(hashmap).bucket_cnt = 0; \
} while (0)

int bytes_are_zero(void const *, size_t);

size_t find_hashmap_entry_impl(
	void const *keys_, size_t key_size,
	size_t bucket_cnt, void const *target_key_);
