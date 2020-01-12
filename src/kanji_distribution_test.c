#include "kanji_distribution.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

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
		xfprintf(out, "%s がマッピングに入っていないため、区切り字"
			      "に使うべきではない\n", ls->cutoff->c);
}

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("does_not_use_uncommon_character_if_rsc_key_is_same");
	{
		int key;

		struct romazi_config romazi_config = {
			.include_kanji_numerals = 1,
			.classic_wo = 0,
			.optimize_keystrokes = 1,
		};
		struct kanji_distribution kd = {0};
		struct key_mapping_array romazi_m = {0};

		init_romazi(&romazi_config);

		get_romazi_codes(&romazi_m);

		kanji_distribution_set_preexisting_convs(&kd, &romazi_m);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);

		for (key = 0; key < kd.line_stats_nr; key++)
			validate_line(kd.line_stats + key);

		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(romazi_m);
	}
	end_test("");

	start_test("does_not_use_uncommon_character_for_cutoff_no_kana");
	{
		int key;

		struct kanji_distribution kd = {0};
		struct key_mapping_array m = {0};

		kanji_distribution_set_preexisting_convs(&kd, &m);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);

		for (key = 0; key < kd.line_stats_nr; key++)
			validate_line(kd.line_stats + key);

		kanji_distribution_destroy(&kd);
	}
	end_test("");

	start_test("can_generate_distribution_with_only_kanji");
	{
		struct kanji_distribution kd = {0};
		struct key_mapping_array preexisting_m = {0};
	
		kanji_distribution_set_preexisting_convs(&kd, &preexisting_m);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);
		if (kd.total_chars != KANJI_KEY_COUNT * (KANJI_KEY_COUNT + 1))
		        xfprintf(out, "%d\n", (int) kd.total_chars);
	
		kanji_distribution_destroy(&kd);
	}
	end_test("");
	
	start_test("can_generate_distribution_with_one_romazi");
	{
		struct kanji_distribution kd = {0};
		struct key_mapping_array preexisting_m = {0};
	
		append_mapping(&preexisting_m, "ka", "か");
	
		kanji_distribution_set_preexisting_convs(&kd, &preexisting_m);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);
		if (kd.total_chars !=
		    KANJI_KEY_COUNT * (KANJI_KEY_COUNT + 1) - 1)
		        xfprintf(out, "%d\n", (int) kd.total_chars);
	
		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(preexisting_m);
	}
	end_test("");
	
	start_test("can_generate_distribution_with_one_key_romazi");
	{
		struct kanji_distribution kd = {0};
		struct key_mapping_array preexisting_m = {0};
	
		append_mapping(&preexisting_m, "a", "あ");
	
		kanji_distribution_set_preexisting_convs(&kd, &preexisting_m);
		kanji_distribution_auto_pick_cutoff(&kd);
		kanji_distribution_populate(&kd);
		if (kd.total_chars !=
		    KANJI_KEY_COUNT * (KANJI_KEY_COUNT + 1) - KANJI_KEY_COUNT)
			xfprintf(out, "%d\n", (int) kd.total_chars);
	
		kanji_distribution_destroy(&kd);
		DESTROY_ARRAY(preexisting_m);
	}
	end_test("");
}
