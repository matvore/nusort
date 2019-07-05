#include "commands.h"
#include "util.h"

#include <stdlib.h>

static void failure(const char *details)
{
	fprintf(stderr, "%s\n", details);
	exit(1);
}

static void verify_contents(const char *file, const char *contents)
{
	size_t contents_len = strlen(contents);
	FILE *actual_stream = xfopen(file, "r");
	while (contents_len) {
		char buffer[512];
		size_t read_len;

		if (!xfgets(buffer, sizeof(buffer), actual_stream))
			failure(file);

		read_len = strlen(buffer);

		if (strncmp(buffer, contents, read_len))
			failure(file);
		contents_len -= read_len;
		contents += read_len;
	}
	xfclose(actual_stream);
}

static void not_enough_args_test(void)
{
	const char *actual_fn = "last_rank_contained.not_enough_args.testout";
	FILE *actual = xfopen(actual_fn, "w");
	const char *argv[] = {"一"};
	out = err = actual;
	fprintf(actual, "exit: %d\n", print_last_rank_contained(argv, 1));
	xfclose(actual);
	out = stdout;
	err = stderr;
	verify_contents(
		actual_fn,
		"32個の区切り漢字を必するけれど、1個が渡された。\n"
		"exit: 1\n");
}

int main(void)
{
	not_enough_args_test();
	return 0;
}
