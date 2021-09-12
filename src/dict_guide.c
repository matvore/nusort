#include "dict_guide.h"
#include "kanji_db.h"
#include "streams.h"
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
	memset(guide.el, 0, sizeof(*guide.el) * guide.cnt);
	guide.cnt = 0;
}

struct dict_guide_el *dict_guide_add_el(void)
{
	GROW_ARRAY_BY(guide, 1);
	return guide.el + guide.cnt - 1;
}

#define RSC_LIST_WRAP_WIDTH 60

static void print_kanji_line(int i)
{
	switch (guide.el[i].type) {
	case DICT_GUIDE_RSC_LIST_BUSHU:
		fprintf(out, "\x1b[%d;%dm%s\x1b[%dm",
			ANSI_BRIGHT_MAGENTA_FG, ANSI_BOLD,
			kanji_db()[guide.el[i].u.rsc_list_bushu_ki].c,
			ANSI_RESET);
		break;
	case DICT_GUIDE_STROKE_COUNT:
		fprintf(out, "\x1b[%dm%d\x1b[%dm",
			ANSI_BRIGHT_YELLOW_FG,
			guide.el[i].u.stroke_count,
			ANSI_RESET);
		break;
	case DICT_GUIDE_KANJI:
		fputs(kanji_db()[guide.el[i].u.kanji.ki].c, out);
		break;
	case DICT_GUIDE_ELLIPSIS:
		fputs("⋯", out);
		break;
	case DICT_GUIDE_BUSHU_STROKE_COUNT:
		fprintf(out, "\x1b[%d;%dm %d画 \x1b[%dm",
			ANSI_BRIGHT_MAGENTA_FG, ANSI_BOLD,
			guide.el[i].u.stroke_count, ANSI_RESET);
		break;
	case DICT_GUIDE_KUGIRI_INPUT_KEY:
		fprintf(out, "\x1b[%dm %c \x1b[%dm", ANSI_REVERSE_VIDEO,
			guide.el[i].u.kugiri_input_key, ANSI_RESET);
		break;
	case DICT_GUIDE_SPACE:
		fputc(' ', out);
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
		fprintf(out, "\x1b[%d;%dm部\x1b[%dm", ANSI_BRIGHT_MAGENTA_FG,
			ANSI_BOLD, ANSI_RESET);
		break;
	case DICT_GUIDE_STROKE_COUNT:
		fputc(' ', out);
		if (guide.el[i].u.stroke_count >= 10)
			fputc(' ', out);
		break;
	case DICT_GUIDE_KANJI:
		fprintf(out, "%c ", guide.el[i].u.kanji.input_c);
		break;
	case DICT_GUIDE_SPACE:
		fputc(' ', out);
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
			fputc(' ', out);
		for (i = last_fitting_chunk; i >= amount_printed; i--)
			print_kanji_line(i);
		add_window_newline();

		if (include_second_line) {
			for (i = 0; i < last_fitting_padding; i++)
				fputc(' ', out);
			for (i = last_fitting_chunk; i >= amount_printed; i--)
				print_key_line(i);
			add_window_newline();
		}

		amount_printed = last_fitting_chunk + 1;
	}
}
