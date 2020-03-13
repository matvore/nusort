#include "commands.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

#include <stdlib.h>

static struct {
	const char *name;
	const char *args[40];
	const char *expected;
} test_cases[] = {

{
	"not_enough_args",
	{"一"},
	"32個の区切り漢字を必するけれど、1個が渡された。\n"
	"exit: 1\n"
},

{
	"typical_kugiri_kanji_test",
	{
		"-s",
		"--short-shifted-codes",
		"会", "健", "刀", "匸", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "日", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	0
},

{
	"typical_kugiri_kanji_test_rank_sort",
	{
		"--short-shifted-codes",
		"会", "健", "刀", "匸", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "日", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	0
},

{
	"kanji_not_in_db",
	{
		"--short-shifted-codes",
		"会", "信", "刀", "匸", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "话", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	"[ 话 ] は区切り漢字に指定されているけれど、KANJI配列に含まれていない。\n"
	"exit: 2\n",
},

{
	"kanji_not_valid_kugiri",
	{
		"--short-shifted-codes",
		"会", "信", "刀", "匸", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "俯", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	"[ 俯 ] は区切り漢字として使えません。\n"
	"exit: 3\n",
},

{
	"automaticly_find_kugiri",
	{"--short-shifted-codes"},
	0
},

{
	"typical_kugiri_kanji_with_showing_count_test",
	{
		"--short-shifted-codes",
		"-s", "-n",
		"会", "健", "刀", "匸", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "日", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	0
},

{
	"automaticly_find_kugiri_hide_kanji",
	{"-k", "--short-shifted-codes"},
	0
},

{
	"typical_kugiri_kanji_with_showing_count_and_hide_kanji_test",
	{
		"-k", "-n", "--short-shifted-codes",
		"会", "健", "刀", "匸", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "日", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	0
},

{
	"showing_space_for_potential_cutoff",
	{"-c", "--short-shifted-codes"},
	0
},

{
	"no_hardcoded_kanji_numerals",
	{"--no-kanji-nums", "--short-shifted-codes"},
	0
},

};

int main(void)
{
	size_t test_i;

	set_test_source_file(__FILE__);

	for (test_i = 0; test_i < sizeof(test_cases) / sizeof(*test_cases);
	     test_i++) {
		int arg_count, exit_code;

		start_test(test_cases[test_i].name);

		for (arg_count = 0; test_cases[test_i].args[arg_count];
		     arg_count++) {}
		exit_code = print_last_rank_contained(
			test_cases[test_i].args, arg_count);
		fprintf(out, "exit: %d\n", exit_code);

		if (test_cases[test_i].expected)
			end_test(test_cases[test_i].expected);
		else
			end_test_expected_content_in_file();
	}
	return 0;
}
