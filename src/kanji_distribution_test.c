#include <inttypes.h>

#include "kanji_distribution.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static int is_in_mapping(
	struct kanji_distribution const *kd, struct kanji_entry const *ke)
{
	int line;
	for (line = 0; line < kd->line_stats_nr; line++) {
		int ke_i;
		for (ke_i = 0; ke_i < kd->line_stats[line].e_nr; ke_i++) {
			if (kd->line_stats[line].e[ke_i] == ke)
				return 1;
		}
	}

	return 0;
}

static void validate_line(struct line_stats const *ls)
{
	int e;
	int share_rsc_key = 0;
	int is_in_mapping = 0;

	for (e = 0; e < ls->e_nr; e++) {
		if (ls->cutoff == ls->e[e])
			is_in_mapping = 1;
		if (ls->cutoff->rsc_sort_key == ls->e[e]->rsc_sort_key)
			share_rsc_key = 1;
	}

	if (share_rsc_key && !is_in_mapping)
		fprintf(out, "%s がマッピングに入っていないため、区切り字"
			      "に使うべきではない\n", ls->cutoff->c);
}

static void show_preferred_next_chars(
	char prior, char const *start, char const *end, int six_is_rh)
{
	struct kanji_distribution kd = {
		.short_shifted_codes = 1,
	};
	struct key_mapping_array preexisting_m = {0};
	int line;

	kd.rsc_range_start = kanji_db_rsc_index(kanji_db_lookup(start));
	kd.rsc_range_end = kanji_db_rsc_index(kanji_db_lookup(end));
	kanji_distribution_set_preexisting_convs(
		&kd, &preexisting_m, 1);
	kanji_distribution_auto_pick_cutoff_exhaustive(
		&kd, prior, 10, six_is_rh);

	fprintf(out, "%c (%s, %s, %d): ", prior, start, end, six_is_rh);
	for (line = 0; line < kd.line_stats_nr; line++)
		fputc(kd.line_stats[line].key_ch, out);
	fputc('\n', out);

	kanji_distribution_destroy(&kd);
	DESTROY_ARRAY(preexisting_m);
}

static void check_using_distinct_cutoff(
	char const *start, char const *end, int max_basic_kanji_per_line)
{
	struct kanji_distribution kd = {
		.short_shifted_codes = 1,
	};
	struct key_mapping_array preexisting_m = {0};
	int i;

	fprintf(out, "--- [%s, %s) --- (basic_per_line <= %d)\n",
		start, end, max_basic_kanji_per_line);

	kd.sort_each_line_by_rsc = 1;
	kd.rsc_range_start = kanji_db_rsc_index(kanji_db_lookup(start));
	kd.rsc_range_end = kanji_db_rsc_index(kanji_db_lookup(end));
	kanji_distribution_set_preexisting_convs(&kd, &preexisting_m, 1);

	kanji_distribution_auto_pick_cutoff_exhaustive(
		&kd, 'j', max_basic_kanji_per_line, 0);
	kanji_distribution_populate(&kd);

	for (i = 0; i < kd.line_stats_nr - 1; i++) {
		struct line_stats const *line = kd.line_stats + i;
		unsigned this_key = line->cutoff->rsc_sort_key;
		unsigned next_key =
			kd.line_stats[i + 1].cutoff->rsc_sort_key;
		int ki;

		if (this_key > next_key) {
			fprintf(out, "\tエラー: (%d) %u > %u\n",
				i, this_key, next_key);
			continue;
		}
		if (this_key < next_key)
			continue;

		fprintf(out, "\t(%d) [%s, %s)\n", i, line->cutoff->c,
			kd.line_stats[i + 1].cutoff->c);

		for (ki = 0; ki < line->e_nr; ki++) {
			if (line->e[ki]->rsc_sort_key != this_key)
				fprintf(out, "\tエラー: %s\n", line->e[ki]->c);
		}
	}

	kanji_distribution_destroy(&kd);
	DESTROY_ARRAY(preexisting_m);
}

#define BASE_KANJI_KEY_COUNT 40

