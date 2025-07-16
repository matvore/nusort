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

/*
 * expected_content とは、合格と見なすテスト出力
 *
 * expected_content がヌルの場合、ファイルから合格の出力を読みます。
 *
 * 使い方:
 *
 * while (run_test("simple_test", "4")) {
 * 	fprintf(out, "%d", 2 + 2);
 * }
 *
 * 一回目の呼び出しはテスト情報を保存し、true を返す
 * 二回目は、テストの合格か不合格を判定し、false を返す
 *
 * 判定モードはテストの出力を expected_content と比較して、違っていれば詳細を標
 * 準出力に書きます。
 * 出力が一致していれば、なにもしない。
 */
int run_test(char const *name, char const *expected_content);

FILE *open_tmp_file_containing(char const *);

/* exit_code が 0 ではない場合, エラーメッセージを out に出力します */
void expect_ok(int exit_code);

/* exit_code が 0 の場合, エラーメッセージを out に出力します */
void expect_fail(int exit_code);
