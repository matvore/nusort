#include <string.h>

#include "kanji_distribution.h"
#include "mapping.h"
#include "mapping_util.h"
#include "residual_stroke_count.h"
#include "romazi.h"
#include "streams.h"
#include "util.h"

static struct line_stats const *line_stats_for_first_key_i(
	struct kanji_distribution const *dist, int first_key_i)
{
	struct line_stats const *l;

	BSEARCH(l, dist->line_stats, dist->line_stats_nr,
		char_to_key_index_or_die(l->key_ch) - first_key_i);

	return l;
}

static void add_code(
	struct key_mapping_array *mapping,
	char const *prefix,
	char const *suffix,
	char const *kanji)
{
	Orig orig = {0};
	int prefix_len = strlen(prefix);
	memcpy(orig, prefix, prefix_len);
	orig[prefix_len] = suffix[0];
	orig[prefix_len + 1] = suffix[1];

	append_mapping(mapping, orig, kanji);
}

// https://ja.wikipedia.org/wiki/JIS漢字コード
#define KANJI_IN_JIS_X_0213 (11233 - 1183)

static void get_kanji_codes(
	char const *orig_prefix,
	unsigned top_level_total_chars,
	struct kanji_distribution *dist,
	struct key_mapping_array *arr,
	int six_is_rh)
{
	size_t free_code;
	size_t codes_consumed[KANJI_KEY_COUNT] = {0};

	kanji_distribution_set_preexisting_convs(dist, arr, !orig_prefix[0]);
	if (orig_prefix[0]) {
		unsigned max_basic_kanji_per_line =
			(long) (KANJI_KEY_COUNT - 10) *
			(BASIC_KANJI_MAX_PRI - top_level_total_chars) /
			(KANJI_IN_JIS_X_0213 - top_level_total_chars);
		kanji_distribution_auto_pick_cutoff_exhaustive(
			dist, orig_prefix[0], max_basic_kanji_per_line,
			six_is_rh);
	} else {
		kanji_distribution_auto_pick_cutoff(dist);
	}
	kanji_distribution_populate(dist);

	/*
	 * この時点で、dist->unused_kanji_origsが既に１打鍵目を共有する
	 * コード群ごとに使用頻度に基づいて並べ替え済みです。
	 * 更にコードを打ちやすい順に並べ替えれば、以降の論理の流れでよく使う漢
	 * 字は打ちやすいコードに割り当てられます。
	 */
	QSORT(, dist->unused_kanji_origs.el, dist->unused_kanji_origs.cnt,
	      ergonomic_lt(dist->unused_kanji_origs.el[a],
			   dist->unused_kanji_origs.el[b], six_is_rh));

	for (free_code = 0; free_code < dist->unused_kanji_origs.cnt;
	     free_code++) {
		char first_key_ch = dist->unused_kanji_origs.el[free_code][0];
		int first_key_i = char_to_key_index_or_die(first_key_ch);
		struct line_stats const *line_stats;
		size_t next_code_i;

		/*
		 * 大文字を小文字にかえ、シフト文字を非シフトにかえる。
		 * (例 [: -> ;] [< -> ,])
		 */
		first_key_i %= KANJI_KEY_COUNT;
		first_key_ch = KEY_INDEX_TO_CHAR_MAP[first_key_i];

		line_stats = line_stats_for_first_key_i(dist, first_key_i);
		if (!line_stats)
			continue;

		next_code_i = codes_consumed[first_key_i]++;
		if (next_code_i < line_stats->e_nr)
			add_code(arr, orig_prefix,
				 dist->unused_kanji_origs.el[free_code],
				 line_stats->e[next_code_i]->c);
	}
}

void init_mapping_config_for_cli_flags(struct mapping *m)
{
	if (!bytes_are_zero(m, sizeof(*m)))
		DIE(0, "mapping_config not initialized to zero bytes");
	m->include_kanji = 1;
	m->resid_sc_3rd_key = 1;
}

