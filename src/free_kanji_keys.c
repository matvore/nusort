#include "commands.h"

#include "romazi.h"
#include "util.h"

int free_kanji_keys(char const *const *argv, int argc)
{
	struct short_code_array codes = {0};
	size_t i;

	if (argc) {
		xfprintf(err, "引数を渡さないでください。\n");
		exit(200);
	}

	get_free_kanji_codes(&codes);
	for (i = 0; i < codes.cnt; i++)
		xfprintf(out, "%.2s\n", codes.el[i]);

	free(codes.el);
	return 0;
}

