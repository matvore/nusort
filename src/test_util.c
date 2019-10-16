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

static void verify_contents(const char *expected_fn, int can_fix_with_cp)
{
	FILE *act = xfopen(actual_fn, "r");
	FILE *exp = fopen(expected_fn, "r");
	int exp_opened = !!exp;

	if (!exp_opened) {
		report_fopen_failure(expected_fn);
		goto failure;
	}

	while (1) {
		int expc = xfgetc(exp);
		int actc = xfgetc(act);

		if (expc != actc)
			goto failure;

		if (expc == EOF)
			break;
	}

	xfclose(exp);
	xfclose(act);

	return;

failure:
	if (exp_opened)
		xfprintf(stderr, "出力が違います。"
				 "詳細はこれを実行してください：\n"
				 "	diff %s %s\n",
			 expected_fn, actual_fn);
	if (can_fix_with_cp)
		xfprintf(stderr, "出力ファイルを更新するには、"
				 "これを実行してください：\n"
				 "	cp %s %s\n",
			 actual_fn, expected_fn);
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

	end_test_common();
	store_in_tmp_file(expected, expected_fn);
	verify_contents(expected_fn, 0);
	FREE(actual_fn);
}

void end_test_expected_content_in_file(void)
{
	char *expected_fn;
	xasprintf(&expected_fn,
		  "expected_test_out/%s.%s", test_source_file, test_name);
	end_test_common();
	verify_contents(expected_fn, 1);
	FREE(expected_fn);
	FREE(actual_fn);
}

void store_in_tmp_file(char const *str, char *tmp_file_template)
{
	int fd = mkstemp(tmp_file_template);
	size_t str_size = strlen(str);
	ssize_t written;

	if (fd == -1)
		DIE(1, "mkstemp");

	written = write(fd, str, str_size);
	if (written == -1)
		DIE(1, "一時ファイルの %s に書き込む", tmp_file_template);

	if (close(fd) == -1)
		DIE(1, "一時ファイルの %s を閉じる", tmp_file_template);
}
