#include "commands.h"
#include "kanji_distribution.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

int free_kanji_keys(char **argv, int argc)
{
	size_t i;
	struct romazi_config romazi_config = {0};
	struct key_mapping_array romazi_m = {0};
	struct kanji_distribution kd = {0};

	init_romazi_config_for_cli_flags(&romazi_config);

	while (argc > 0 && argv[0][0] == '-') {
		if (!strcmp(argv[0], "--")) {
			argv++;
			argc--;
			break;
		} else if (!parse_romazi_flags(&argc, &argv, &romazi_config) &&
			   !parse_kanji_distribution_flags(&argc, &argv, &kd)) {
			fprintf(err,
				 "フラグを認識できませんでした：%s\n", argv[0]);
			return 200;
		}
	}

	get_romazi_codes(&romazi_config, &romazi_m);
	kanji_distribution_set_preexisting_convs(&kd, &romazi_m, 1);
	
	for (i = 0; i < kd.unused_kanji_origs.cnt; i++)
		fprintf(out, "%.2s\n", kd.unused_kanji_origs.el[i]);

	DESTROY_ARRAY(romazi_m);
	kanji_distribution_destroy(&kd);

	return 0;
}

