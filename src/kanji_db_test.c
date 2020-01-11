#include "kanji_db.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static int cutoff_type(char const *k)
{
	struct kanji_entry const *e = kanji_db_lookup(k);
	if (e)
		return e->cutoff_type;
	xfprintf(err, "見つからない: %s\n", k);
	return 0;
}

static void print_if_cutoff(char const *k)
{
	if (cutoff_type(k))
		xfprintf(out, "%s\n", k);
}

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("lookup_non_kanji");
	{
		if (kanji_db_lookup("?"))
			xfputs("漢字ではない文字がデータベースにある", out);
	}
	end_test("");

	start_test("lookup_kanji");
	{
		struct kanji_entry const *e = kanji_db_lookup("格");
		if (e)
			xfputs(e->c, out);
	}
	end_test("格");

	start_test("rittou_hen_7_stroke_cutoff");
	{
		print_if_cutoff("剃");
		print_if_cutoff("則");
		print_if_cutoff("削");
		print_if_cutoff("剋");
		print_if_cutoff("剌");
		print_if_cutoff("前");
	}
	end_test("前\n");

	start_test("tree_rad_with_11_resid_strokes_bad_cutoffs");
	{
		print_if_cutoff("槻");
		print_if_cutoff("槽");
		print_if_cutoff("槿");
		print_if_cutoff("樂");
		print_if_cutoff("樅");
		print_if_cutoff("樋");
		print_if_cutoff("樓");
		print_if_cutoff("樗");
		print_if_cutoff("樟");
		print_if_cutoff("樣");
	}
	end_test("");

	start_test("tree_rad_with_11_resid_strokes_good_cutoffs");
	{
		int cutoff_count = 0;
		if (cutoff_type("標"))
			cutoff_count++;
		if (cutoff_type("権"))
			cutoff_count++;
		if (cutoff_type("横"))
			cutoff_count++;
		xfprintf(out, "%d", cutoff_count);
	}
	end_test("1");

	start_test("include_sude_no_tsukuri_and_munyou");
	{
		struct kanji_entry const *e = kanji_db_lookup("无");
		if (!e)
			xfprintf(out, "ない\n");
		e = kanji_db_lookup("旡");
		if (!e)
			xfprintf(out, "ない\n");
	}
	end_test("");

	start_test("sude_no_tsukuri_and_munyou_cutoff_type");
	{
		xfprintf(out, "%d\n", cutoff_type("无"));
		xfprintf(out, "%d\n", cutoff_type("旡"));
	}
	end_test("2\n0\n");

	start_test("kunshou_no_shou_has_correct_rad");
	{
		struct kanji_entry const *a = kanji_db_lookup("立");
		struct kanji_entry const *b = kanji_db_lookup("章");
		struct kanji_entry const *c = kanji_db_lookup("竹");

		if (!a || !b || !c)
			xfputs("ない\n", out);
		else if (a->rsc_sort_key < b->rsc_sort_key &&
			 b->rsc_sort_key < c->rsc_sort_key)
			xfputs("ok\n", out);
	}
	end_test("ok\n");

	start_test("kunshou_no_shou_is_after_simplified_dragon_with_diff_rsc");
	{
		struct kanji_entry const *a = kanji_db_lookup("竜");
		struct kanji_entry const *b = kanji_db_lookup("章");

		if (!a || !b)
			xfputs("ない\n", out);
		else if (a->rsc_sort_key < b->rsc_sort_key)
			xfputs("ok\n", out);
	}
	end_test("ok\n");

	start_test("u_7adf_is_same_rsc_as_kunshou_no_shou");
	{
		struct kanji_entry const *a = kanji_db_lookup("竟");
		struct kanji_entry const *b = kanji_db_lookup("章");

		if (!a || !b)
			xfputs("ない\n", out);
		else if (a->rsc_sort_key == b->rsc_sort_key)
			xfputs("ok\n", out);
	}
	end_test("ok\n");
}
