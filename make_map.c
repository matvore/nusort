#include "commands.h"
#include "mapping.h"
#include "mapping_util.h"
#include "streams.h"
#include "util.h"

int make_map(struct flagset *fs, char **argv, int argc) {
	struct mapping m = {0};
	size_t i;
	int res = 0;

	romazi_flags(fs);
	mapping_flags(fs);

	if (argc < 0) return 0;

	parsflag(fs, &argc, argv);
	if (argc) {
		fprintf(err, "フラグを認識できませんでした：%s\n", argv[0]);
		res = 3;
		goto cleanup;
	}

	get_romazi_codes(fs, &m.arr);
	res = mapping_populate(fs, &m);
	if (res)
		goto cleanup;

	for (i = 0; i < m.arr.cnt; i++)
		fprintf(out, "%s\t%s\n",
			 m.arr.el[i].orig, m.arr.el[i].conv);

cleanup:
	destroy_mapping(&m);

	return res;
}
