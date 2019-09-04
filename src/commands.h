#include <stdio.h>

extern FILE *out, *err;

int check_kanji_db_order(const char **argv, int argc);
int free_kanji_keys(const char **argv, int argc);
int make_map(const char **argv, int argc);
int print_last_rank_contained(const char **argv, int argc);
