#include "mapping.h"
#include "mapping_util.h"

struct input_flags {
	unsigned show_keyboard : 1;
	unsigned show_pending_and_converted : 1;
	unsigned show_cutoff_guide : 1;
	unsigned save_with_osc52 : 1;
	unsigned show_rsc_list : 1;

	/* 標準入力・出力の使い方を変えます。
	 * プロセスが開始するとパケットが標準出力で送られます。
	 *
	 * パケットの種類 :
	 * \x01
	 *	ユーザの入力待ちです。標準入力に 1 バイトを書き込んでください。
	 *	Esc (0x1b) で応答するとプロセスを終了します。
	 * \x02 + <後のデータサイズを指定する 1 バイト> + <データ>
	 *	変換済みデータです。\b または \n が Backspace と リターン の
	 *	代わりに含まれている可能性があります。
	 * \x04 + <後のデータサイズを指定する 1 バイト> + <データ>
	 *	ユーザに表示できる utf-8 テキスト
	 */
	unsigned rpc_mode : 1;
};

int input_impl(struct mapping *, struct input_flags const *);
