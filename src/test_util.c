#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>

#ifdef _MSC_VER

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <fcntl.h>
#include <io.h>
#include <synchapi.h>

#else

#include <pthread.h>
#include <unistd.h>

#endif

static char *actual_fn;
static FILE *test_output_pipe_read;
/* Thread that takes test output and removes binary-looking bytes. */
static const char *test_source_file;
static const char *test_name;
static int flags;

/*
 * stderr 以外のストリームでエラーが発生した場合、テストの結果を信用できない
 * ので失敗扱いになります。
 *
 * 標準出力に１バイトだけを書いてしまうとテストバイナリ全体が失敗したとみなさ
 * れるので、stdout 以外のエラーを stdout に報告して大丈夫です。
 */
static int report_output_errors(
	char const *stream_name, FILE *s, FILE *report_to)
{
	fflush(s);
	if (!ferror(s))
		return 0;

	fprintf(report_to, "出力ストリームのエラービットがオンです: %s",
		stream_name);
	clearerr(s);

	return 1;
}

static void verify_contents(const char *expected_fn, int can_fix_with_cp)
{
	FILE *act = xfopen(actual_fn, "r");
	FILE *exp = fopen(expected_fn, "r");
	int failed = 0;
	int exp_opened = !!exp;

	if (!exp_opened) {
		report_fopen_failure(expected_fn);
		failed = 1;
	}

	while (!failed) {
		int expc = fgetc(exp);
		int actc = fgetc(act);

		if (expc != actc)
			failed = 1;

		if (expc == EOF)
			break;
	}

	XFCLOSE(exp);
	XFCLOSE(act);

	if (!failed)
		return;

	if (exp_opened)
		printf("出力が違います。"
		       "詳細はこれを実行してください：\n"
		       "	diff %s %s\n",
		       expected_fn, actual_fn);
	if (can_fix_with_cp)
		printf("出力ファイルを更新するには、"
		       "これを実行してください：\n"
		       "	cp %s %s\n",
		       actual_fn, expected_fn);
}

static void process_output(void)
{
	FILE *output = xfopen(actual_fn, "w");
	int b;

	while ((b = fgetc(test_output_pipe_read)) != EOF) {
		if (b || !(flags & CONFIG_TESTS_IGNORE_NULL_BYTES))
			fputc(b, output);
	}

	report_output_errors("実際テスト出力", output, stdout);
	XFCLOSE(output);
	XFCLOSE(test_output_pipe_read);
}

#ifdef _MSC_VER

static PTP_WORK opp_work;

void CALLBACK opp_cb(
	PTP_CALLBACK_INSTANCE inst, void *unused, PTP_WORK work)
{
	process_output();
}

static void start_output_processor(void)
{
	if (!(opp_work = CreateThreadpoolWork(opp_cb, NULL, NULL)))
		DIE(0, "CreateThreadpoolWork: %lu", GetLastError());
	SubmitThreadpoolWork(opp_work);
}

static void join_output_processor(void)
{
	WaitForThreadpoolWorkCallbacks(opp_work, FALSE);
}

static void make_pipe(int *ps)
{
	HANDLE read, write;

	if (!CreatePipe(&read, &write, NULL, 512))
		DIE(0, "CreatePipe: %lu", GetLastError());

	ps[0] = _open_osfhandle((intptr_t)read, _O_RDONLY);
	if (ps[0] == -1) DIE(1, "_open_osfhandle");
	ps[1] = _open_osfhandle((intptr_t)write, 0);
	if (ps[1] == -1) DIE(1, "_open_osfhandle");
}

#else

static pthread_t test_output_processor;

static void *opp_cb(void *unused)
{
	process_output();
	return NULL;
}

static void start_output_processor(void)
{
	if (pthread_create(&test_output_processor, NULL, opp_cb, NULL))
		DIE(1, "pthread_create");
}

static void join_output_processor(void)
{
	errno = pthread_join(test_output_processor, NULL);
	if (errno)
		DIE(1, "pthread_join");
}

static void make_pipe(int *ps)
{
	if (pipe(ps)) DIE(1, "pipe");
}

#endif

void config_tests(int flags_) { flags = flags_; }

void set_test_source_file(char const *fn)
{
	int ci, last_bs = -1;

	if (!fn || !strlen(fn))
		DIE(0, "test_source_file が無効です");
	if (test_source_file)
		DIE(0, "test_source_file が期に設定されています。");

	for (ci = 0; fn[ci]; ci++)
		if (fn[ci] == '\\') last_bs = ci;

	test_source_file = fn + last_bs + 1;
}

