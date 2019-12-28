#include "commands.h"

#include "romazi.h"
#include "streams.h"
#include "util.h"

int free_kanji_keys(char const *const *argv, int argc)
{
	struct short_code_array codes = {0};
	size_t i;
	struct romazi_config romazi_config = {0};

	init_romazi_config_for_cli_flags(&romazi_config);

	while (argc > 0 && argv[0][0] == '-') {
		if (!strcmp(argv[0], "--")) {
			argv++;
			argc--;
			break;
		} else if (!parse_romazi_flags(&argc, &argv, &romazi_config)) {
			xfprintf(err,
				 "フラグを認識できませんでした：%s\n", argv[0]);
			return 200;
		}
	}

	init_romazi(&romazi_config);
	get_free_kanji_codes(&codes);
	for (i = 0; i < codes.cnt; i++)
		xfprintf(out, "%.2s\n", codes.el[i]);

	free(codes.el);
	return 0;
}

