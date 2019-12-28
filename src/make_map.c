#include "commands.h"
#include "mapping.h"
#include "streams.h"
#include "util.h"

int make_map(char const *const *argv, int argc) {
	struct mapping mapping = {0};
	size_t i;
	int res = 0;
	struct romazi_config romazi_config = {0};

	init_romazi_config_for_cli_flags(&romazi_config);

	mapping.ergonomic_sort = 0;
	mapping.include_kanji = 1;

	while (argc > 0) {
		if (!strcmp(argv[0], "-s")) {
			mapping.ergonomic_sort = 1;
			argv++;
			argc--;
		} else if (!strcmp(argv[0], "--no-kanji")) {
			mapping.include_kanji = 0;
			argv++;
			argc--;
		} else if (!parse_romazi_flags(&argc, &argv, &romazi_config)) {
			xfprintf(err,
				 "フラグを認識できませんでした：%s\n", argv[0]);
			res = 3;
			goto cleanup;
		}
	}

	init_romazi(&romazi_config);
	mapping_populate(&mapping);

	for (i = 0; i < mapping.codes.cnt; i++)
		xfprintf(out, "%s\t%s\n",
			 mapping.codes.el[i].orig, mapping.codes.el[i].conv);

cleanup:
	mapping_destroy(&mapping);

	return res;
}