int parse_mapping_flags(int *argc, char const *const **argv, struct mapping *m)
{
	if (!strcmp((*argv)[0], "-s")) {
		m->six_is_rh = 1;
		(*argv)++;
		(*argc)--;
		return 1;
	}
	if (!strcmp((*argv)[0], "--no-kanji")) {
		m->include_kanji = 0;
		(*argv)++;
		(*argc)--;
		return 1;
	}
	if (!strcmp((*argv)[0], "--recurs-ksort")) {
		m->resid_sc_3rd_key = 0;
		(*argv)++;
		(*argc)--;
		return 1;
	}
	return parse_kanji_distribution_flags(argc, argv, &m->dist);
}

static void add_cutoffs(struct mapping *m, char const *pref,
			struct kanji_distribution const *dist)
{
	Orig key = {0};
	int pref_len = strlen(pref);
	int i;
	Orig *found_key;
	uint16_t *value;

	memcpy(key, pref, pref_len);

	for (i = 0; i < dist->line_stats_nr; i++) {

		key[pref_len] = dist->line_stats[i].key_ch;

		FIND_HASHMAP_ENTRY(m->cutoff_map, key, found_key);

		memcpy(*found_key, key, sizeof(key));
		value = VALUE_PTR_FOR_HASH_KEY(m->cutoff_map, found_key);
		*value = dist->line_stats[i].cutoff - kanji_db();
	}

	key[pref_len] = 1;
	FIND_HASHMAP_ENTRY(m->cutoff_map, key, found_key);
	memcpy(*found_key, key, sizeof(key));
	value = VALUE_PTR_FOR_HASH_KEY(m->cutoff_map, found_key);
	if (dist->rsc_range_end == kanji_db_nr())
		*value = kanji_db_nr();
	else
		*value = kanji_from_rsc_index(dist->rsc_range_end) - kanji_db();
}

int mapping_populate(struct mapping *m)
{
	INIT_HASHMAP(m->cutoff_map,
		     KANJI_KEY_COUNT * (KANJI_KEY_COUNT + 1) * 2);

	if (m->include_kanji) {
		get_kanji_codes("", 0, &m->dist, &m->arr, m->six_is_rh);
		add_cutoffs(m, "", &m->dist);
	}

	return sort_and_validate_no_conflicts(&m->arr);
}

static void lazy_pop_rad_sc(
	struct mapping *m,
	char const *key_prefix,
	unsigned rsc_ndx_lo, unsigned rsc_ndx_hi)
{
	struct kanji_distribution dist = {
		.rsc_range_start = rsc_ndx_lo,
		.rsc_range_end = rsc_ndx_hi,
	};

	get_kanji_codes(
		key_prefix, m->dist.total_chars, &dist, &m->arr, m->six_is_rh);

	add_cutoffs(m, key_prefix, &dist);

	kanji_distribution_destroy(&dist);
}

#define RESID_SC_CELLS 16
#define RESID_SC_CELL_CHARS "qwertyuiopasdfgh"
#define RESID_SC_XCEL_CHARS "1234567890zxcvbn"

