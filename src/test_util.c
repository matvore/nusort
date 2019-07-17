#include "commands.h"
#include "test_util.h"
#include "util.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

static char *actual_fn;

static void verify_contents(const char *contents)
{
	size_t contents_i = 0;
	FILE *actual_stream = xfopen(actual_fn, "r");
	char buffer[512];
	char expected_fn[] = "/tmp/expected-XXXXXX";
	int expected_fd;

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
	expected_fd = mkstemp(expected_fn);
	if (expected_fd == -1) {
		xfprintf(stderr, "mkstemp failed: %s\n", strerror(errno));
		exit(4);
	}
	if (dprintf(expected_fd, "%s", contents) < 0) {
		xfprintf(stderr, "dprintf failed: %s\n", strerror(errno));
		exit(184);
	}
	if (close(expected_fd) == -1) {
		xfprintf(stderr, "close failed: %s\n", strerror(errno));
		exit(197);
	}
	xfprintf(stderr, "出力が違います。詳細はこれを実行してください：\n"
			 "	diff %s %s\n",
		 expected_fn, actual_fn);
	exit(99);
}

void start_test(const char *file, const char *name)
{
	if (actual_fn != NULL)
		BUG("既にテストが実行中です。");
	xasprintf(&actual_fn, "%s.%s.testout", file, name);
	printf("テスト：(%s) %s\n", file, name);
	out = err = xfopen(actual_fn, "w");
}

void end_test(const char *expected)
{
	if (actual_fn == NULL)
		BUG("実行中のテストはありません。");
	xfclose(out);
	out = stdout;
	err = stderr;
	verify_contents(expected);
	free(actual_fn);
	actual_fn = NULL;
}
