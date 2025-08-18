#include "input_impl.h"
#include "streams.h"
#include "test_util.h"

#include <stdlib.h>

int main(int argc, char **argv)
{
	set_test_source_file(__FILE__);
	debugout = 1;
	while (run_test("await_input", 0)) {
		struct input_flags f = {
			.rpc_mode = 1,
		};
		struct mapping m = {0};

		append_mapping(&m.arr, "xyz", "あ");
		in = open_tmp_file_containing("");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("wrap_output", 0)) {
		struct mapping m = {0};
		struct input_flags f = {
			.rpc_mode = 1,
			.show_pending_and_converted = 1,
		};

		append_mapping(&m.arr, "xyz", "あ");
		in = open_tmp_file_containing("x");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("wrap_output_longer_orig", 0)) {
		struct mapping m = {0};
		struct input_flags f = {
			.rpc_mode = 1,
			.show_pending_and_converted = 1,
		};

		append_mapping(&m.arr, "xyz.", "あ");
		in = open_tmp_file_containing("xyz.");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("unrecognized_orig_pref", 0)) {
		struct mapping m = {0};
		struct input_flags f = {
			.rpc_mode = 1,
			.show_pending_and_converted = 1,
		};

		append_mapping(&m.arr, "a", "あ");
		in = open_tmp_file_containing("u");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("show_cutoff_guide", NULL)) {
		struct mapping m = {
			.include_kanji = 1,
		};
		struct input_flags f = {
			.rpc_mode = 1,
			.show_pending_and_converted = 1,
			.show_cutoff_guide = 1,
		};

		expect_ok(mapping_populate(&m));
		in = open_tmp_file_containing("");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("show_rsc_list", NULL)) {
		struct mapping m = {
			.include_kanji = 1,
		};
		struct input_flags f = {
			.rpc_mode = 1,
			.show_pending_and_converted = 1,
			.show_rsc_list = 1,
		};

		expect_ok(mapping_populate(&m));
		in = open_tmp_file_containing("k");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("propagate_newline", 0)) {
		struct mapping m = {0};
		struct input_flags f = {
			.rpc_mode = 1,
		};

		append_mapping(&m.arr, "a", "あ");
		in = open_tmp_file_containing("a\n");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("process_or_propagate_backspace", 0)) {
		struct mapping m = {0};
		struct input_flags f = {
			.rpc_mode = 1,
			.show_pending_and_converted = 1,
		};

		append_mapping(&m.arr, "xa", "あ");
		in = open_tmp_file_containing("x\b\b");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("propagate_arrow_keys_1", 0)) {
		struct mapping m = {0};
		struct input_flags f = {
			.rpc_mode = 1,
			.show_pending_and_converted = 1,
		};

		append_mapping(&m.arr, "xa", "あ");
		in = open_tmp_file_containing("\x1b[A" "\x1b[B" "\x1b" "a");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("no_extra_output_after_esc_eof", 0)) {
		struct mapping m = {0};
		struct input_flags f = {
			.rpc_mode = 1,
			.show_pending_and_converted = 1,
		};

		append_mapping(&m.arr, "xa", "あ");
		in = open_tmp_file_containing("\x1b");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}
}
