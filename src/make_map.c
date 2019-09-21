#include "commands.h"
#include "kanji_distribution.h"
#include "romazi.h"
#include "util.h"

static void get_kanji_codes(struct key_mapping_array *m)
{
	size_t free_code;
	size_t codes_consumed[KANJI_KEY_COUNT] = {0};

	struct kanji_distribution kd = {0};
	struct short_code_array free_kanji_codes = {0};

	kanji_distribution_auto_pick_cutoff(&kd);
	kanji_distribution_populate(&kd);
	get_free_kanji_codes(&free_kanji_codes);

	for (free_code = 0; free_code < free_kanji_codes.cnt; free_code++) {
		char first_key_ch = free_kanji_codes.el[free_code][0];
		ssize_t first_key_i = char_to_key_index_or_die(first_key_ch);
		struct line_stats *line_stats;
		size_t next_code_i;

		/*
		 * 大文字を小文字にかえ、シフト文字を非シフトにかえる。
		 * (例 [: -> ;] [< -> ,])
		 */
		first_key_i %= KANJI_KEY_COUNT;
		first_key_ch = KEY_INDEX_TO_CHAR_MAP[first_key_i];

		BSEARCH(line_stats, kd.line_stats, kd.line_stats_nr,
			char_to_key_index_or_die(line_stats->key_ch) -
				first_key_i);

		if (line_stats == NULL)
			BUG("kanji_distributionで行が見つかりません：%ld",
			    first_key_i);

		next_code_i = codes_consumed[first_key_i]++;
		if (next_code_i >= line_stats->e_nr)
			BUG("kanji_distributionの１打鍵目が'%ld'のコード数が"
			    "足りません", first_key_i);
		GROW_ARRAY_BY(*m, 1);
		memcpy(m->el[m->cnt - 1].orig, free_kanji_codes.el[free_code],
		       2);
		strncpy(m->el[m->cnt - 1].conv, line_stats->e[next_code_i]->c,
			sizeof(m->el->conv));
	}

	DESTROY_ARRAY(free_kanji_codes);
}

int make_map(char const *const *argv, int argc) {
	struct key_mapping_array codes = {0};
	size_t i;

	get_romazi_codes(&codes);
	get_kanji_codes(&codes);

	for (i = 0; i < codes.cnt; i++)
		xfprintf(out, "%s\t%s\n", codes.el[i].orig, codes.el[i].conv);
	DESTROY_ARRAY(codes);
	return 0;
}
