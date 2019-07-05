#include <stdio.h>

extern FILE *out, *err;

void print_free_kanji_keys(void);
int check_kanji_db_order(const char **argv, int argc);
int print_last_rank_contained(const char **argv, int argc);
