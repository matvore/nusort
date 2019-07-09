#include "commands.h"
#include "test_util.h"
#include "util.h"

#include <string.h>

static void verify_contents(const char *file, const char *contents)
{
	size_t contents_i = 0;
	FILE *actual_stream = xfopen(file, "r");
	char buffer[512];

	while (contents[contents_i]) {
		size_t read_len;

		if (!xfgets(buffer, sizeof(buffer), actual_stream))
			goto failure;

		read_len = strlen(buffer);

		if (strncmp(buffer, contents + contents_i, read_len))
			goto failure;
		contents_i += read_len;
	}

	if (xfgets(buffer, sizeof(buffer), actual_stream))
		/* contentsがfileの内容より短い */
		goto failure;
	xfclose(actual_stream);

	return;
failure:
	fprintf(stderr, "actual: %s\nexpected:\n%s\n", file, contents);
	exit(1);
}

static char actual_fn[512];

void start_test(const char *file, const char *name)
{
	int printed = snprintf(
		actual_fn, sizeof(actual_fn), "%s.%s.testout", file, name);
	printf("テスト：(%s) %s\n", file, name);
	if (sizeof(actual_fn) <= printed) {
		fprintf(stderr, "バファが短すぎます\n");
		exit(1);
	}

	out = err = xfopen(actual_fn, "w");
}

void end_test(const char *expected)
{
	xfclose(out);
	out = stdout;
	err = stderr;
	verify_contents(actual_fn, expected);
}