static void start_test(char const *name)
{
	int test_output_pipe[2];
	if (actual_fn != NULL)
		DIE(0, "既にテストが実行中です。");
	if (!test_source_file)
		DIE(0, "test_source_file が設定されていない。start_testを呼ぶ"
		    "前に設定してください。");
	xasprintf(&actual_fn, "actual_test_out/%s.%s.out",
		  test_source_file, name);
	fprintf(stderr, "テスト：(%s) %s\n", test_source_file, name);

	make_pipe(test_output_pipe);

	test_output_pipe_read = xfdopen(test_output_pipe[0], "r");
	out = err = xfdopen(test_output_pipe[1], "w");

	start_output_processor();

	if (flags & CONFIG_TESTS_STDIN_FROM_FILE) {
		char *input_fn;
		xasprintf(&input_fn, "test_input/%s.%s",
			  test_source_file, name);
		in = xfopen(input_fn, "r");
		FREE(input_fn);
	}

	test_name = name;
}

static void end_test_common(void)
{
	if (flags & CONFIG_TESTS_STDIN_FROM_FILE) {
		if (fgetc(in) != -1)
			fputs("入力が最後まで読み込まれていない", stdout);
		XFCLOSE(in);
	}

	if (actual_fn == NULL)
		DIE(0, "実行中のテストはありません。");
	report_output_errors("未処理実際テスト出力", out, stdout);
	XFCLOSE(out);
	out = err = NULL;
	join_output_processor();
	test_name = NULL;

	if (report_output_errors("標準出力", stdout, stderr))
		exit(204);
}

static char *store_in_tmp_file(
	char const *str, const char *tmp_file_pref);

static void end_test(const char *expected)
{
	char *expected_fn;

	end_test_common();
	expected_fn = store_in_tmp_file(expected, "expected-");
	verify_contents(expected_fn, 0);
	FREE(expected_fn);
	FREE(actual_fn);
}

static void end_test_expected_content_in_file(void)
{
	char *expected_fn;
	xasprintf(&expected_fn,
		  "expected_test_out/%s.%s", test_source_file, test_name);
	end_test_common();
	verify_contents(expected_fn, 1);
	FREE(expected_fn);
	FREE(actual_fn);
}

static char *store_in_tmp_file(char const *str, const char *tmp_file_pref)
{
	int fd;
	size_t str_size = strlen(str);
	long written;
	char *fn;
#ifdef _MSC_VER
	char fnbuf[L_tmpnam_s+1];
#endif

#ifdef _MSC_VER
	errno = tmpnam_s(fnbuf, sizeof(fnbuf));
	if (errno) DIE(1, "tmpnam_s");
	fn = strdup(fnbuf);
	if (!fn) DIE(0, "strdup: %s", fnbuf);
	fd = open(fn, O_CREAT | O_WRONLY);
	if (fd == -1) DIE(1, "open: %s", fn);
#else
	xasprintf(&fn, "/tmp/%sXXXXXX", tmp_file_pref);
	fd = mkstemp(fn);
	if (fd == -1) DIE(1, "mkstemp: %s", fn);
#endif

	written = write(fd, str, str_size);
	if (written == -1) DIE(1, "一時ファイルの %s に書き込む", fn);

	if (close(fd) == -1) DIE(1, "一時ファイルの %s を閉じる", fn);

	return fn;
}

int run_test(char const *name, char const *expected_content)
{
	if (!test_name) {
		start_test(name);
		return 1;
	}
	if (strcmp(test_name, name)) {
		DIE(0, "run_test の使い方が間違っています: %s != %s",
		    test_name, name);
	}

	if (expected_content)
		end_test(expected_content);
	else
		end_test_expected_content_in_file();

	return 0;
}

FILE *open_tmp_file_containing(char const *str)
{
	char *fn;
	FILE *f;

	fn = store_in_tmp_file(str, "nusort_test_tmp_");
	f = xfopen(fn, "r");
	FREE(fn);
	return f;
}

void expect_ok(int exit_code)
{
	if (exit_code)
		fprintf(out, "失敗: %d\n", exit_code);
}

void expect_fail(int exit_code)
{
	if (!exit_code)
		fprintf(out, "失敗しなかった: %d\n", exit_code);
}
