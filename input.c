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

int input(char **argv, int argc, int set_raw_mode)
{
	struct mapping mapping = {0};
#if HAVE_TERMIOS
	struct termios orig_termios;
#endif
	struct romazi_config romazi_config = {0};
	struct input_flags flags = {
		.show_pending_and_converted = 1,
		.show_keyboard = 1,
		.show_cutoff_guide = 1,
		.save_with_osc52 = 1,
		.show_rsc_list = 1,
	};
	int res;

	init_romazi_config_for_cli_flags(&romazi_config);
	init_mapping_config_for_cli_flags(&mapping);

	while (argc > 0 && argv[0][0] == '-') {
		if (parse_mapping_flags(&argc, &argv, &mapping))
			continue;
		if (parse_romazi_flags(&argc, &argv, &romazi_config))
			continue;
		if (!strcmp(argv[0], "--no-show-cutoff-guide")) {
			argv++;
			argc--;
			flags.show_cutoff_guide = 0;
			continue;
		}
		if (!strcmp(argv[0], "--no-show-rsc-list")) {
			argv++;
			argc--;
			flags.show_rsc_list = 0;
			continue;
		}
		if (!strcmp(argv[0], "--rpc-mode")) {
			argv++;
			argc--;
			hide_term_warn = 1;
			flags.rpc_mode = 1;
			continue;
		}

		badflag(argv[0]);
	}

	get_romazi_codes(&romazi_config, &mapping.arr);

	append_mapping(&mapping.arr, "\"", "々");
	append_mapping(&mapping.arr, ".  ", "。");
	append_mapping(&mapping.arr, ",  ", "、");
	append_mapping(&mapping.arr, "/  ", "・");
	append_mapping(&mapping.arr, "\\  ", "￥");

	res = mapping_populate(&mapping);
	if (res)
		return res;

	if (set_raw_mode) {
#if HAVE_TERMIOS
		check_term_op(tcgetattr(STDIN_FILENO, &orig_termios));
		customize_term_attributes(orig_termios);
#endif
		enable_windows();
	}
	res = input_impl(&mapping, &flags);
#if HAVE_TERMIOS
	if (set_raw_mode)
		check_term_op(tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios));
#endif

	destroy_mapping(&mapping);

	return res;
}
