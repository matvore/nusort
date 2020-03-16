#include "kanji_db.h"
#include "mapping.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static void print_mapping_array(struct key_mapping_array const *a)
{
	int i;
	for (i = 0; i < a->cnt; i++) {
		print_mapping(a->el + i, out);
		fputc('\n', out);
	}
}

static void check_ergonomic_lt_same_first_key(
	char first_key, char second_a, char second_b, int six_is_rh)
{
	if (!ergonomic_lt_same_first_key(first_key, second_a, second_b,
					 six_is_rh))
		fprintf(out, "失敗: %c%c < %c%c (%d)\n",
			first_key, second_a, first_key, second_b, six_is_rh);

	if (ergonomic_lt_same_first_key(first_key, second_b, second_a,
					six_is_rh))
		fprintf(out, "失敗: %c%c > %c%c (%d)\n",
			first_key, second_b, first_key, second_a, six_is_rh);
}

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("check_conflicts_one_code");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "xxa", "ぁ");
		expect_ok(sort_and_validate_no_conflicts(&a));
		print_mapping_array(&a);

		DESTROY_ARRAY(a);
	}
	end_test("xxa->ぁ\n");

	start_test("check_conflicts_two_codes_ok");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "xxa", "ぁ");
		append_mapping(&a, "ya", "や");
		expect_ok(sort_and_validate_no_conflicts(&a));
		print_mapping_array(&a);

		DESTROY_ARRAY(a);
	}
	end_test("ya->や\nxxa->ぁ\n");

	start_test("check_conflicts_two_codes_equal_orig");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "me", "め");
		append_mapping(&a, "me", "眼");
		expect_fail(sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: me->め と me->眼\n");

	start_test("check_conflicts_two_codes_first_and_last_conflict");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "me", "め");
		append_mapping(&a, "yu", "ゆ");
		append_mapping(&a, "me", "眼");
		expect_fail(sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: me->め と me->眼\n");

	start_test("check_conflicts_two_codes_first_and_2nd_to_last_conflict");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "me", "め");
		append_mapping(&a, "yu", "ゆ");
		append_mapping(&a, "me", "眼");
		append_mapping(&a, "YU", "湯");
		expect_fail(sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: me->め と me->眼\n");

	start_test("check_conflicts_two_codes_2nd_and_2nd_to_last_conflict");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "yu", "ゆ");
		append_mapping(&a, "ka", "か");
		append_mapping(&a, "ka", "蚊");
		append_mapping(&a, "YU", "湯");
		expect_fail(sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: ka->か と ka->蚊\n");

	start_test("check_conflicts_two_codes_2nd_and_3rd_to_last_conflict");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "yu", "ゆ");
		append_mapping(&a, "ka", "か");
		append_mapping(&a, "ka", "蚊");
		append_mapping(&a, "YU", "湯");
		append_mapping(&a, "za", "ざ");
		expect_fail(sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: ka->か と ka->蚊\n");

	start_test("multiple_conflicts");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "ka", "か");
		append_mapping(&a, "za", "ざ");
		append_mapping(&a, "ka", "蚊");
		append_mapping(&a, "YU", "湯");
		append_mapping(&a, "za", "座");
		expect_fail(sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: ka->か と ka->蚊\n"
		 "コード衝突: za->ざ と za->座\n");

	start_test("no_conflicts_3_codes");
	{
		struct key_mapping_array a = {0};
		int i;

		append_mapping(&a, "ka", "か");
		append_mapping(&a, "za", "ざ");
		append_mapping(&a, "yu", "ゆ");
		expect_ok(sort_and_validate_no_conflicts(&a));

		for (i = 0; i < a.cnt; i++) {
			print_mapping(a.el + i, out);
			fputc('\n', out);
		}
		DESTROY_ARRAY(a);
	}
	end_test("ka->か\n"
		 "yu->ゆ\n"
		 "za->ざ\n");

	start_test("conflict_by_matching_prefix");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "k", "ん");
		append_mapping(&a, "ka", "か");
		expect_fail(sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: k->ん と ka->か\n");

	start_test("conflict_by_matching_2_char_prefix");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "ka", "か");
		append_mapping(&a, "kar", "車");
		expect_fail(sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: ka->か と kar->車\n");

	start_test("incomplete_code_is_prefix_1");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "ka", "か");
		append_mapping(&a, "za", "ざ");
		append_mapping(&a, "yu", "ゆ");
		append_mapping(&a, "tye", "ちぇ");
		expect_ok(sort_and_validate_no_conflicts(&a));

		fprintf(out, "k: %d\n", !!incomplete_code_is_prefix(&a, "k"));
		fprintf(out, "f: %d\n", !!incomplete_code_is_prefix(&a, "f"));
		fprintf(out, "y: %d\n", !!incomplete_code_is_prefix(&a, "y"));
		fprintf(out, "ty: %d\n", !!incomplete_code_is_prefix(&a, "ty"));
		fprintf(out, "tt: %d\n", !!incomplete_code_is_prefix(&a, "tt"));

		DESTROY_ARRAY(a);
	}
	end_test("k: 1\n"
		 "f: 0\n"
		 "y: 1\n"
		 "ty: 1\n"
		 "tt: 0\n");

	start_test("incomplete_code_is_prefix_2");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "ga", "が");
		append_mapping(&a, "fa", "ふぁ");
		append_mapping(&a, "mu", "む");
		append_mapping(&a, "tta", "台");
		expect_ok(sort_and_validate_no_conflicts(&a));

		fprintf(out, "k: %d\n", !!incomplete_code_is_prefix(&a, "k"));
		fprintf(out, "f: %d\n", !!incomplete_code_is_prefix(&a, "f"));
		fprintf(out, "y: %d\n", !!incomplete_code_is_prefix(&a, "y"));
		fprintf(out, "ty: %d\n", !!incomplete_code_is_prefix(&a, "ty"));
		fprintf(out, "tt: %d\n", !!incomplete_code_is_prefix(&a, "tt"));

		DESTROY_ARRAY(a);
	}
	end_test("k: 0\n"
		 "f: 1\n"
		 "y: 0\n"
		 "ty: 0\n"
		 "tt: 1\n");

	start_test("sort_and_validate_supports_long_codes");
	{
		struct key_mapping_array a = {0};
		append_mapping(&a, "hwwa", "ふぁ");
		append_mapping(&a, "hwwa", "ふ!");
		expect_fail(sort_and_validate_no_conflicts(&a));
		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: hwwa->ふ! と hwwa->ふぁ\n");

	start_test("lowest_rsc_index_for_codes_with_first_key_singleton_map");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "bb", "三");
		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('b'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("三")))
			fprintf(out, "三: %u\n", lowest);
		DESTROY_ARRAY(a);

		append_mapping(&a, "bb", "四");
		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('b'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("四")))
			fprintf(out, "四: %u\n", lowest);
		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("lowest_rsc_index_for_codes_with_first_key_size_two_map");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "bb", "三");
		append_mapping(&a, "cb", "四");
		expect_ok(sort_and_validate_no_conflicts(&a));

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('b'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("三")))
			fprintf(out, "b: %u\n", lowest);

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('c'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("四")))
			fprintf(out, "c: %u\n", lowest);
		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("lowest_rsc_index_for_codes_with_first_key_two_matching");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "ba", "四");
		append_mapping(&a, "bb", "三");
		expect_ok(sort_and_validate_no_conflicts(&a));

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('b'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("三")))
			fprintf(out, "b: %u\n", lowest);

		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("lowest_rsc_index_for_codes_with_first_key_two_matching_2");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "ba", "三");
		append_mapping(&a, "bb", "四");
		expect_ok(sort_and_validate_no_conflicts(&a));

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('b'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("三")))
			fprintf(out, "b: %u\n", lowest);

		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("lowest_rsc_index_consider_code_where_second_key_is_0");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "b0", "三");
		append_mapping(&a, "bb", "四");
		expect_ok(sort_and_validate_no_conflicts(&a));

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('b'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("三")))
			fprintf(out, "b: %u\n", lowest);

		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("lowest_rsc_index_consider_code_where_second_key_is_comma");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "b,", "三");
		append_mapping(&a, "bb", "四");
		expect_ok(sort_and_validate_no_conflicts(&a));

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('b'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("三")))
			fprintf(out, "b: %u\n", lowest);

		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("lowest_rsc_index_where_first_key_has_shift");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "b0", "三");
		append_mapping(&a, "bb", "四");
		append_mapping(&a, "B", "万");
		expect_ok(sort_and_validate_no_conflicts(&a));

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('b'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("万")))
			fprintf(out, "b: %u\n", lowest);

		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("lowest_rsc_index_where_first_key_has_shift_2");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "A", "ア");
		append_mapping(&a, "b0", "三");
		append_mapping(&a, "bb", "四");
		append_mapping(&a, "B", "万");
		expect_ok(sort_and_validate_no_conflicts(&a));

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('b'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("万")))
			fprintf(out, "b: %u\n", lowest);

		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("lowest_rsc_index_where_first_key_has_shift_3");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "0", "ア");
		append_mapping(&a, "a0", "三");
		append_mapping(&a, "ab", "四");
		append_mapping(&a, "A", "万");
		expect_ok(sort_and_validate_no_conflicts(&a));

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('a'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("万")))
			fprintf(out, "a: %u\n", lowest);

		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("lowest_rsc_index_where_first_key_shifted_is_hiragana");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "0", "ア");
		append_mapping(&a, "a0", "三");
		append_mapping(&a, "ab", "四");
		append_mapping(&a, "A", "を");
		expect_ok(sort_and_validate_no_conflicts(&a));

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('a'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("三")))
			fprintf(out, "a: %u\n", lowest);

		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("lowest_rsc_index_where_first_key_shifted_is_katakana");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "k0", "三");
		append_mapping(&a, "kb", "四");
		append_mapping(&a, "KA", "カ");
		expect_ok(sort_and_validate_no_conflicts(&a));

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('k'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("三")))
			fprintf(out, "k: %u\n", lowest);

		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("lowest_rsc_index_where_first_key_shifted_is_katakana_2");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "g0", "三");
		append_mapping(&a, "gb", "四");
		append_mapping(&a, "GA", "ガ");
		expect_ok(sort_and_validate_no_conflicts(&a));

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('g'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("三")))
			fprintf(out, "g: %u\n", lowest);

		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("lowest_rsc_index_where_first_key_shifted_is_katakana_3");
	{
		struct key_mapping_array a = {0};
		unsigned lowest;

		append_mapping(&a, "n0", "三");
		append_mapping(&a, "nb", "四");
		append_mapping(&a, "NA", "ナ");
		expect_ok(sort_and_validate_no_conflicts(&a));

		lowest = lowest_rsc_index_for_codes_with_first_key(
			&a, char_to_key_index_or_die('n'));
		if (lowest != kanji_db_rsc_index(kanji_db_lookup("三")))
			fprintf(out, "n: %u\n", lowest);

		DESTROY_ARRAY(a);
	}
	end_test("");

	start_test("ergonomic_lt_same_first_key");
	{
		check_ergonomic_lt_same_first_key('a', 'j', 'u', 0);
		check_ergonomic_lt_same_first_key('j', 'j', 'u', 0);

		check_ergonomic_lt_same_first_key('a', '5', '6', 0);
		check_ergonomic_lt_same_first_key('a', '6', '5', 1);

		check_ergonomic_lt_same_first_key('k', '6', '7', 0);
		check_ergonomic_lt_same_first_key('k', '7', '6', 1);

		check_ergonomic_lt_same_first_key('p', 'u', 'p', 0);
		check_ergonomic_lt_same_first_key('p', 'u', 'p', 1);

		check_ergonomic_lt_same_first_key('p', 'j', 'k', 0);
		check_ergonomic_lt_same_first_key('f', 'a', 'r', 0);
	}
	end_test("");
}
