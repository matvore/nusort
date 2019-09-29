#include "romazi.h"

struct mapping {
	struct key_mapping_array codes;
	unsigned ergonomic_sort : 1;
};

void mapping_populate(struct mapping *mapping);
void mapping_destroy(struct mapping *mapping);
