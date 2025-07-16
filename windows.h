#define WINDOW_CUTOFF_GUIDE 0
#define WINDOW_RSC_LIST 1
#define WINDOW_INPUT_LINE 2
#define WINDOW_TYPE_COUNT 3

void enable_windows(void);
void to_top_of_screen(void);
void start_window(int window_id);
void add_window_newline(void);
void finish_window(void);
