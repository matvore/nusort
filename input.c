#ifndef _MSC_VER
#define HAVE_TERMIOS 1
#else
#undef HAVE_TERMIOS
#endif

#include <errno.h>
#if HAVE_TERMIOS
#include <termios.h>
#include <unistd.h>
#endif

#include "commands.h"
#include "input_impl.h"
#include "mapping.h"
#include "streams.h"
#include "util.h"
#include "windows.h"

static int hide_term_warn;

#if HAVE_TERMIOS
static void check_term_op(int res)
{
	if (res == 0) return;
	if (res != -1) DIE(0, "規定に反する戻り値");
	if (!hide_term_warn)
		perror("ターミナルの属性を設定する際にエラーが発生しました");
}

static void customize_term_attributes(struct termios t)
{
	t.c_lflag &= ~(ICANON | ECHO);
	check_term_op(tcsetattr(STDIN_FILENO, TCSANOW, &t));
}
#endif

int input(struct flagset *fs, char **argv, int argc)
{
	struct mapping mapping = {0};
	int res;

#if HAVE_TERMIOS
	struct termios orig_termios;
#endif
	flagcat(fs, "input");
	addflag(fs, "--no-show-pending-and-converted", 'b', 0,
"\n\t"	"未変換と変換済みの入力を表示しない"
	);
	addflag(fs, "--no-show-keyboard", 'b', 0,
"\n\t"	"キーボードを表示しない"
	);
	addflag(fs, "--no-show-cutoff-guide", 'b', 0,
"\n\t"	"区切りガイドを表示しない"
	);
	addflag(fs, "--no-show-rsc-list", 'b', 0,
"\n\t"	"部首画数リストを表示しない"
	);
	addflag(fs, "--no-real-tty", 'b', 0,
"\n\t"	"標準出力を実際のttyとして扱わない。端末の機能をいくつか無効にする。"
	);

	addflag(fs, "--no-save-with-osc52", 'b', 0,
"\n\t"	"確定する〔改行を打つ〕際はosc-52でコピーをしない");
	addflag(fs, "--rpc-mode", 'b', 0,
"\n\t"	"標準入力・出力の使い方を変えます。"
"\n\t"	"プロセスが開始するとパケットが標準出力で送られます。"
"\n\t"
"\n\t"	"パケットの種類 :"
"\n\t"	"\\x01"
"\n\t"	"	ユーザの入力待ちです。標準入力に 1 バイトを書き込んでください。"
"\n\t"	"	Esc (0x1b) で応答するとプロセスを終了します。"
"\n\t"	"\\x02 + <後のデータサイズを指定する 1 バイト> + <データ>"
"\n\t"	"	変換済みデータです。\\b または \\n が Backspace と リターン の"
"\n\t"	"	代わりに含まれている可能性があります。"
"\n\t"	"\\x04 + <後のデータサイズを指定する 1 バイト> + <データ>"
"\n\t"	"	ユーザに表示できる utf-8 テキスト"
	);

	romazi_flags(fs);
	mapping_flags(fs);

	if (argc < 0) return 0;

	parsflag(fs, &argc, argv);
	if (argc) badflag(argv[0]);

	if (flagval(fs, "--rpc-mode")) hide_term_warn = 1;

	get_romazi_codes(fs, &mapping.arr);

	append_mapping(&mapping.arr, "\"", "々");
	append_mapping(&mapping.arr, ".  ", "。");
	append_mapping(&mapping.arr, ",  ", "、");
	append_mapping(&mapping.arr, "/  ", "・");
	append_mapping(&mapping.arr, "\\  ", "￥");

	res = mapping_populate(fs, &mapping);
	if (res) goto cleanup;

	if (!flagval(fs, "--no-real-tty")) {
#if HAVE_TERMIOS
		check_term_op(tcgetattr(STDIN_FILENO, &orig_termios));
		customize_term_attributes(orig_termios);
#endif
		enable_windows();
		fputs("\x1b[?1049h" "\x1b[?25l", err);
	}
	res = input_impl(&mapping, fs);
	if (!flagval(fs, "--no-real-tty")) {
#if HAVE_TERMIOS
		check_term_op(tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios));
#endif
		fputs("\x1b[?25h" "\x1b[?1049l", err);
	}

cleanup:
	destroy_mapping(&mapping);

	return res;
}