int main(void)
{
	set_test_source_file(__FILE__);

	while (run_test("does_not_use_uncommon_character_if_rsc_key_is_same", "")) {
		int key;

		struct romazi_config romazi_config = {
			.include_kanji_numerals = 1,
			.classic_wo = 0,
			.optimize_keystrokes = 1,
		};
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		struct key_mapping_array romazi_m = {0};

		get_romazi_codes(&romazi_config, &romazi_m);

		kanji_distribution_set_preexisting_convs(&kd, &romazi_m, 1);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);

		for (key = 0; key < kd.line_stats_nr; key++)
			validate_line(kd.line_stats + key);

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(romazi_m);
	}

	while (run_test("does_not_use_uncommon_character_if_rsc_key_is_same_2", "")) {
		int key;

		struct romazi_config romazi_config = {0};
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		struct key_mapping_array romazi_m = {0};

		get_romazi_codes(&romazi_config, &romazi_m);

		kanji_distribution_set_preexisting_convs(&kd, &romazi_m, 1);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);

		for (key = 0; key < kd.line_stats_nr; key++)
			validate_line(kd.line_stats + key);

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(romazi_m);
	}

	while (run_test("does_not_use_uncommon_character_if_rsc_key_is_same_3", "")) {
		int key;

		struct romazi_config romazi_config = {
			.include_kanji_numerals = 1,
			.optimize_keystrokes = 1,
		};
		struct kanji_distribution kd = {
			.busy_right_pinky = 1,
		};
		struct key_mapping_array romazi_m = {0};

		get_romazi_codes(&romazi_config, &romazi_m);

		kanji_distribution_set_preexisting_convs(&kd, &romazi_m, 1);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);

		for (key = 0; key < kd.line_stats_nr; key++)
			validate_line(kd.line_stats + key);

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(romazi_m);
	}

	while (run_test("does_not_use_uncommon_character_if_rsc_key_is_same_4", "")) {
		int key;

		struct romazi_config romazi_config = {
			.include_kanji_numerals = 1,
			.classic_wo = 1,
		};
		struct kanji_distribution kd = {0};
		struct key_mapping_array romazi_m = {0};

		get_romazi_codes(&romazi_config, &romazi_m);

		kanji_distribution_set_preexisting_convs(&kd, &romazi_m, 1);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);

		for (key = 0; key < kd.line_stats_nr; key++)
			validate_line(kd.line_stats + key);

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(romazi_m);
	}

	while (run_test("does_not_use_uncommon_character_for_cutoff_no_kana", "")) {
		int key;

		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		struct key_mapping_array m = {0};

		kanji_distribution_set_preexisting_convs(&kd, &m, 1);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);

		for (key = 0; key < kd.line_stats_nr; key++)
			validate_line(kd.line_stats + key);

		kanji_distribution_destroy(&kd);
	}

	while (run_test("can_generate_distribution_with_only_kanji", "")) {
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		struct key_mapping_array preexisting_m = {0};

		kanji_distribution_set_preexisting_convs(
			&kd, &preexisting_m, 1);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);
		if (kd.total_chars !=
		    BASE_KANJI_KEY_COUNT * (BASE_KANJI_KEY_COUNT + 1))
		        fprintf(out, "%d\n", (int) kd.total_chars);

		kanji_distribution_destroy(&kd);
	}

	while (run_test("can_generate_distribution_with_one_romazi", "")) {
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		struct key_mapping_array preexisting_m = {0};

		append_mapping(&preexisting_m, "ka", "か");
		kanji_distribution_set_preexisting_convs(
			&kd, &preexisting_m, 1);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);
		if (kd.total_chars !=
		    BASE_KANJI_KEY_COUNT * (BASE_KANJI_KEY_COUNT + 1) - 1)
		        fprintf(out, "%d\n", (int) kd.total_chars);

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(preexisting_m);
	}

	while (run_test("can_generate_distribution_with_one_key_romazi", "")) {
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		struct key_mapping_array preexisting_m = {0};

		append_mapping(&preexisting_m, "a", "あ");
		kanji_distribution_set_preexisting_convs(
			&kd, &preexisting_m, 1);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);
		if (kd.total_chars !=
		    BASE_KANJI_KEY_COUNT * (BASE_KANJI_KEY_COUNT + 1)
		    - BASE_KANJI_KEY_COUNT)
			fprintf(out, "%d\n", (int) kd.total_chars);

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(preexisting_m);
	}

	while (run_test("does_not_pass_up_distinct_but_uncommon_kugiri_ji", "")) {
		struct key_mapping_array romazi_m = {0};
		struct romazi_config romazi_config = {
			.optimize_keystrokes = 1,
		};
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		int line;

		get_romazi_codes(&romazi_config, &romazi_m);

		kanji_distribution_set_preexisting_convs(&kd, &romazi_m, 1);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);

		for (line = 0; line < kd.line_stats_nr; line++) {
			struct kanji_entry const *k =
				kd.line_stats[line].cutoff;
			uint16_t rsc_index = kanji_db_rsc_index(k);
			int orig_co_type = k->cutoff_type;

			while (rsc_index > 0) {
				k = kanji_db() +
					kanji_db_rsc_sorted()[--rsc_index];
				if (is_in_mapping(&kd, k))
					break;
				if (k->cutoff_type > orig_co_type)
					fprintf(out, "%s", k->c);
			}
		}

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(romazi_m);
	}

	while (run_test("can_limit_rsc_range_to_auto_pick_cutoff", "")) {
		int line;
		struct key_mapping_array preexisting_m = {0};
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};

		kd.rsc_range_end = kanji_db_rsc_index(kanji_db_lookup("薄"));
		kanji_distribution_set_preexisting_convs(
			&kd, &preexisting_m, 1);
		kanji_distribution_auto_pick_cutoff(&kd);

		for (line = 0; line < kd.line_stats_nr; line++) {
			struct kanji_entry const *cutoff =
				kd.line_stats[line].cutoff;
			int i = kanji_db_rsc_index(cutoff);
			if (i >= 3525) fprintf(out, "%s %d\n", cutoff->c, i);
		}

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(preexisting_m);
	}

	while (run_test("can_limit_rsc_range_to_limit_chars_in_mapping", "")) {
		int line;
		struct key_mapping_array preexisting_m = {0};
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		unsigned end = kanji_db_rsc_index(kanji_db_lookup("笑"));

		kd.rsc_range_end = end;
		kanji_distribution_set_preexisting_convs(
			&kd, &preexisting_m, 1);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);

		for (line = 0; line < kd.line_stats_nr; line++) {
			int c;
			for (c = 0; c < kd.line_stats[line].e_nr; c++) {
				struct kanji_entry const *k =
					kd.line_stats[line].e[c];
				if (kanji_db_rsc_index(k) >= end)
					fprintf(out, "%s", k->c);
			}
		}

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(preexisting_m);
	}

	while (run_test("can_limit_rsc_range_start", "")) {
		int line;
		struct key_mapping_array preexisting_m = {0};
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		unsigned start = kanji_db_rsc_index(kanji_db_lookup("広"));

		kd.rsc_range_start = start;
		kanji_distribution_set_preexisting_convs(
			&kd, &preexisting_m, 1);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);

		if (kanji_db_rsc_index(kd.line_stats[0].cutoff) != start)
			fprintf(out, "最初の区切り字が違います: %s",
				 kd.line_stats[0].cutoff->c);

		for (line = 1; line < kd.line_stats_nr; line++) {
			int c;
			for (c = 0; c < kd.line_stats[line].e_nr; c++) {
				struct kanji_entry const *k =
					kd.line_stats[line].e[c];
				if (kanji_db_rsc_index(k) < start)
					fprintf(out, "%s", k->c);
			}
		}

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(preexisting_m);
	}

	while (run_test("set_rsc_range_not_enough_to_fill_all_codes_sets_cutoff", "")) {
		int line;
		struct key_mapping_array preexisting_m = {0};
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		unsigned start = kanji_db_rsc_index(kanji_db_lookup("忠"));
		unsigned end = kanji_db_rsc_index(kanji_db_lookup("数"));

		kd.rsc_range_start = start;
		kd.rsc_range_end = end;
		kanji_distribution_set_preexisting_convs(
			&kd, &preexisting_m, 1);
		kanji_distribution_auto_pick_cutoff_exhaustive(&kd, 'j', 10, 0);
		kanji_distribution_populate(&kd);

		if (kanji_db_rsc_index(kd.line_stats[0].cutoff) != start)
			fprintf(out, "最初の区切り字が違います: %s",
				 kd.line_stats[0].cutoff->c);

		for (line = 1; line < kd.line_stats_nr; line++) {
			if (!kd.line_stats[line].cutoff) {
				fprintf(out,
					 "区切り字が設定されていない: %d\n",
					 line);
				continue;
			}
			if (distinct_rsc_cmp(kd.line_stats[line - 1].cutoff,
					     kd.line_stats[line].cutoff) >= 0)
				fprintf(out,
					 "区切り字の部首+画数キーが増加して"
					 "いない: %d\n",
					 line);
		}

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(preexisting_m);
	}

	while (run_test("set_rsc_range_allocates_many_kanji", "")) {
		int line;
		struct key_mapping_array preexisting_m = {0};
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		struct {
			struct kanji_entry const **el;
			size_t cnt, alloc;
		} allocated = {0};
		unsigned alloc_i;

		kd.rsc_range_start = kanji_db_rsc_index(kanji_db_lookup("忠"));
		kd.rsc_range_end = kanji_db_rsc_index(kanji_db_lookup("数"));
		kanji_distribution_set_preexisting_convs(
			&kd, &preexisting_m, 1);
		kanji_distribution_auto_pick_cutoff_exhaustive(&kd, 'j', 10, 0);
		kanji_distribution_populate(&kd);

		for (line = 0; line < kd.line_stats_nr; line++) {
			int i;

			for (i = 0; i < kd.line_stats[line].e_nr; i++) {
				struct kanji_entry const *e =
					kd.line_stats[line].e[i];
				if (!e)
					fprintf(out, "!%d / %d\n", line, i);
				GROW_ARRAY_BY(allocated, 1);
				allocated.el[allocated.cnt - 1] = e;
			}
		}

		predictably_sort_by_rsc(allocated.el, allocated.cnt);

		for (alloc_i = 1; alloc_i < allocated.cnt; alloc_i++) {
			if (!strcmp(allocated.el[alloc_i]->c,
				    allocated.el[alloc_i - 1]->c))
				fprintf(out, "err: %u %s\n",
					 alloc_i, allocated.el[alloc_i]->c);
		}

		if (allocated.cnt < 301)
			fprintf(out, "%zu", allocated.cnt);

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(preexisting_m);
		DESTROY_ARRAY(allocated);
	}

	while (run_test("set_rsc_range_allocates_kanji_honoring_cutoffs", "")) {
		int line;
		struct key_mapping_array preexisting_m = {0};
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};

		kd.rsc_range_start = kanji_db_rsc_index(kanji_db_lookup("忠"));
		kd.rsc_range_end = kanji_db_rsc_index(kanji_db_lookup("数"));
		kanji_distribution_set_preexisting_convs(
			&kd, &preexisting_m, 1);
		kanji_distribution_auto_pick_cutoff_exhaustive(&kd, 'j', 10, 0);
		kanji_distribution_populate(&kd);

		for (line = 0; line < kd.line_stats_nr; line++) {
			int i;

			for (i = 0; i < kd.line_stats[line].e_nr; i++) {
				struct kanji_entry const *e =
					kd.line_stats[line].e[i];
				struct kanji_entry const *cutoff =
					kd.line_stats[line].cutoff;
				if (e->rsc_sort_key < cutoff->rsc_sort_key)
					fprintf(out, "%s < %s\n",
						 e->c, cutoff->c);
				if (line == kd.line_stats_nr - 1)
					continue;
				cutoff = kd.line_stats[line + 1].cutoff;
				if (distinct_rsc_cmp(e, cutoff) >= 0)
					fprintf(out, "%s >= %s\n",
						 e->c, cutoff->c);
			}
		}

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(preexisting_m);
	}

	while (run_test("set_rsc_range_not_enough_cutoffs_for_every_key", "")) {
		struct key_mapping_array preexisting_m = {0};
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		int line;

		kd.rsc_range_start = kanji_db_rsc_index(kanji_db_lookup("石"));
		kd.rsc_range_end = kanji_db_rsc_index(kanji_db_lookup("禁"));
		kanji_distribution_set_preexisting_convs(
			&kd, &preexisting_m, 1);
		kanji_distribution_auto_pick_cutoff_exhaustive(&kd, 'j', 10, 0);

		for (line = 1; line < kd.line_stats_nr; line++) {
			struct kanji_entry const *e1 =
				kd.line_stats[line - 1].cutoff;
			struct kanji_entry const *e2 =
				kd.line_stats[line].cutoff;
			if (e1->rsc_sort_key >= e2->rsc_sort_key)
				fprintf(out, "%d: %s %s\n",
					 line, e1->c, e2->c);
		}

		kanji_distribution_populate(&kd);
		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(preexisting_m);
	}

	while (run_test("last_kanji_in_db_in_limited_rsc_range", "1")) {
		struct key_mapping_array preexisting_m = {0};
		struct kanji_distribution kd = {
			.short_shifted_codes = 1,
		};
		int line;
		int e;
		int found = 0;
		unsigned start = kanji_db_rsc_index(kanji_db_lookup("項"));
		unsigned end = kanji_db_nr();

		kd.rsc_range_start = start;
		kd.rsc_range_end = end;
		kanji_distribution_set_preexisting_convs(
			&kd, &preexisting_m, 1);
		kanji_distribution_auto_pick_cutoff_exhaustive(&kd, 'j', 10, 0);
		kanji_distribution_populate(&kd);

		line = kd.line_stats_nr - 1;
		for (e = 0; e < kd.line_stats[line].e_nr; e++) {
			if (kanji_db_rsc_index(kd.line_stats[line].e[e]) ==
			    end - 1)
				found++;
		}

		fprintf(out, "%d", found);

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(preexisting_m);
	}

	while (run_test("choose_easy_to_type_1st_chars_in_exhaustive_pick_cutoff", NULL)) {
		show_preferred_next_chars('.', "閉", "項", 0);
		show_preferred_next_chars('6', "閉", "項", 0);
		show_preferred_next_chars('6', "閉", "項", 1);

		show_preferred_next_chars('f', "一", "亘", 1);
	}

	while (run_test("use_distinct_cutoff_if_possible",
			"--- [授, 日) --- (basic_per_line <= 10)\n"
			"	(0) [授, 排)\n"
			"	(1) [排, 措)\n"
			"	(3) [提, 援)\n"
			"	(7) [撒, 撲)\n"
			"--- [決, 減) --- (basic_per_line <= 10)\n"
			"	(0) [決, 沖)\n"
			"	(2) [法, 泉)\n"
			"	(3) [泉, 泰)\n"
			"	(5) [活, 洲)\n"
			"	(7) [流, 浸)\n"
			"	(9) [済, 淘)\n"
			"	(10) [淘, 清)\n"
			"--- [決, 減) --- (basic_per_line <= 20)\n"
			"	(1) [法, 泰)\n"
			"	(4) [済, 清)\n"
			"--- [超, 通) --- (basic_per_line <= 5)\n"
			"	(2) [践, 跳)\n"
			"	(20) [迷, 送)\n"
			"--- [超, 通) --- (basic_per_line <= 10)\n")) {
		check_using_distinct_cutoff("授", "日", 10);
		check_using_distinct_cutoff("決", "減", 10);
		check_using_distinct_cutoff("決", "減", 20);
		check_using_distinct_cutoff("超", "通", 5);
		check_using_distinct_cutoff("超", "通", 10);
	}

	while (run_test("busy_right_pinky_line_counts", NULL)) {
		struct {
			const char *name;
			struct kanji_distribution kd;
		} *c, cases[] = {
			{
				.name = "brp",
				.kd = {.busy_right_pinky = 1},
			},
			{
				.name = "lbrack",
				.kd = {.allow_left_bracket_key1 = 1},
			},
			{
				.name = "both",
				.kd = {
					.allow_left_bracket_key1 = 1,
					.busy_right_pinky = 1,
				},
			},
			{0},
		};

		for (c = cases; c->name; c++) {
			struct key_mapping_array preexisting_m = {0};
			int line;

			kanji_distribution_set_preexisting_convs(
				&c->kd, &preexisting_m, 1);
			kanji_distribution_auto_pick_cutoff(&c->kd);
			kanji_distribution_populate(&c->kd);

			fprintf(out, "%s: %d\n", c->name, c->kd.total_chars);

			for (line = 0; line < c->kd.line_stats_nr; line++)
				fprintf(out, "%c %"PRIu8"\n",
					c->kd.line_stats[line].key_ch,
					c->kd.line_stats[line].e_nr);

			kanji_distribution_destroy(&c->kd);
		}
	}
}
