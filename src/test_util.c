#include "commands.h"
#include "test_util.h"
#include "util.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

static char *actual_fn;
static const char *test_source_file;
static const char *test_name;

struct io_buffer {
	FILE *stream;
	uint8_t buf[512];
	size_t pos;
	size_t end;
};

static int io_buffer_has_more(struct io_buffer *b)
{
	if (b->pos < b->end)
		return 1;

	b->end = xfread(b->buf, 1, sizeof(b->buf), b->stream);

	b->pos = 0;
	memset(b->buf + b->end, 0, sizeof(b->buf) - b->end);

	return b->end != 0;
}

static uint8_t io_buffer_read(struct io_buffer *b) { return b->buf[b->pos++]; }

static void verify_contents(const char *expected_fn)
{
	struct io_buffer exp = {xfopen(expected_fn, "r")};
	struct io_buffer act = {xfopen(actual_fn, "r")};

	while (io_buffer_has_more(&exp) && io_buffer_has_more(&act)) {
		if (io_buffer_read(&exp) != io_buffer_read(&act))
			goto failure;
	}

	if (io_buffer_has_more(&exp) || io_buffer_has_more(&act))
		/* ファイルのサイズが違います */
		goto failure;

	xfclose(exp.stream);
	xfclose(act.stream);

	return;

failure:
	xfprintf(stderr, "出力が違います。詳細はこれを実行してください：\n"
			 "	diff %s %s\n",
		 expected_fn, actual_fn);
	exit(99);
}

void start_test(const char *source_file, const char *name)
{
	if (actual_fn != NULL)
		BUG("既にテストが実行中です。");
	xasprintf(&actual_fn, "%s.%s.testout", source_file, name);
	xfprintf(stderr, "テスト：(%s) %s\n", source_file, name);
	out = err = xfopen(actual_fn, "w");

	test_source_file = source_file;
	test_name = name;
}

static void end_test_common(void)
{
	if (actual_fn == NULL)
		BUG("実行中のテストはありません。");
	xfclose(out);
	out = stdout;
	err = stderr;
	test_source_file = NULL;
	test_name = NULL;
}

void end_test(const char *expected)
{
	char expected_fn[] = "/tmp/expected-XXXXXX";
	int expected_fd;

	end_test_common();

	expected_fd = mkstemp(expected_fn);
	if (expected_fd == -1) {
		xfprintf(stderr, "mkstemp failed: %s\n", strerror(errno));
		exit(4);
	}
	if (dprintf(expected_fd, "%s", expected) < 0) {
		xfprintf(stderr, "dprintf failed: %s\n", strerror(errno));
		exit(184);
	}
	if (close(expected_fd) == -1) {
		xfprintf(stderr, "close failed: %s\n", strerror(errno));
		exit(197);
	}
	verify_contents(expected_fn);
	FREE(actual_fn);
}

void end_test_expected_content_in_file(void)
{
	char *expected_fn;
	xasprintf(&expected_fn,
		  "expected_test_out/%s.%s", test_source_file, test_name);
	end_test_common();
	verify_contents(expected_fn);
	FREE(expected_fn);
	FREE(actual_fn);
}
