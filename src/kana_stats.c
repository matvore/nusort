#include <stdint.h>

#include "chars.h"
#include "commands.h"
#include "kana_stats_util.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

/* 未サポート: e3 82 9[9a] (composed 濁点＋半濁点 ) */

struct stats_entry {
	Conv kana;
	uint64_t count;
};

struct stats_map {
	struct stats_entry *el;
	size_t cnt;
	size_t alloc;
};

static void collect_potential_mappings(struct stats_map *out)
{
	struct key_mapping_array input_mapping = {0};
	struct romazi_config romazi_config = {0};
	int i;

	init_romazi(&romazi_config);

	get_romazi_codes(&input_mapping);

	for (i = 0; i < input_mapping.cnt; i++) {
		char const *conv = input_mapping.el[i].conv;
		/*
		 * 濁点を必要とする仮名をサポートしない。その字は実際ではあまり
		 * 使われない。
		 */
		if (!strncmp(conv + 3, "\xe3\x82\x99", 3))
			continue;

		GROW_ARRAY_BY(*out, 1);
		memcpy(&out->el[out->cnt - 1], conv, sizeof(Conv));
	}

	QSORT(, out->el, out->cnt,
	      strcmp(out->el[a].kana, out->el[b].kana) < 0);

	DESTROY_ARRAY(input_mapping);
}

static uint64_t kata_count_of(struct stats_map const *stats, char const *hira)
{
	Conv kata;
	struct stats_entry *kata_entry;

	memcpy(kata, hira, sizeof(kata));
	hiragana_to_katakana(kata);
	BSEARCH(kata_entry, stats->el, stats->cnt,
		strcmp(kata_entry->kana, kata));
	if (!kata_entry)
		BUG("%s （%sを片仮名にした）のための入力コードがない",
		    kata, hira);

	return kata_entry->count;
}

int kana_stats(char const *const *argv, int argc)
{
	struct stats_map stats = {0};
	uint64_t hira_total = 0;
	uint64_t kata_total = 0;
	uint64_t total = 0;
	int last_c = 0;
	int last_last_c = 0;
	int i;

	if (argc) {
		xfputs("引数を渡さないでください。\n", err);
		return 92;
	}

	collect_potential_mappings(&stats);

	while (1) {
		int c = xfgetc(in);
		Conv conv = {last_last_c, last_c, c};
		struct stats_entry *entry;

		if (c == -1)
			break;

		last_last_c = last_c;
		last_c = c;

		BSEARCH(entry, stats.el, stats.cnt, strcmp(entry->kana, conv));

		if (!entry)
			continue;

		entry->count++;
		total++;
		switch (codepoint_range(entry->kana))
		{
		case CODEPOINT_RANGE_KATAKANA_SPECIAL:
		case CODEPOINT_RANGE_KATAKANA:
			kata_total++;
			break;
		case CODEPOINT_RANGE_HIRAGANA:
			hira_total++;
			break;
		default:
			BUG("仮名のはずなのに仮名範囲外：%s", entry->kana);
		}
	}

	print_kana_stats_header();

	for (i = 0; i < stats.cnt; i++) {
		uint64_t kata_count, hira_count;
		struct stats_entry *i_entry = &stats.el[i];

		switch (codepoint_range(i_entry->kana)) {
		case CODEPOINT_RANGE_KATAKANA:
			continue;
		case CODEPOINT_RANGE_HIRAGANA:
			hira_count = i_entry->count;
			kata_count = kata_count_of(&stats, i_entry->kana);
			break;
		case CODEPOINT_RANGE_KATAKANA_SPECIAL:
			hira_count = 0;
			kata_count = i_entry->count;
			break;
		default:
			BUG("仮名のはずなのに仮名範囲外：%s", i_entry->kana);
		}

		if (!hira_count && !kata_count)
			continue;

		print_kana_stats_line(i_entry->kana, 2,
				      hira_count, kata_count, total);
	}

	print_kana_stats_line("<合計>", 6, hira_total, kata_total, total);

	DESTROY_ARRAY(stats);
	return 0;
}
