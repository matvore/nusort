#include "commands.h"
#include "mapping.h"
#include "mapping_util.h"
#include "streams.h"
#include "util.h"

int make_map(char const *const *argv, int argc) {
	struct mapping_config mapping_config = {0};
	struct key_mapping_array mapping = {0};
	size_t i;
	int res = 0;
	struct romazi_config romazi_config = {0};

	init_romazi_config_for_cli_flags(&romazi_config);

	mapping_config.ergonomic_sort = 0;
	mapping_config.include_kanji = 1;

	while (argc > 0) {
		if (!strcmp(argv[0], "-s")) {
			mapping_config.ergonomic_sort = 1;
			argv++;
			argc--;
		} else if (!strcmp(argv[0], "--no-kanji")) {
			mapping_config.include_kanji = 0;
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
	if (!mapping_populate(&mapping_config, &mapping)) {
		res = 11;
		goto cleanup;
	}

	for (i = 0; i < mapping.cnt; i++)
		xfprintf(out, "%s\t%s\n",
			 mapping.el[i].orig, mapping.el[i].conv);

cleanup:
	DESTROY_ARRAY(mapping);

	return res;
}
