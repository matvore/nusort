#include "kanji_db.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static void print_if_cutoff(char const *k)
{
	struct kanji_entry const *e = kanji_db_lookup(k);
	if (!e)
		xfprintf(err, "見つからない: %s\n", k);
	if (e->cutoff_type)
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
}
