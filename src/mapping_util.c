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
	int error = 0;

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
				error = 1;
		}
	}

	return error;
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

int incomplete_code_is_prefix(
	struct key_mapping_array const *mapping, char const *incomplete_code)
{
	size_t so_far_len = strlen(incomplete_code);
	Orig extended_input = {0};

	strcpy(extended_input, incomplete_code);

	for (size_t try_i = so_far_len; try_i < sizeof(Orig) - 1; try_i++) {
		ssize_t extended_i;
		extended_input[try_i] = 1;

		BSEARCH_INDEX(extended_i, mapping->cnt,,
			      code_cmp(mapping->el[extended_i].orig,
				       extended_input));

		if (extended_i >= 0)
			DIE(0, "一致するコードはあるはずありません：%s",
			    incomplete_code);
		extended_i = ~extended_i;

		if (extended_i >= mapping->cnt)
			continue;

		if (strncmp(mapping->el[extended_i].orig, incomplete_code,
			    so_far_len))
			continue;

		return 1;
	}

	return 0;
}