static void lazy_pop_sc(
	struct mapping *m, char first_key,
	unsigned rsc_ndx_lo, unsigned rsc_ndx_hi)
{
	unsigned ki, usd;
	struct {
		uint8_t used;
		char c[MAPPABLE_CHAR_COUNT / 2];
		char x[MAPPABLE_CHAR_COUNT / 2];
	} ergo_sorted_4th[RESID_SC_CELLS], *fourth;
	int sc;
	char suff[3], pref[3];
	struct kanji_entries avail = {0};

	add_available_kanji(&avail, &m->arr, rsc_ndx_lo, rsc_ndx_hi);
	QSORT(, avail.el, avail.cnt,
	      avail.el[a]->ranking < avail.el[b]->ranking);

	for (sc = 0; sc < RESID_SC_CELLS; sc++) {
		fourth = ergo_sorted_4th + sc;
		fourth->used = 0;
		memcpy(fourth->c, KEY_INDEX_TO_CHAR_MAP, sizeof(fourth->c));
		memcpy(fourth->x, KEY_INDEX_TO_CHAR_MAP, sizeof(fourth->x));
		QSORT(, fourth->c, sizeof(fourth->c),
		      ergonomic_lt_same_first_key(RESID_SC_CELL_CHARS[sc],
						  fourth->c[a], fourth->c[b],
						  m->six_is_rh));
		QSORT(, fourth->x, sizeof(fourth->x),
		      ergonomic_lt_same_first_key(RESID_SC_XCEL_CHARS[sc],
						  fourth->x[a], fourth->x[b],
						  m->six_is_rh));
	}

	pref[0] = first_key;
	pref[2] = 0;
	suff[2] = 0;
	for (ki = 0; ki < avail.cnt; ki++) {
		sc = residual_stroke_count(avail.el[ki]);
		if (sc > 0) sc--;
		if (sc >= RESID_SC_CELLS) sc = RESID_SC_CELLS-1;

		fourth = ergo_sorted_4th + sc;

		usd = fourth->used++;
		pref[1] = ' ';
		if (usd >= MAPPABLE_CHAR_COUNT) {
			usd -= MAPPABLE_CHAR_COUNT;
			pref[1] = '\\';
		}

		if (usd < sizeof(fourth->c)) {
			suff[0] = RESID_SC_CELL_CHARS[sc];
			suff[1] = fourth->c[usd];
		} else {
			usd -= sizeof(fourth->c);
			if (usd >= sizeof(fourth->x)) {
				fprintf(err, "first_key='%c'\n", first_key);
				fprintf(err, "sc=%d\n", sc);
				DIE(0, "入力コードがたりない");
			}
			suff[0] = RESID_SC_XCEL_CHARS[sc];
			suff[1] = fourth->x[usd];
		}
		add_code(&m->arr, pref, suff, avail.el[ki]->c);
	}

	DESTROY_ARRAY(avail);
}

int mapping_lazy_populate(struct mapping *m, char const *key_prefix)
{
	int key_index = char_to_key_index(key_prefix[0]);
	struct line_stats const *line_a;
	unsigned rsc_ndx_lo, rsc_ndx_hi;

	if (!m->include_kanji)
		return 0;
	if (key_index == -1 || key_index >= KANJI_KEY_COUNT)
		return 0;
	if (key_prefix[1] != ' ' && key_prefix[1] != '\\')
		return 0;
	if (incomplete_code_is_prefix_for_code_len(&m->arr, key_prefix, 4))
		return 0;

	if (strlen(key_prefix) != 2)
		DIE(0, "接頭辞の長さが2ではない: %s", key_prefix);

	line_a = line_stats_for_first_key_i(&m->dist, key_index);
	if (!line_a) return 0;

	rsc_ndx_lo = kanji_db_rsc_index(line_a->cutoff);
	if (line_a - m->dist.line_stats == m->dist.line_stats_nr - 1)
		rsc_ndx_hi = kanji_db_nr();
	else
		rsc_ndx_hi = kanji_db_rsc_index((line_a + 1)->cutoff);

	if (m->resid_sc_3rd_key)
		lazy_pop_sc(m, key_prefix[0], rsc_ndx_lo, rsc_ndx_hi);
	else if (key_prefix[1] == '\\')
		return 0;
	else
		lazy_pop_rad_sc(m, key_prefix, rsc_ndx_lo, rsc_ndx_hi);

	return sort_and_validate_no_conflicts(&m->arr);
}

void destroy_mapping(struct mapping *m)
{
	DESTROY_ARRAY(m->arr);
	DESTROY_HASHMAP(m->cutoff_map);
	kanji_distribution_destroy(&m->dist);
	memset(m, 0, sizeof(*m));
}
