#include <string.h>

#include "mapping_util.h"
#include "streams.h"
#include "util.h"

int code_cmp(char const *a, char const *b)
{
	size_t a_len = strlen(a);
	size_t b_len = strlen(b);

	if (a_len != b_len)
		return a_len < b_len ? -1 : 1;

	return strcmp(a, b);
}

static int conflicts(struct key_mapping const *a, struct key_mapping const *b)
{
	size_t a_len = strlen(a->orig);
	size_t b_len = strlen(b->orig);

	if (strncmp(a->orig, b->orig, a_len < b_len ? a_len : b_len))
		return 0;

	fputs("コード衝突: ", err);
	print_mapping(a, err);
	fputs(" と ", err);
	print_mapping(b, err);
	fputc('\n', err);

	return 1;
}

int sort_and_validate_no_conflicts(struct key_mapping_array *arr)
{
	int i;
	int ok = 1;

	QSORT(, arr->el, arr->cnt,
	      code_cmp(arr->el[a].orig, arr->el[b].orig) < 0);

	/*
	 * code_cmp は長さの異なるコードを一緒に並べ替えないので、隣接のコードを
	 * チェックするだけで不充分です。O(n^2)で全てのコードを他の全コードと対
	 * にして比較します。
	 */
	for (i = 0; i < arr->cnt; i++) {
		int j;
		for (j = i + 1; j < arr->cnt; j++) {
			if (i != j && conflicts(arr->el + i, arr->el + j))
				ok = 0;
		}
	}

	return ok;
}

void append_mapping(
	struct key_mapping_array *a, char const *orig, char const *conv)
{
	GROW_ARRAY_BY(*a, 1);
	strcpy(a->el[a->cnt - 1].orig, orig);
	strcpy(a->el[a->cnt - 1].conv, conv);
}

void print_mapping(struct key_mapping const *m, FILE *stream)
{
	fprintf(stream, "%s->%s", m->orig, m->conv);
}
