#include <stdio.h>

#include "flags.h"

int check_kanji_db_order	(struct flagset *fs, char **argv, int argc);
int free_kanji_keys		(struct flagset *fs, char **argv, int argc);
int h2k				(struct flagset *fs, char **argv, int argc);
int input			(struct flagset *fs, char **argv, int argc);
int kana_stats			(struct flagset *fs, char **argv, int argc);
int kanji_db_chart		(struct flagset *fs, char **argv, int argc);
int longest_rsc_block		(struct flagset *fs, char **argv, int argc);
int make_map			(struct flagset *fs, char **argv, int argc);
int practice_set		(struct flagset *fs, char **argv, int argc);
int last_rank_contained		(struct flagset *fs, char **argv, int argc);
int rsc_gaps			(struct flagset *fs, char **argv, int argc);
int rsc_sort_key		(struct flagset *fs, char **argv, int argc);
int expand_rsc_keys		(struct flagset *fs, char **argv, int argc);
