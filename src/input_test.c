#include "commands.h"
#include "input_impl.h"
#include "mapping.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

enum mapping_type {
	ERGONOMIC_SORT,
	HAS_XYZ,
};

static struct {
	char const *name;
	char const *in;
	enum mapping_type mapping_type;
} test_cases[] = {

{"first_key_k", "kj", ERGONOMIC_SORT},

{"impossible_code", "kJ", ERGONOMIC_SORT},

{"possible_code_requires_two_more_chars", "x!", HAS_XYZ},

{"long_conv_strs", "tya" "HWI" "DWO" "WHO" "YE", ERGONOMIC_SORT},

};

static void set_mapping(struct mapping *m, enum mapping_type type)
{
	switch (type) {
		case ERGONOMIC_SORT:
			m->ergonomic_sort = 1;
			if (!mapping_populate(m))
				DIE(0, "mapping_populate");
			break;
		case HAS_XYZ:
			GROW_ARRAY_BY(m->codes, 1);
			strcpy(m->codes.el[0].orig, "xyz");
			strcpy(m->codes.el[0].conv, "あ");
			break;
		default:
			BUG("無効な mapping_type");
	}
}

int main(void)
{
	size_t test_i;
	struct romazi_config romazi_config = {
		.include_kanji_numerals = 1,
		.classic_wo = 1,
	};

	init_romazi(&romazi_config);

	set_test_source_file(__FILE__);
	config_tests(CONFIG_TESTS_IGNORE_NULL_BYTES);

	for (test_i = 0; test_i < sizeof(test_cases) / sizeof(*test_cases);
	     test_i++) {
		struct mapping mapping = {0};

		mapping.include_kanji = 1;

		start_test(test_cases[test_i].name);

		in = open_tmp_file_containing(test_cases[test_i].in);

		set_mapping(&mapping, test_cases[test_i].mapping_type);
		input_impl(&mapping);
		XFCLOSE(in);

		end_test_expected_content_in_file();

		mapping_destroy(&mapping);
	}
	return 0;
}
