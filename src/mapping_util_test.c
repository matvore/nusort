#include "mapping.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static void print_mapping_array(struct key_mapping_array const *a)
{
	int i;
	for (i = 0; i < a->cnt; i++) {
		print_mapping(a->el + i, out);
		xfputc('\n', out);
	}
}

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("check_conflicts_one_code");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "xxa", "ぁ");
		xfprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));
		print_mapping_array(&a);

		DESTROY_ARRAY(a);
	}
	end_test("1\nxxa->ぁ\n");

	start_test("check_conflicts_two_codes_ok");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "xxa", "ぁ");
		append_mapping(&a, "ya", "や");
		xfprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));
		print_mapping_array(&a);

		DESTROY_ARRAY(a);
	}
	end_test("1\nya->や\nxxa->ぁ\n");

	start_test("check_conflicts_two_codes_equal_orig");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "me", "め");
		append_mapping(&a, "me", "眼");
		xfprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: me->め と me->眼\n0\n");

	start_test("check_conflicts_two_codes_first_and_last_conflict");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "me", "め");
		append_mapping(&a, "yu", "ゆ");
		append_mapping(&a, "me", "眼");
		xfprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: me->め と me->眼\n0\n");

	start_test("check_conflicts_two_codes_first_and_2nd_to_last_conflict");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "me", "め");
		append_mapping(&a, "yu", "ゆ");
		append_mapping(&a, "me", "眼");
		append_mapping(&a, "YU", "湯");
		xfprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: me->め と me->眼\n0\n");

	start_test("check_conflicts_two_codes_2nd_and_2nd_to_last_conflict");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "yu", "ゆ");
		append_mapping(&a, "ka", "か");
		append_mapping(&a, "ka", "蚊");
		append_mapping(&a, "YU", "湯");
		xfprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: ka->か と ka->蚊\n0\n");

	start_test("check_conflicts_two_codes_2nd_and_3rd_to_last_conflict");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "yu", "ゆ");
		append_mapping(&a, "ka", "か");
		append_mapping(&a, "ka", "蚊");
		append_mapping(&a, "YU", "湯");
		append_mapping(&a, "za", "ざ");
		xfprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: ka->か と ka->蚊\n0\n");

	start_test("multiple_conflicts");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "ka", "か");
		append_mapping(&a, "za", "ざ");
		append_mapping(&a, "ka", "蚊");
		append_mapping(&a, "YU", "湯");
		append_mapping(&a, "za", "座");
		xfprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: ka->か と ka->蚊\n"
		 "コード衝突: za->ざ と za->座\n"
		 "0\n");

	start_test("no_conflicts_3_codes");
	{
		struct key_mapping_array a = {0};
		int i;

		append_mapping(&a, "ka", "か");
		append_mapping(&a, "za", "ざ");
		append_mapping(&a, "yu", "ゆ");
		xfprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		for (i = 0; i < a.cnt; i++) {
			print_mapping(a.el + i, out);
			xfputc('\n', out);
		}
		DESTROY_ARRAY(a);
	}
	end_test("1\n"
		 "ka->か\n"
		 "yu->ゆ\n"
		 "za->ざ\n");

	start_test("conflict_by_matching_prefix");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "k", "ん");
		append_mapping(&a, "ka", "か");
		xfprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: k->ん と ka->か\n0\n");

	start_test("conflict_by_matching_2_char_prefix");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "ka", "か");
		append_mapping(&a, "kar", "車");
		xfprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: ka->か と kar->車\n0\n");
}
