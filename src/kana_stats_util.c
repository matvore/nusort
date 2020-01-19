#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "commands.h"
#include "kana_stats_util.h"
#include "streams.h"
#include "util.h"

#define LEFT_COLUMN_WIDTH 7

void print_kana_stats_header(void)
{
	int i;
	for (i = 0; i < LEFT_COLUMN_WIDTH; i++)
		xfputc(' ', out);
	xfputs("    平仮名数 (  割合  )"
	       "    片仮名数 (  割合  )"
	       "  全文字中の割合\n",
	       out);
}

static uint32_t whole_part(uint32_t def, uint64_t numer, uint64_t denom)
{
	if (!denom)
		return def;
	return numer / denom;
}

static uint32_t fractional_part(
	uint32_t decimal_scale, uint64_t numer, uint64_t denom)
{
	if (!denom)
		return 0;
	return (decimal_scale * numer + denom/2) / denom % decimal_scale;
}

void print_kana_stats_line(
	char const *left_column, int left_column_str_width, uint64_t hira_count,
	uint64_t kata_count, uint64_t all_kana_count)
{
	uint64_t hira_kata_count = hira_count + kata_count;

	if (hira_kata_count > all_kana_count)
		DIE(0, "all_kana_count が低すぎます: %"PRIu64, all_kana_count);

	xfputs(left_column, out);
	while (left_column_str_width++ < LEFT_COLUMN_WIDTH)
		xfputc(' ', out);

	xfprintf(out, "%12"PRIu64" (%3d.%04"PRIu32")"
		      "%12"PRIu64" (%3d.%04"PRIu32")    "
		      "   %3d.%05"PRIu32"\n",

		      hira_count,
		      whole_part(0, hira_count * 100, all_kana_count),
		      fractional_part(10000, hira_count * 100, all_kana_count),

		      kata_count,
		      whole_part(0, kata_count * 100, all_kana_count),
		      fractional_part(10000, kata_count * 100, all_kana_count),

		      whole_part(75, hira_kata_count * 75, all_kana_count),
		      fractional_part(100000, hira_kata_count * 75,
				      all_kana_count));
}
