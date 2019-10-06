#include "mapping.h"

#include <stdio.h>

char const *keyboard_bytes(void);
size_t keyboard_size(void);
void keyboard_write(FILE *stream);
void keyboard_update(struct mapping const *mapping, char const *prefix);
