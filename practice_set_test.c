#include "commands.h"
#include "flags.h"
#include "test_util.h"

static struct flagset fs;

static void cleanup(void) { destroy_flagset(&fs); }

int main(void)
{
	set_test_source_file(__FILE__);

	while (run_test("basic", NULL)) {
		practice_set(&fs, NULL, 0);
		cleanup();
	}

	while (run_test("custom_layout", NULL)) {
		practice_set(&fs, (char *[]){
			"--kakko", "p",
			"--allow-left-bracket-key1",
			"--busy-right-pinky",
			"--romazi-optimize-keystrokes",
			"--no-classic-wo",
			"--hiragana-wo-key",
			"'"}, 8);
		cleanup();
	}
}
