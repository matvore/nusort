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

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("check_conflicts_one_code");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "xxa", "ぁ");
		fprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));
		print_mapping_array(&a);

		DESTROY_ARRAY(a);
	}
	end_test("1\nxxa->ぁ\n");

	start_test("check_conflicts_two_codes_ok");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "xxa", "ぁ");
		append_mapping(&a, "ya", "や");
		fprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));
		print_mapping_array(&a);

		DESTROY_ARRAY(a);
	}
	end_test("1\nya->や\nxxa->ぁ\n");

	start_test("check_conflicts_two_codes_equal_orig");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "me", "め");
		append_mapping(&a, "me", "眼");
		fprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: me->め と me->眼\n0\n");

	start_test("check_conflicts_two_codes_first_and_last_conflict");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "me", "め");
		append_mapping(&a, "yu", "ゆ");
		append_mapping(&a, "me", "眼");
		fprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

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
		fprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

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
		fprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

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
		fprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

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
		fprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

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
		fprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		for (i = 0; i < a.cnt; i++) {
			print_mapping(a.el + i, out);
			fputc('\n', out);
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
		fprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: k->ん と ka->か\n0\n");

	start_test("conflict_by_matching_2_char_prefix");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "ka", "か");
		append_mapping(&a, "kar", "車");
		fprintf(out, "%d\n", sort_and_validate_no_conflicts(&a));

		DESTROY_ARRAY(a);
	}
	end_test("コード衝突: ka->か と kar->車\n0\n");

	start_test("incomplete_code_is_prefix_1");
	{
		struct key_mapping_array a = {0};

		append_mapping(&a, "ka", "か");
		append_mapping(&a, "za", "ざ");
		append_mapping(&a, "yu", "ゆ");
		append_mapping(&a, "tye", "ちぇ");
		fprintf(out, "ok: %d\n", sort_and_validate_no_conflicts(&a));

		fprintf(out, "k: %d\n", incomplete_code_is_prefix(&a, "k"));
		fprintf(out, "f: %d\n", incomplete_code_is_prefix(&a, "f"));
		fprintf(out, "y: %d\n", incomplete_code_is_prefix(&a, "y"));
		fprintf(out, "ty: %d\n", incomplete_code_is_prefix(&a, "ty"));
		fprintf(out, "tt: %d\n", incomplete_code_is_prefix(&a, "tt"));

		DESTROY_ARRAY(a);
	}
	end_test("ok: 1\n"
		 "k: 1\n"
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
		fprintf(out, "ok: %d\n", sort_and_validate_no_conflicts(&a));

		fprintf(out, "k: %d\n", incomplete_code_is_prefix(&a, "k"));
		fprintf(out, "f: %d\n", incomplete_code_is_prefix(&a, "f"));
		fprintf(out, "y: %d\n", incomplete_code_is_prefix(&a, "y"));
		fprintf(out, "ty: %d\n", incomplete_code_is_prefix(&a, "ty"));
		fprintf(out, "tt: %d\n", incomplete_code_is_prefix(&a, "tt"));

		DESTROY_ARRAY(a);
	}
	end_test("ok: 1\n"
		 "k: 0\n"
		 "f: 1\n"
		 "y: 0\n"
		 "ty: 0\n"
		 "tt: 1\n");
}
