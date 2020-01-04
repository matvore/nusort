#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include "commands.h"
#include "input_impl.h"
#include "mapping.h"
#include "streams.h"
#include "util.h"

static void check_term_op(int res)
{
	if (res == 0)
		return;

	if (res == -1)
		DIE(1, "ターミナルの属性を設定する際にエラーが発生しました。");

	BUG("規定に反する戻り値");
}

static void customize_term_attributes(struct termios t)
{
	t.c_lflag &= ~(ICANON | ECHO);
	check_term_op(tcsetattr(STDIN_FILENO, TCSANOW, &t));
}

int input(char const *const *argv, int argc, int set_raw_mode)
{
	struct mapping mapping = {0};
	struct termios orig_termios;
	struct romazi_config romazi_config = {0};
	int res;

	init_romazi_config_for_cli_flags(&romazi_config);

	while (argc > 0 && argv[0][0] == '-') {
		if (!parse_romazi_flags(&argc, &argv, &romazi_config)) {
			xfprintf(err,
				 "フラグを認識できませんでした：%s\n", argv[0]);
			return 3;
		}
	}

	init_romazi(&romazi_config);

	mapping.ergonomic_sort = 1;
	mapping.include_kanji = 1;
	if (!mapping_populate(&mapping))
		return 250;

	if (set_raw_mode) {
		check_term_op(tcgetattr(STDIN_FILENO, &orig_termios));
		customize_term_attributes(orig_termios);
	}
	res = input_impl(&mapping);
	if (set_raw_mode)
		check_term_op(tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios));

	mapping_destroy(&mapping);

	return res;
}
