#include <inttypes.h>
#include <stdio.h>

#include "kanji_db.h"
#include "mapping.h"
#include "mapping_util.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static unsigned print_kanji_out_of_rsc_range(
	struct key_mapping_array const *m,
	char const *prefix,
	unsigned start, unsigned end)
{
	unsigned i;
	unsigned unique_added = 0;
	struct {
		Conv *keys;
		void *values;
		size_t bucket_cnt;
	} added_kanji = {0};

	INIT_HASHMAP(added_kanji, 0x10000);

	for (i = 0; i < m->cnt; i++) {
		Conv kanji = {0};
		Conv *found;
		struct kanji_entry const *e;

		if (strncmp(prefix, m->el[i].orig, 2))
			continue;

		strcpy(kanji, m->el[i].conv);
		e = kanji_db_lookup(kanji);

		if (!e)
			DIE(0, "kanji_db で見つからない: %s", kanji);

		if (e->rsc_sort_key < start || e->rsc_sort_key >= end) {
			print_mapping(m->el + i, out);
			fprintf(out, " (%u, %"PRIu16", %u)\n",
				start, e->rsc_sort_key, end);
		}

		FIND_HASHMAP_ENTRY(added_kanji, kanji, found);
		if (!bytes_are_zero(*found, sizeof(*found)))
			continue;

		strcpy(*found, kanji);
		unique_added++;
	}

	DESTROY_HASHMAP(added_kanji);

	return unique_added;
}

static void check_rank_order(
	struct key_mapping_array const *m, char const *a, char const *b)
{
	struct key_mapping const *a_m, *b_m;
	struct kanji_entry const *a_k, *b_k;

	BSEARCH(a_m, m->el, m->cnt, code_cmp(a_m->orig, a));
	if (!a_m) {
		fprintf(out, "コードが見つからない: %s", a);
		return;
	}

	BSEARCH(b_m, m->el, m->cnt, code_cmp(b_m->orig, b));
	if (!b_m) {
		fprintf(out, "コードが見つからない: %s", b);
		return;
	}

	a_k = kanji_db_lookup(a_m->conv);
	b_k = kanji_db_lookup(b_m->conv);

	if (a_k->ranking < b_k->ranking)
		return;

	fprintf(out, "%s の順位 (%"PRIu16") >= %s の順位 (%"PRIu16")\n",
		a_k->c, a_k->ranking, b_k->c, b_k->ranking);
}

static unsigned lowest_rsc_sort_key_with_prefix(
	struct key_mapping_array const *m, char p1)
{
	struct kanji_entry const *e = kanji_from_rsc_index(
		lowest_rsc_index_for_codes_with_first_key(
			m, char_to_key_index_or_die(p1)));

	return e->rsc_sort_key;
}

static int has_prefix_and_len(
	struct key_mapping const *m, char pref, int len)
{
	return strlen(m->orig) == len && m->orig[0] == pref;
}

static long rsc_sort_key_for_any_4_key_code(
	struct key_mapping_array const *m, char first_key, int third_key_index)
{
	Orig find = {first_key, ' ', KEY_INDEX_TO_CHAR_MAP[third_key_index], 1};
	long km_i;
	struct key_mapping const *km;

	BSEARCH_INDEX(km_i, m->cnt, , code_cmp(m->el[km_i].orig, find));
	km = m->el + ~km_i;

	if (strncmp(km->orig, find, 3))
		return -1;

	return kanji_db_lookup(km->conv)->rsc_sort_key;
}

