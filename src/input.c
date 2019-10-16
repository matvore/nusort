#include "input_impl.h"
#include "mapping.h"
#include "util.h"

#include <errno.h>
#include <termios.h>
#include <unistd.h>

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

int input(char const *const *argv, int argc)
{
	struct mapping mapping = {0};
	struct termios orig_termios;
	int res;

	mapping.ergonomic_sort = 1;
	mapping_populate(&mapping);

	check_term_op(tcgetattr(STDIN_FILENO, &orig_termios));
	customize_term_attributes(orig_termios);
	res = input_impl(&mapping);
	check_term_op(tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios));

	mapping_destroy(&mapping);

	return res;
}