#ifndef MAPPING_H
#define MAPPING_H

#include <stdio.h>

#include "romazi.h"

struct mapping {
	struct key_mapping_array codes;
	unsigned ergonomic_sort : 1;
	unsigned include_kanji : 1;
};

int mapping_populate(struct mapping *mapping);
void mapping_destroy(struct mapping *mapping);

#endif
