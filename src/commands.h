#include <stdio.h>

extern FILE *out, *err, *in;

int check_kanji_db_order(char const *const *argv, int argc);
int free_kanji_keys(char const *const *argv, int argc);
int input(char const *const *argv, int argc);
int make_map(char const *const *argv, int argc);
int print_last_rank_contained(char const *const *argv, int argc);
