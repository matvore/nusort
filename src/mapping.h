#ifndef MAPPING_H
#define MAPPING_H

#include <stdio.h>

#include "romazi.h"

struct mapping_config {
	unsigned ergonomic_sort : 1;
	unsigned include_kanji : 1;
};

int mapping_populate(struct mapping_config const *, struct key_mapping_array *);

#endif
