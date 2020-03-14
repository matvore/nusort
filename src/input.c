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

	DIE(0, "規定に反する戻り値");
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
	init_mapping_config_for_cli_flags(&mapping);

	while (argc > 0 && argv[0][0] == '-') {
		if (parse_mapping_flags(&argc, &argv, &mapping))
			continue;
		if (parse_romazi_flags(&argc, &argv, &romazi_config))
			continue;

		fprintf(err,
			 "フラグを認識できませんでした：%s\n", argv[0]);
		return 3;
	}

	get_romazi_codes(&romazi_config, &mapping.arr);

	append_mapping(&mapping.arr, ".  ", "。");
	append_mapping(&mapping.arr, ",  ", "、");
	append_mapping(&mapping.arr, "/  ", "・");

	res = mapping_populate(&mapping);
	if (res)
		return res;

	if (set_raw_mode) {
		check_term_op(tcgetattr(STDIN_FILENO, &orig_termios));
		customize_term_attributes(orig_termios);
	}
	res = input_impl(&mapping.arr, out, out);
	if (set_raw_mode)
		check_term_op(tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios));

	destroy_mapping(&mapping);

	return res;
}
