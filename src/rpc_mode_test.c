#include "input_impl.h"
#include "streams.h"
#include "test_util.h"

#include <stdlib.h>

int main(int argc, char **argv)
{
	set_test_source_file(__FILE__);
	while (run_test("await_input", "\x01")) {
		struct input_flags f = {
			.rpc_mode = 1,
		};
		struct mapping m = {0};

		append_mapping(&m.arr, "xyz", "あ");
		in = open_tmp_file_containing("");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("wrap_output", "\x01"
				       "\x04\x04" "<x>\n"
				       "\x01")) {
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

	while (run_test("wrap_output_longer_orig", "\x01"
						   "\x04\x04" "<x>\n"
						   "\x01"
						   "\x04\x05" "<xy>\n"
						   "\x01"
						   "\x04\x06" "<xyz>\n"
						   "\x01"
						   "\x02\x03" "あ"
						   "\x01")) {
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

	while (run_test("unrecognized_orig_pref", "\x01"
						  "\x02\x01" "u"
						  "\x01")) {
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

	while (run_test("propagate_newline", "\x01"
					     "\x02" "\x03" "あ"
					     "\x01"
					     "\x02" "\x01" "\n"
					     "\x01")) {
		struct mapping m = {0};
		struct input_flags f = {
			.rpc_mode = 1,
		};

		append_mapping(&m.arr, "a", "あ");
		in = open_tmp_file_containing("a\n");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}

	while (run_test("process_or_propagate_backspace", "\x01"
							  "\x04" "\x04" "<x>\n"
							  "\x01"
							  "\x04" "\x03" "<>\n"
							  "\x01"
							  "\x02" "\x01" "\b"
							  "\x01")) {
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

	while (run_test("propagate_arrow_keys_1",
			"\x01"
			"\x02" "\x03" "\x1b[A"
			"\x01"
			"\x02" "\x03" "\x1b[B"
			"\x01"
			"\x02" "\x02" "\x1ba"
			"\x01")) {
		struct mapping m = {0};
		struct input_flags f = {
			.rpc_mode = 1,
			.show_pending_and_converted = 1,
		};

		append_mapping(&m.arr, "xa", "あ");
		in = open_tmp_file_containing("\x1b[A" "\x1b[B" "\x1ba");
		expect_ok(input_impl(&m, &f));

		destroy_mapping(&m);
	}
}