static void test_6_is_rh_setting(int six_is_rh)
{
	struct mapping m = {
		.include_kanji = 1,
		.six_is_rh = six_is_rh,
		.dist = {
			.short_shifted_codes = 1,
		},
	};
	struct key_mapping_array subarr = {0};
	Orig find = {'z', ' ', 1, 1};
	long find_i;
	struct {
		char *keys;
		unsigned *values;
		size_t bucket_cnt;
	} last_rank_by_penult_orig_c = {0};

	INIT_HASHMAP(last_rank_by_penult_orig_c, 256);

	expect_ok(mapping_populate(&m));
	expect_ok(mapping_lazy_populate(&m, "z "));

	BSEARCH_INDEX(find_i, m.arr.cnt, ,
		      code_cmp(m.arr.el[find_i].orig, find));
	find_i = ~find_i;
	while (1) {
		struct key_mapping const *km = m.arr.el + find_i;
		if (strlen(km->orig) != 4 || km->orig[0] != 'z')
			break;
		GROW_ARRAY_BY(subarr, 1);
		subarr.el[subarr.cnt - 1] = *km;
		find_i++;
	}

	QSORT(, subarr.el, subarr.cnt,
	      ergonomic_lt(subarr.el[a].orig + 2,
			   subarr.el[b].orig + 2,
			   six_is_rh));

	for (find_i = 0; find_i < subarr.cnt; find_i++) {
		struct key_mapping const *km = subarr.el + find_i;
		unsigned ranking = kanji_db_lookup(km->conv)->ranking;
		char penult_orig_c = km->orig[2];
		char *found_key;
		unsigned *found_val;
		FIND_HASHMAP_ENTRY(last_rank_by_penult_orig_c,
				   penult_orig_c, found_key);
		if (!*found_key)
			*found_key = penult_orig_c;
		found_val = VALUE_PTR_FOR_HASH_KEY(
			last_rank_by_penult_orig_c, found_key);
		if (*found_val > ranking) {
			fputs("順番が間違っています: ", out);
			print_mapping(km, out);
			fputc('\n', out);
		}
		*found_val = ranking;
	}

	DESTROY_HASHMAP(last_rank_by_penult_orig_c);
	DESTROY_ARRAY(subarr);
	destroy_mapping(&m);
}

static void check_limit_basic_kanji_per_line(int key_index)
{
	struct mapping m = {
		.include_kanji = 1,
		.dist = {
			.short_shifted_codes = 1,
		},
	};
	unsigned basic_kanji_per_first_char[128] = {0};
	unsigned i;
	char prefix[3] = {KEY_INDEX_TO_CHAR_MAP[key_index], ' '};

	expect_ok(mapping_populate(&m));
	expect_ok(mapping_lazy_populate(&m, prefix));

	for (i = 0; i < m.arr.cnt; i++) {
		struct kanji_entry const *k;
		struct key_mapping const *ent = m.arr.el + i;

		if (strlen(ent->orig) != 4)
			continue;

		k = kanji_db_lookup(ent->conv);
		if (k->ranking <= BASIC_KANJI_MAX_PRI) {
			int subs = ent->orig[2];
			if (subs < 0)
				fprintf(out, "(%d) 無効なコード: %s\n",
					key_index, ent->orig);
			else
				basic_kanji_per_first_char[subs]++;
		}
	}

	for (i = 0; i < 128; i++) {
		unsigned basic_count = basic_kanji_per_first_char[i];
		if (basic_count > 10)
			fprintf(out, "(%d) %c: %u > 10\n",
				key_index, i, basic_count);
	}
}

