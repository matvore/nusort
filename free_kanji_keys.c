#include "commands.h"
#include "kanji_distribution.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

int free_kanji_keys(struct flagset *fs, char **argv, int argc)
{
	size_t i;
	struct key_mapping_array romazi_m = {0};
	struct kanji_distribution kd = {0};

	romazi_flags(fs);
	kanji_distribution_flags(fs);

	if (argc < 0) return 0;

	parsflag(fs, &argc, argv);

	if (argc) {
		fprintf(err, "フラグを認識できませんでした：%s\n", argv[0]);
		return 200;
	}

	get_romazi_codes(fs, &romazi_m);
	kanji_distribution_set_preexisting_convs(fs, &kd, &romazi_m, 1);
	
	for (i = 0; i < kd.unused_kanji_origs.cnt; i++)
		fprintf(out, "%.2s\n", kd.unused_kanji_origs.el[i]);

	DESTROY_ARRAY(romazi_m);
	kanji_distribution_destroy(&kd);

	return 0;
}

