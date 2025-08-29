#include "dict_guide.h"
#include "kanji_db.h"
#include "packetized_out.h"
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
	/* memset にヌルポインターを渡すのは未定義動作 */
	if (guide.cnt) memset(guide.el, 0, sizeof(*guide.el) * guide.cnt);
	guide.cnt = 0;
}

struct dict_guide_el *dict_guide_add(int type, int highlight)
{
	struct dict_guide_el *ne;
	GROW_ARRAY_BY(guide, 1);
	ne = guide.el + guide.cnt - 1;

	ne->type = type;
	ne->highlight = !!highlight;

	return ne;
}

static void out_stroke_count(struct dict_guide_el *e)
{
	char dig[3];
	unsigned sc = e->u.stroke_count;

	if (sc > 99) DIE(0, "画数が無効です: %u", sc);
	sprintf(dig, "%u", sc);

	packout(dig, -1);
}

static void colorout(const char *s) { if (!debugout) packout(s, -1); }

static void do_el(int i, const char *flags, int *wid_)
{
	int kan_line = !!strchr(flags, '1');
	int key_line = !!strchr(flags, '2');
	int any_line = kan_line || key_line;
	int reset = 0;
	struct dict_guide_el *e = guide.el + i;
	int wid;

	if (e->highlight) {
		colorout("\x1b[" ANSI_REVERSE_VIDEO "m");
		reset = 1;
	}

	switch (e->type) {
	case DICT_GUIDE_RSC_LIST_BUSHU:
		wid = 2;
		if (any_line) {
			colorout("\x1b["	ANSI_BRIGHT_MAGENTA_FG ";"
						ANSI_BOLD "m");
			reset = 1;
		}
		if (kan_line) packout(kanji_db()[e->u.rsc_list_bushu_ki].c, -1);
		if (key_line) packout("部", -1);
		break;
	case DICT_GUIDE_STROKE_COUNT:
		wid = e->u.stroke_count >= 10 ? 2 : 1;
		if (kan_line) {
			colorout("\x1b[" ANSI_BRIGHT_YELLOW_FG "m");
			reset = 1;
			out_stroke_count(e);
		}
		if (key_line) packout("  ", wid);
		break;
	case DICT_GUIDE_KANJI:
		wid = 2;
		if (kan_line) packout(kanji_db()[e->u.kanji.ki].c, -1);
		if (key_line) {
			packout(&e->u.kanji.input_c, 1);
			packout(" ", -1);
		}
		break;
	case DICT_GUIDE_SPACE:
		wid = 1;
		if (any_line) packout(" ", -1);
		break;
	case DICT_GUIDE_ELLIPSIS:
		wid = 1;
		if (any_line) packout("⋯", -1);
		break;
	case DICT_GUIDE_BUSHU_STROKE_CNT:
		wid = 5 + (e->u.stroke_count >= 10 ? 1 : 0);

		if (any_line) {
			colorout("\x1b["	ANSI_BRIGHT_MAGENTA_FG ";"
						ANSI_BOLD "m");
			reset = 1;

			packout(" ", -1);
			out_stroke_count(e);
			packout("画 ", -1);
		}
		break;
	case DICT_GUIDE_LINE_WRAPPABLE_POINT:
		wid = 0;
		break;
	case DICT_GUIDE_KUGIRI_INPUT_KEY:
		wid = 3;
		if (any_line) {
			packout(" ", -1);
			packout(&e->u.kugiri_input_key, 1);
			packout(" ", -1);
		}
		break;
	default: DIE(0, "%d", e->type);
	}

	if (wid_) *wid_ = wid;
	if (reset) colorout("\x1b[" ANSI_RESET "m");
}

void dict_guide_show(int width, int include_second_line)
{
	int amount_printed = 0;

	while (amount_printed < guide.cnt) {
		int remaining_width = width, last_fitting_padding;
		unsigned cursor = amount_printed, last_fitting_chunk = 0;
		int i, thiswid;

		while (cursor < guide.cnt &&
		       (remaining_width >= 0 || !last_fitting_chunk)) {
			int can_wrap = cursor == guide.cnt - 1;

			do_el(cursor, "", &thiswid);
			remaining_width -= thiswid;
			if (!thiswid) can_wrap = 1;

			if (can_wrap && (remaining_width >= 0 ||
					 !last_fitting_chunk)) {
				last_fitting_chunk = cursor;
				last_fitting_padding = remaining_width;
			}
			cursor++;
		}

		for (i = 0; i < last_fitting_padding; i++)
			packout(" ", -1);
		for (i = last_fitting_chunk; i >= amount_printed; i--)
			do_el(i, "1", 0);
		add_window_newline();

		if (include_second_line) {
			for (i = 0; i < last_fitting_padding; i++)
				packout(" ", -1);
			for (i = last_fitting_chunk; i >= amount_printed; i--)
				do_el(i, "2", 0);
			add_window_newline();
		}

		amount_printed = last_fitting_chunk + 1;
	}
}