int main(void)
{
	set_test_source_file(__FILE__);

	while (run_test("6_is_rh_6s_is_higher_ranked_kanji_than_6k", "")) {
		struct mapping m = {
			.six_is_rh = 1,
			.include_kanji = 1,
		};

		expect_ok(mapping_populate(&m));
		check_rank_order(&m.arr, "6s", "6k");
		destroy_mapping(&m);
	}

	while (run_test("6_is_lh_6k_is_higher_ranked_kanji_than_6s", "")) {
		struct mapping m = {
			.six_is_rh = 0,
			.include_kanji = 1,
		};

		expect_ok(mapping_populate(&m));
		check_rank_order(&m.arr, "6k", "6s");
		destroy_mapping(&m);
	}

	while (run_test("6_is_lh_6o_is_higher_ranked_kanji_than_6q", "")) {
		struct mapping m = {
			.six_is_rh = 0,
			.include_kanji = 1,
		};

		expect_ok(mapping_populate(&m));
		check_rank_order(&m.arr, "6o", "6q");
		destroy_mapping(&m);
	}

	while (run_test("6_is_lh_k6_is_higher_ranked_kanji_than_ki", "")) {
		struct mapping m = {
			.six_is_rh = 0,
			.include_kanji = 1,
		};

		expect_ok(mapping_populate(&m));
		check_rank_order(&m.arr, "k6", "ki");
		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_adds_more_mappings_if_do_not_exist_yet",
			"")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		unsigned orig_count;

		expect_ok(mapping_populate(&m));
		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "b "));
		if (orig_count >= m.arr.cnt)
			fprintf(out, "マッピングが増えていない: %u >= %zu\n",
				orig_count, m.arr.cnt);

		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "b "));
		if (orig_count < m.arr.cnt)
			fprintf(out, "マッピングが増えている: %u < %zu\n",
				orig_count, m.arr.cnt);

		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "c "));
		if (orig_count >= m.arr.cnt)
			fprintf(out, "マッピングが増えていない: %u >= %zu\n",
				orig_count, m.arr.cnt);

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_does_nothing_if_not_4_char_code_prefix",
			"")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		unsigned orig_count;

		expect_ok(mapping_populate(&m));
		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, " c"));
		if (orig_count != m.arr.cnt)
			fprintf(out, "マッピングが増えている: %u >= %zu\n",
				orig_count, m.arr.cnt);

		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "[ "));
		if (orig_count != m.arr.cnt)
			fprintf(out, "マッピングが増えている: %u >= %zu\n",
				orig_count, m.arr.cnt);

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_considers_shifted_first_key_invalid",
			"")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		unsigned orig_count;

		append_mapping(&m.arr, "KYA", "キャ");

		expect_ok(mapping_populate(&m));
		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "K "));
		if (orig_count != m.arr.cnt)
			fprintf(out, "マッピングが増えている: %u >= %zu\n",
				orig_count, m.arr.cnt);

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_considers_shifted_first_key_invalid_2",
			"")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		unsigned orig_count;

		append_mapping(&m.arr, "!!", "キャ");

		expect_ok(mapping_populate(&m));
		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "! "));
		if (orig_count != m.arr.cnt)
			fprintf(out, "マッピングが増えている: %u >= %zu\n",
				orig_count, m.arr.cnt);

		destroy_mapping(&m);
	}

	while (run_test("at_least_19_rsc_sort_keys_in_1st_line_and_11_in_2nd",
			"")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		unsigned rsc_keys_1, rsc_keys_2;
		expect_ok(mapping_populate(&m));

		rsc_keys_1 = lowest_rsc_sort_key_with_prefix(&m.arr, '2');
		if (rsc_keys_1 < 19)
			fprintf(out, "漢字が欠如している? %u\n", rsc_keys_1);

		rsc_keys_2 = lowest_rsc_sort_key_with_prefix(&m.arr, '3')
			- rsc_keys_1;
		if (rsc_keys_2 < 11)
			fprintf(out, "漢字が欠如している? %u\n", rsc_keys_2);

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_adds_some_possible_missing_kanji", "")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		unsigned orig_count;
		unsigned end_rsc_sort_key;
		unsigned unique_added;
		unsigned expect_to_add = 21;

		expect_ok(mapping_populate(&m));
		end_rsc_sort_key =
			lowest_rsc_sort_key_with_prefix(&m.arr, '2');
		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "1 "));
		if (orig_count >= m.arr.cnt ||
		    m.arr.cnt - orig_count < expect_to_add)
			fprintf(out,
				"マッピングが充分に増えていない: %u -> %zu\n",
				orig_count, m.arr.cnt);

		/* 範囲内の字だけを追加すべき */
		unique_added = print_kanji_out_of_rsc_range(
			&m.arr, "1 ", 0, end_rsc_sort_key);

		if (unique_added < expect_to_add)
			fprintf(out, "%u < %u\n", unique_added, expect_to_add);

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_add_available_kanji_when_prefix_is_2",
			"")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		unsigned orig_count;
		unsigned start_rsc_sort_key, end_rsc_sort_key;
		unsigned unique_added;
		unsigned expect_to_add = 25;

		expect_ok(mapping_populate(&m));

		start_rsc_sort_key =
			lowest_rsc_sort_key_with_prefix(&m.arr, '2');
		end_rsc_sort_key =
			lowest_rsc_sort_key_with_prefix(&m.arr, '3');

		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "2 "));
		if (orig_count >= m.arr.cnt ||
		    m.arr.cnt - orig_count < expect_to_add)
			fprintf(out,
				"マッピングが充分に増えていない: %u -> %zu\n",
				orig_count, m.arr.cnt);

		/* 範囲内の字だけを追加すべき */
		unique_added = print_kanji_out_of_rsc_range(
			&m.arr, "2 ", start_rsc_sort_key, end_rsc_sort_key);

		if (unique_added < expect_to_add)
			fprintf(out, "%u < %u\n", unique_added, expect_to_add);

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_when_prefix_is_2_and_includes_romazi",
			"")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		struct romazi_config rc = {0};
		unsigned orig_count;
		unsigned start_rsc_sort_key, end_rsc_sort_key;
		unsigned unique_added;

		get_romazi_codes(&rc, &m.arr);
		expect_ok(mapping_populate(&m));

		start_rsc_sort_key =
			lowest_rsc_sort_key_with_prefix(&m.arr, '2');
		end_rsc_sort_key =
			lowest_rsc_sort_key_with_prefix(&m.arr, '3');

		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "2 "));
		if (orig_count >= m.arr.cnt || m.arr.cnt - orig_count < 60)
			fprintf(out,
				"マッピングが充分に増えていない: %u -> %zu\n",
				orig_count, m.arr.cnt);

		/* 範囲内の字だけを追加すべき */
		unique_added = print_kanji_out_of_rsc_range(
			&m.arr, "2 ", start_rsc_sort_key, end_rsc_sort_key);

		if (unique_added < 60)
			fprintf(out, "%u < 60\n", unique_added);

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_hardmapped_kanji_do_not_influence_dist",
			"ok\n")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		Orig find_1 = {'1', ' ', 1, 1};
		long i_1;
		unsigned numeral_nine_rsc_i =
			kanji_db_rsc_index(kanji_db_lookup("九"));

		append_mapping(&m.arr, "1b", "索");
		append_mapping(&m.arr, "2b", "九");
		expect_ok(mapping_populate(&m));
		expect_ok(mapping_lazy_populate(&m, "1 "));

		BSEARCH_INDEX(i_1, m.arr.cnt, ,
			      code_cmp(m.arr.el[i_1].orig, find_1));

		i_1 = ~i_1;

		while (has_prefix_and_len(m.arr.el + i_1, '1', 4)) {
			struct kanji_entry const *e =
				kanji_db_lookup(m.arr.el[i_1].conv);
			if (kanji_db_rsc_index(e) > numeral_nine_rsc_i) {
				fputs("ok\n", out);
				break;
			}
			i_1++;
		}

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_hardmapped_kanji_do_not_influence_dist2",
			"ok\n")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		Orig find_1 = {'1', ' ', 1, 1};
		long i_1;
		unsigned numeral_one_rsc_i =
			kanji_db_rsc_index(kanji_db_lookup("一"));

		append_mapping(&m.arr, "2b", "一");
		expect_ok(mapping_populate(&m));
		expect_ok(mapping_lazy_populate(&m, "1 "));

		BSEARCH_INDEX(i_1, m.arr.cnt, ,
			      code_cmp(m.arr.el[i_1].orig, find_1));

		i_1 = ~i_1;

		while (has_prefix_and_len(m.arr.el + i_1, '1', 4)) {
			struct kanji_entry const *e =
				kanji_db_lookup(m.arr.el[i_1].conv);
			if (kanji_db_rsc_index(e) > numeral_one_rsc_i) {
				fputs("ok\n", out);
				break;
			}
			i_1++;
		}

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_hardmapped_kanji_do_not_influence_dist3",
			"ok\n")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		Orig find_1 = {'1', ' ', 1, 1};
		long i_1;
		unsigned numeral_one_rsc_i =
			kanji_db_rsc_index(kanji_db_lookup("一"));

		append_mapping(&m.arr, "@", "一");
		expect_ok(mapping_populate(&m));
		expect_ok(mapping_lazy_populate(&m, "1 "));

		BSEARCH_INDEX(i_1, m.arr.cnt, ,
			      code_cmp(m.arr.el[i_1].orig, find_1));

		i_1 = ~i_1;

		while (has_prefix_and_len(m.arr.el + i_1, '1', 4)) {
			struct kanji_entry const *e =
				kanji_db_lookup(m.arr.el[i_1].conv);
			if (kanji_db_rsc_index(e) > numeral_one_rsc_i) {
				fputs("ok\n", out);
				break;
			}
			i_1++;
		}

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_hardmapped_kanji_do_not_influence_dist4",
			"ok\n")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		struct romazi_config rc = {
			.include_kanji_numerals = 1,
		};
		Orig find_7 = {'7', ' ', 1, 1};
		long i_7;
		unsigned numeral_hachi_rsc_i =
			kanji_db_rsc_index(kanji_db_lookup("八"));

		get_romazi_codes(&rc, &m.arr);
		expect_ok(mapping_populate(&m));
		expect_ok(mapping_lazy_populate(&m, "7 "));

		BSEARCH_INDEX(i_7, m.arr.cnt, ,
			      code_cmp(m.arr.el[i_7].orig, find_7));

		i_7 = ~i_7;

		while (has_prefix_and_len(m.arr.el + i_7, '7', 4)) {
			struct kanji_entry const *e =
				kanji_db_lookup(m.arr.el[i_7].conv);
			if (kanji_db_rsc_index(e) > numeral_hachi_rsc_i) {
				fputs("ok\n", out);
				break;
			}
			i_7++;
		}

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_may_not_include_kanji_if_not_configured",
			"1")) {
		struct mapping m = {
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		struct romazi_config rc = {
			.include_kanji_numerals = 1,
		};
		unsigned orig_cnt;

		get_romazi_codes(&rc, &m.arr);
		expect_ok(mapping_populate(&m));
		orig_cnt = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "7 "));
		fprintf(out, "%d", orig_cnt == m.arr.cnt);
		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_next_key_index_is_single_key_code",
			"")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		struct romazi_config rc = {0};
		unsigned start_rsc_sort_key, end_rsc_sort_key;
		unsigned orig_count;
		unsigned unique_added;
		unsigned expect_to_add = 45;

		get_romazi_codes(&rc, &m.arr);
		expect_ok(mapping_populate(&m));

		start_rsc_sort_key =
			lowest_rsc_sort_key_with_prefix(&m.arr, 'y');
		end_rsc_sort_key =
			lowest_rsc_sort_key_with_prefix(&m.arr, 'p');

		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "y "));

		if (orig_count >= m.arr.cnt ||
		    m.arr.cnt - orig_count < expect_to_add)
			fprintf(out,
				"マッピングが充分に増えていない: %u -> %zu\n",
				orig_count, m.arr.cnt);

		/* 範囲内の字だけを追加すべき */
		unique_added = print_kanji_out_of_rsc_range(
			&m.arr, "y ", start_rsc_sort_key, end_rsc_sort_key);
		if (unique_added < expect_to_add)
			fprintf(out, "%u < %u\n", unique_added, expect_to_add);

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_last_key_prefix", "")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		unsigned start_rsc_sort_key, end_rsc_sort_key;
		unsigned orig_count;
		unsigned unique_added;

		expect_ok(mapping_populate(&m));

		start_rsc_sort_key =
			lowest_rsc_sort_key_with_prefix(&m.arr, '/');
		end_rsc_sort_key =
			kanji_from_rsc_index(kanji_db_nr() - 1)->rsc_sort_key +
			1;

		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "/ "));

		if (orig_count >= m.arr.cnt || m.arr.cnt - orig_count < 60)
			fprintf(out,
				"マッピングが充分に増えていない: %u -> %zu\n",
				orig_count, m.arr.cnt);

		/* 範囲内の字だけを追加すべき */
		unique_added = print_kanji_out_of_rsc_range(
			&m.arr, "/ ", start_rsc_sort_key, end_rsc_sort_key);
		if (unique_added < 60)
			fprintf(out, "%u < 60\n", unique_added);

		destroy_mapping(&m);
	}

	while (run_test("lazy_populate_last_key_prefix_includes_one_key_romazi",
			"")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		unsigned start_rsc_sort_key, end_rsc_sort_key;
		unsigned orig_count;
		unsigned unique_added;

		append_mapping(&m.arr, ".", "。");
		append_mapping(&m.arr, ">", "＞");
		expect_ok(mapping_populate(&m));

		start_rsc_sort_key =
			lowest_rsc_sort_key_with_prefix(&m.arr, '/');
		end_rsc_sort_key =
			kanji_from_rsc_index(kanji_db_nr() - 1)->rsc_sort_key +
			1;

		orig_count = m.arr.cnt;
		expect_ok(mapping_lazy_populate(&m, "/ "));

		if (orig_count >= m.arr.cnt || m.arr.cnt - orig_count < 60)
			fprintf(out,
				"マッピングが充分に増えていない: %u -> %zu\n",
				orig_count, m.arr.cnt);

		/* 範囲内の字だけを追加すべき */
		unique_added = print_kanji_out_of_rsc_range(
			&m.arr, "/ ", start_rsc_sort_key, end_rsc_sort_key);
		if (unique_added < 60)
			fprintf(out, "%u < 60\n", unique_added);

		destroy_mapping(&m);
	}

	while (run_test("lazy_mapping_increasing_rsc_with_increasing_key_index",
			"")) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.short_shifted_codes = 1,
			},
		};
		int third_key;
		long last_key = -1;

		expect_ok(mapping_populate(&m));
		expect_ok(mapping_lazy_populate(&m, "j "));

		for (third_key = 0; third_key < KANJI_KEY_COUNT; third_key++) {
			long key = rsc_sort_key_for_any_4_key_code(
				&m.arr, 'j', third_key);
			if (key < 0)
				continue;
			if (last_key >= key)
				fprintf(out, "%ld -> %ld(%c)\n", last_key, key,
					KEY_INDEX_TO_CHAR_MAP[third_key]);
			last_key = key;
		}

		destroy_mapping(&m);
	}

	while (run_test("lazy_mapping_honors_six_is_lh_setting", ""))
		test_6_is_rh_setting(0);

	while (run_test("lazy_mapping_honors_six_is_rh_setting", ""))
		test_6_is_rh_setting(1);

	while (run_test("lazy_mapping_limit_basic_kanji_per_line", "")) {
		int i;
		for (i = 0; i < 40; i++)
			check_limit_basic_kanji_per_line(i);
	}

	while (run_test("busy_right_pinky", NULL)) {
		struct mapping m = {
			.include_kanji = 1,
			.dist = {
				.busy_right_pinky = 1,
			},
		};
		int i;

		expect_ok(mapping_populate(&m));
		for (i = 0; i < m.arr.cnt; i++) {
			if (strchr("-=[]'", m.arr.el[i].orig[1]))
				fprintf(out, "%s\n", m.arr.el[i].orig);
		}

		destroy_mapping(&m);
	}
}
