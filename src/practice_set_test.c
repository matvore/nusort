#include "commands.h"
#include "test_util.h"

int main(void)
{
	set_test_source_file(__FILE__);

	while (run_test("basic", NULL)) {
		practice_set(NULL, 0);
	}

	while (run_test("custom_layout", NULL)) {
		practice_set((const char *[]){
			"--pack-kakko",
			"--allow-left-bracket-key1",
			"--busy-right-pinky",
			"--romazi-optimize-keystrokes",
			"--no-classic-wo",
			"--hiragana-wo-key",
			"'"}, 7);
	}
}
