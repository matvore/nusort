#ifndef KANA_STATS_UTIL_H
#define KANA_STATS_UTIL_H

#include <stdint.h>

void print_kana_stats_header(void);

void print_kana_stats_line(
	char const *left_column, int left_column_str_width, uint64_t hira_count,
	uint64_t kata_count, uint64_t all_kana_count);

#endif
