#include "packetized_out.h"
#include "util.h"
#include "windows.h"

#include <signal.h>
#include <stdio.h>
#ifndef _MSC_VER
#include <termios.h>
#include <sys/ioctl.h>
#endif

static int enabled;

static unsigned newline_records[WINDOW_TYPE_COUNT];
static unsigned current_window_newlines;
static int current_window_id = -1;
static int require_clear_screen = 1;

static void check_has_not_current_window(void)
{
	if (current_window_id != -1)
		DIE(0, "ウィンドの出力中: %d", current_window_id);
}

static void check_has_current_window(void)
{
	if (current_window_id == -1)
		DIE(0, "ウィンドの出力中ではない");
}

static void handle_sigwinch(int signal)
{
	require_clear_screen = 1;
}

void enable_windows(void)
{
#ifndef _MSC_VER
	struct sigaction sigact = {0};

	sigemptyset(&sigact.sa_mask);
	sigact.sa_handler = handle_sigwinch;
	if (sigaction(SIGWINCH, &sigact, NULL) == -1)
		DIE(1, "sigaction");
#endif

	enabled = 1;
}

void to_top_of_screen(void)
{
	check_has_not_current_window();

	if (!enabled)
		return;

	if (require_clear_screen) {
		add_packetized_out_null_terminated("\x1b[2J");
		require_clear_screen = 0;
	}
	add_packetized_out_null_terminated("\x1b[0;0H");
}

void start_window(int window_id)
{
	if (window_id < 0 || window_id >= WINDOW_TYPE_COUNT)
		DIE(0, "window_id が範囲外: %d", window_id);
	check_has_not_current_window();
	current_window_id = window_id;
	current_window_newlines = 0;
}

void add_window_newline(void)
{
	check_has_current_window();
	if (enabled)
		add_packetized_out_null_terminated("\x1b[0K");
	add_packetized_out_null_terminated("\n");
	current_window_newlines++;
}

void finish_window(void)
{
	check_has_current_window();
	if (!enabled)
		goto cleanup;

	while (current_window_newlines < newline_records[current_window_id])
		add_window_newline();

	newline_records[current_window_id] = current_window_newlines;

cleanup:
	current_window_id = -1;
}
