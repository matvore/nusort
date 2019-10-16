#include "commands.h"
#include "input_impl.h"
#include "mapping.h"
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
			mapping_populate(m);
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
	for (test_i = 0; test_i < sizeof(test_cases) / sizeof(*test_cases);
	     test_i++) {
		char input_file[] = "/tmp/input_test_in_XXXXXX";
		struct mapping mapping = {0};

		start_test(__FILE__, test_cases[test_i].name);

		store_in_tmp_file(test_cases[test_i].in, input_file);

		in = xfopen(input_file, "r");
		set_mapping(&mapping, test_cases[test_i].mapping_type);
		input_impl(&mapping);
		xfclose(in);
		in = stdin;

		end_test_expected_content_in_file();

		mapping_destroy(&mapping);
	}
	return 0;
}