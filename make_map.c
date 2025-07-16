#include "commands.h"
#include "mapping.h"
#include "mapping_util.h"
#include "streams.h"
#include "util.h"

int make_map(char const *const *argv, int argc) {
	struct mapping m = {0};
	size_t i;
	int res = 0;
	struct romazi_config romazi_config = {0};

	init_romazi_config_for_cli_flags(&romazi_config);
	init_mapping_config_for_cli_flags(&m);

	while (argc > 0) {
		if (parse_mapping_flags(&argc, &argv, &m))
			continue;
		if (parse_romazi_flags(&argc, &argv, &romazi_config))
			continue;

		fprintf(err,
			 "フラグを認識できませんでした：%s\n", argv[0]);
		res = 3;
		goto cleanup;
	}

	get_romazi_codes(&romazi_config, &m.arr);
	res = mapping_populate(&m);
	if (res)
		goto cleanup;

	for (i = 0; i < m.arr.cnt; i++)
		fprintf(out, "%s\t%s\n",
			 m.arr.el[i].orig, m.arr.el[i].conv);

cleanup:
	destroy_mapping(&m);

	return res;
}
