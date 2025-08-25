#include "commands.h"
#include "input_impl.h"
#include "streams.h"
#include "test_util.h"

#include <stdlib.h>

static struct flagset fs;
static struct mapping m;

static void initflags(void)
{
	input(&fs, 0, -1);
	setflag(&fs, "--no-kanji",			1);
	setflag(&fs, "--recurs-ksort",			1);
	setflag(&fs, "--no-show-pending-and-converted",	1);
	setflag(&fs, "--no-show-keyboard",		1);
	setflag(&fs, "--no-show-cutoff-guide",		1);
	setflag(&fs, "--no-show-rsc-list",		1);
	setflag(&fs, "--no-real-tty",			1);
	setflag(&fs, "--no-save-with-osc52",		1);
	setflag(&fs, "--rpc-mode",			1);
}

static void cleanup(void) { destroy_flagset(&fs); destroy_mapping(&m); }

int main(int argc, char **argv)
{
	set_test_source_file(__FILE__);
	debugout = 1;
	while (run_test("await_input", 0)) {
		initflags();

		append_mapping(&m.arr, "xyz", "あ");
		in = open_tmp_file_containing("");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}

	while (run_test("wrap_output", 0)) {
		initflags();
		setflag(&fs, "--no-show-pending-and-converted", 0);

		append_mapping(&m.arr, "xyz", "あ");
		in = open_tmp_file_containing("x");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}

	while (run_test("wrap_output_longer_orig", 0)) {
		initflags();
		setflag(&fs, "--no-show-pending-and-converted", 0);

		append_mapping(&m.arr, "xyz.", "あ");
		in = open_tmp_file_containing("xyz.");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}

	while (run_test("unrecognized_orig_pref", 0)) {
		initflags();
		setflag(&fs, "--no-show-pending-and-converted", 0);

		append_mapping(&m.arr, "a", "あ");
		in = open_tmp_file_containing("u");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}

	while (run_test("show_cutoff_guide", NULL)) {
		initflags();
		setflag(&fs, "--no-show-pending-and-converted", 0);
		setflag(&fs, "--no-kanji",			0);
		setflag(&fs, "--no-show-cutoff-guide",		0);

		expect_ok(mapping_populate(&fs, &m));
		in = open_tmp_file_containing("");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}

	while (run_test("show_rsc_list", NULL)) {
		initflags();
		setflag(&fs, "--no-show-pending-and-converted", 0);
		setflag(&fs, "--no-kanji",			0);
		setflag(&fs, "--no-show-rsc-list",		0);

		expect_ok(mapping_populate(&fs, &m));
		in = open_tmp_file_containing("k");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}

	while (run_test("propagate_newline", 0)) {
		initflags();

		append_mapping(&m.arr, "a", "あ");
		in = open_tmp_file_containing("a\n");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}

	while (run_test("process_or_propagate_backspace", 0)) {
		initflags();
		setflag(&fs, "--no-show-pending-and-converted", 0);

		append_mapping(&m.arr, "xa", "あ");
		in = open_tmp_file_containing("x\b\b");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}

	while (run_test("propagate_arrow_keys_1", 0)) {
		initflags();
		setflag(&fs, "--no-show-pending-and-converted", 0);

		append_mapping(&m.arr, "xa", "あ");
		in = open_tmp_file_containing("\x1b[A" "\x1b[B" "\x1b" "a");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}

	while (run_test("no_extra_output_after_esc_eof", 0)) {
		initflags();
		setflag(&fs, "--no-show-pending-and-converted", 0);

		append_mapping(&m.arr, "xa", "あ");
		in = open_tmp_file_containing("\x1b");
		expect_ok(input_impl(&m, &fs));

		cleanup();
	}
}
