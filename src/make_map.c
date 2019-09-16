#include "commands.h"
#include "romazi.h"
#include "util.h"

int make_map(const char **argv, int argc) {
	struct key_mapping_array codes = {0};
	size_t i;
	get_romazi_codes(&codes);
	for (i = 0; i < codes.cnt; i++)
		xfprintf(out, "%s\t%s\n", codes.el[i].orig, codes.el[i].conv);
	DESTROY_ARRAY(codes);
	return 0;
}
