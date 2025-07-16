#define DICT_GUIDE_RSC_LIST_BUSHU 0
#define DICT_GUIDE_STROKE_COUNT 1
#define DICT_GUIDE_KANJI 2
#define DICT_GUIDE_ELLIPSIS 3
#define DICT_GUIDE_BUSHU_STROKE_COUNT 4
#define DICT_GUIDE_KUGIRI_INPUT_KEY 5
#define DICT_GUIDE_SPACE 6
#define DICT_GUIDE_LINE_WRAPPABLE_POINT 7

struct dict_guide_el {
	unsigned type;

	union {
		unsigned rsc_list_bushu_ki;
		unsigned stroke_count;
		struct {
			unsigned ki;
			char input_c;
		} kanji;
		char kugiri_input_key;
	} u;
};

void dict_guide_clear(void);
struct dict_guide_el *dict_guide_add_el(void);
void dict_guide_show(int include_second_line);
