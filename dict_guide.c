#include "dict_guide.h"
#include "kanji_db.h"
#include "packetized_out.h"
#include "util.h"
#include "windows.h"

#include <stddef.h>
#include <string.h>

static struct {
	struct dict_guide_el *el;
	size_t cnt, alloc;
} guide;

void dict_guide_clear(void)
{
	/* memset にヌルポインターを渡すのは未定義行動 */
	if (guide.cnt) memset(guide.el, 0, sizeof(*guide.el) * guide.cnt);
	guide.cnt = 0;
}

struct dict_guide_el *dict_guide_add_el(void)
{
	GROW_ARRAY_BY(guide, 1);
	return guide.el + guide.cnt - 1;
}

#define RSC_LIST_WRAP_WIDTH 60

static void out_stroke_count(int guide_el_i)
{
	unsigned sc;
	char dig[3];

	sc = guide.el[guide_el_i].u.stroke_count;

	if (sc > 99) DIE(0, "画数が無効です: %u", sc);
	sprintf(dig, "%u", sc);

	add_packetized_out_null_terminated(dig);
}

static void print_kanji_line(int i)
{
	switch (guide.el[i].type) {
	case DICT_GUIDE_RSC_LIST_BUSHU:
		add_packetized_out_null_terminated(
			"\x1b[" ANSI_BRIGHT_MAGENTA_FG ";" ANSI_BOLD "m");
		add_packetized_out_null_terminated(
			kanji_db()[guide.el[i].u.rsc_list_bushu_ki].c);
		add_packetized_out_null_terminated("\x1b[" ANSI_RESET "m");
		break;
	case DICT_GUIDE_STROKE_COUNT:
		add_packetized_out_null_terminated(
			"\x1b[" ANSI_BRIGHT_YELLOW_FG "m");
		out_stroke_count(i);
		add_packetized_out_null_terminated("\x1b[" ANSI_RESET "m");
		break;
	case DICT_GUIDE_KANJI:
		add_packetized_out_null_terminated(
			kanji_db()[guide.el[i].u.kanji.ki].c);
		break;
	case DICT_GUIDE_ELLIPSIS:
		add_packetized_out_null_terminated("⋯");
		break;
	case DICT_GUIDE_BUSHU_STROKE_COUNT:
		add_packetized_out_null_terminated(
			"\x1b[" ANSI_BRIGHT_MAGENTA_FG ";" ANSI_BOLD "m ");

		out_stroke_count(i);
		add_packetized_out_null_terminated("画 \x1b[" ANSI_RESET "m");
		break;
	case DICT_GUIDE_KUGIRI_INPUT_KEY:
		add_packetized_out_null_terminated(
			"\x1b[" ANSI_REVERSE_VIDEO "m ");
		add_packetized_out(&guide.el[i].u.kugiri_input_key, 1);
		add_packetized_out_null_terminated(" \x1b[" ANSI_RESET "m");
		break;
	case DICT_GUIDE_SPACE:
		add_packetized_out_null_terminated(" ");
		break;
	case DICT_GUIDE_LINE_WRAPPABLE_POINT:
		break;
	default:
		DIE(0, "");
	}
}

static void print_key_line(int i)
{
	switch (guide.el[i].type) {
	case DICT_GUIDE_RSC_LIST_BUSHU:
		add_packetized_out_null_terminated(
			"\x1b[" ANSI_BRIGHT_MAGENTA_FG ";" ANSI_BOLD "m部\x1b["
			ANSI_RESET "m");
		break;
	case DICT_GUIDE_STROKE_COUNT:
		add_packetized_out("  ",
				   guide.el[i].u.stroke_count >= 10 ? 2 : 1);
		break;
	case DICT_GUIDE_KANJI:
		add_packetized_out(&guide.el[i].u.kanji.input_c, 1);
		add_packetized_out_null_terminated(" ");
		break;
	case DICT_GUIDE_SPACE:
		add_packetized_out_null_terminated(" ");
		break;
	case DICT_GUIDE_LINE_WRAPPABLE_POINT:
		break;
	default:
		DIE(0, "");
	}
}

void dict_guide_show(int include_second_line)
{
	int amount_printed = 0;

	while (amount_printed < guide.cnt) {
		int remaining_width = RSC_LIST_WRAP_WIDTH, last_fitting_padding;
		unsigned cursor = amount_printed, last_fitting_chunk = 0;
		int i;

		while (cursor < guide.cnt &&
		       (remaining_width >= 0 || !last_fitting_chunk)) {
			int can_wrap = cursor == guide.cnt - 1;

			switch (guide.el[cursor].type) {
			case DICT_GUIDE_RSC_LIST_BUSHU:
				remaining_width -= 2;
				break;
			case DICT_GUIDE_KANJI:
				remaining_width -= 2;
				break;
			case DICT_GUIDE_STROKE_COUNT:
				remaining_width -= 1;
				if (guide.el[cursor].u.stroke_count >= 10)
					remaining_width -= 1;
				break;
			case DICT_GUIDE_ELLIPSIS:
				remaining_width -= 1;
				break;
			case DICT_GUIDE_BUSHU_STROKE_COUNT:
				remaining_width -= 5;
				if (guide.el[cursor].u.stroke_count >= 10)
					remaining_width -= 1;
				break;
			case DICT_GUIDE_KUGIRI_INPUT_KEY:
				remaining_width -= 3;
				break;
			case DICT_GUIDE_SPACE:
				remaining_width -= 1;
				break;
			case DICT_GUIDE_LINE_WRAPPABLE_POINT:
				can_wrap = 1;
				break;
			default:
				DIE(0, "");
			}

			if (can_wrap && (remaining_width >= 0 ||
					 !last_fitting_chunk)) {
				last_fitting_chunk = cursor;
				last_fitting_padding = remaining_width;
			}
			cursor++;
		}

		for (i = 0; i < last_fitting_padding; i++)
			add_packetized_out_null_terminated(" ");
		for (i = last_fitting_chunk; i >= amount_printed; i--)
			print_kanji_line(i);
		add_window_newline();

		if (include_second_line) {
			for (i = 0; i < last_fitting_padding; i++)
				add_packetized_out_null_terminated(" ");
			for (i = last_fitting_chunk; i >= amount_printed; i--)
				print_key_line(i);
			add_window_newline();
		}

		amount_printed = last_fitting_chunk + 1;
	}
}
