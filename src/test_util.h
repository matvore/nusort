#include <stdio.h>
#include <stdlib.h>

/*
 * テストの標準ストリームの使い方
 *
 * stdout: テストがエラーを報告するために使います。エラー以外は書き込みません。
 * stderr: ステータスや進捗のメッセージ。
 * stdin: 使いません。
 *
 * streams.h のストリームの使い方
 * アプリケーション (SUT) がこの３つのストリームを使うので、test_util モジュール
 * とテストコードがこのストリームをいじることで、SUT の動作を観察できます。SUTが
 * stdoutとstdinを基本的には使わないし、stderrに環境に関するエラー (テストされよ
 * うがないエラー) 以外はなにも書きません。
 *
 * テストは標準出力に何も書かなく、終了コードがゼロの場合は、パスとなります。
 * 標準出力が空ではない時と、終了コードがゼロではない時は、失敗です。
 */

#define CONFIG_TESTS_IGNORE_NULL_BYTES 1
#define CONFIG_TESTS_STDIN_FROM_FILE 2
void config_tests(int flags_);
void set_test_source_file(char const *fn);

void start_test(const char *name);

/*
 * end_test* 関数
 *
 * テストの出力を expected出力と比較して、違っていれば詳細を標準出力に書きます。
 * 出力が一致していれば、なにもしない。
 */
void end_test(const char *expected);
void end_test_expected_content_in_file(void);

FILE *open_tmp_file_containing(char const *);

/* exit_code が 0 ではない場合, エラーメッセージを out に出力します */
void expect_ok(int exit_code);

/* exit_code が 0 の場合, エラーメッセージを out に出力します */
void expect_fail(int exit_code);
