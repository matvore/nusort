#include <stdint.h>

#include "commands.h"
#include "test_util.h"
#include "util.h"

struct int_hashmap {
	int *keys;
	int *values;
	size_t bucket_cnt;
};

struct string_hashmap {
	char (*keys)[8];
	uint8_t *values;
	size_t bucket_cnt;
};

int main(void)
{
	start_test(__FILE__, "finds_unused_bucket");
	{
		struct int_hashmap h = {0};
		int key;
		int *found_key, *found_value;

		INIT_HASHMAP(h, 100);

		key = 20;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		xfprintf(out, "%d %d ", *found_key, *found_value);

		DESTROY_HASHMAP(h);
	}
	end_test("0 0 ");

	start_test(__FILE__, "filling_up_all_buckets");
	{
		struct int_hashmap h = {0};
		int key;
		int *found_key, *found_value;

		INIT_HASHMAP(h, 5);

		key = 10;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		xfprintf(out, "%d ", *found_key);
		*found_key = key;
		*found_value = 1;
		key = 20;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		xfprintf(out, "%d ", *found_key);
		*found_key = key;
		*found_value = 1;
		key = 30;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		xfprintf(out, "%d ", *found_key);
		*found_key = key;
		*found_value = 1;
		key = 40;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		xfprintf(out, "%d ", *found_key);
		*found_key = key;
		*found_value = 1;
		key = 50;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		xfprintf(out, "%d ", *found_key);
		*found_key = key;
		*found_value = 1;

		DESTROY_HASHMAP(h);
	}
	end_test("0 0 0 0 0 ");

	start_test(__FILE__, "nonconsecutive_buckets");
	{
		struct int_hashmap h = {0};
		int key;
		int *found_key, *found_value;
		size_t i;

		INIT_HASHMAP(h, 16);

		key = 10;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		*found_key = key;
		*found_value = 100;
		key = 30;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		*found_key = key;
		*found_value = 900;

		for (i = 0; i < h.bucket_cnt; i++)
			xfprintf(out, "<%d %d>", h.keys[i], h.values[i]);

		DESTROY_HASHMAP(h);
	}
	end_test("<0 0><0 0><30 900><0 0><0 0><0 0><10 100><0 0><0 0>"
		 "<0 0><0 0><0 0><0 0><0 0><0 0><0 0>");

	start_test(__FILE__, "zero_out_bytes_in_free");
	{
		struct int_hashmap h = {0};
		int key;
		int *found_key, *found_value;

		INIT_HASHMAP(h, 16);

		key = 33;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		*found_key = key;
		*found_value = 42;

		DESTROY_HASHMAP(h);
		xfprintf(out, "%d %d %zu", !h.keys, !h.values, h.bucket_cnt);
	}
	end_test("1 1 0");

	start_test(__FILE__, "finds_existing_key");
	{
		struct int_hashmap h = {0};
		int key;
		int *found_key, *found_value;

		INIT_HASHMAP(h, 16);

		key = 33;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		*found_key = key;
		*found_value = 42;

		key = -101;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		*found_key = key;
		*found_value = 99;

		key = 33;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		xfprintf(out, "<%d %d>", *found_key, *found_value);

		key = -101;
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		xfprintf(out, "<%d %d>", *found_key, *found_value);
	}
	end_test("<33 42><-101 99>");

	start_test(__FILE__, "string_hashmap");
	{
		struct string_hashmap h = {0};
		char key[8];
		char (*found_key)[8];
		uint8_t *found_value;
		size_t i;

		INIT_HASHMAP(h, 16);

		strcpy(key, "foobar!");
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		xfprintf(out, "'%s' ", *found_key);
		strcpy(*found_key, key);
		*found_value = 44;

		strcpy(key, "!FOOBAR");
		FIND_HASHMAP_ENTRY(h, key, found_key, found_value);
		xfprintf(out, "'%s' ", *found_key);
		strcpy(*found_key, key);
		*found_value = 88;

		for (i = 0; i < h.bucket_cnt; i++)
			xfprintf(out, "<%s %d>", h.keys[i], h.values[i]);

		DESTROY_HASHMAP(h);
	}
	end_test("'' '' < 0>< 0><!FOOBAR 88>< 0>< 0>< 0>< 0>< 0>< 0>< 0>< 0>"
		 "< 0><foobar! 44>< 0>< 0>< 0>");
}
