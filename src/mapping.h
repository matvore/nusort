#ifndef MAPPING_H
#define MAPPING_H

#include "romazi.h"

struct mapping {
	struct key_mapping_array codes;
	unsigned ergonomic_sort : 1;
};

int code_cmp(char const *a, char const *b);
void mapping_populate(struct mapping *mapping);
void mapping_destroy(struct mapping *mapping);

#endif
