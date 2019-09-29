#include "commands.h"
#include "mapping.h"
#include "util.h"

int make_map(char const *const *argv, int argc) {
	struct mapping mapping = {0};
	size_t i;
	int res = 0;

	mapping.ergonomic_sort = 0;

	while (argc > 0) {
		const char *arg = argv[0];
		argc--;
		argv++;
		if (!strcmp(arg, "-s")) {
			mapping.ergonomic_sort = 1;
		} else {
			xfprintf(err,
				 "フラグを認識できませんでした：%s\n", arg);
			res = 3;
			goto cleanup;
		}
	}

	mapping_populate(&mapping);

	for (i = 0; i < mapping.codes.cnt; i++)
		xfprintf(out, "%s\t%s\n",
			 mapping.codes.el[i].orig, mapping.codes.el[i].conv);

cleanup:
	mapping_destroy(&mapping);

	return res;
}
