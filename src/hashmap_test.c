#include <stdint.h>

#include "streams.h"
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
	set_test_source_file(__FILE__);

	while (run_test("finds_unused_bucket", "0 0 ")) {
		struct int_hashmap h = {0};
		int key;
		int *found_key, *found_value;

		INIT_HASHMAP(h, 100);

		key = 20;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		fprintf(out, "%d %d ", *found_key, *found_value);

		DESTROY_HASHMAP(h);
	}

	while (run_test("filling_up_all_buckets", "0 0 0 0 0 ")) {
		struct int_hashmap h = {0};
		int key;
		int *found_key, *found_value;

		INIT_HASHMAP(h, 5);

		key = 10;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		fprintf(out, "%d ", *found_key);
		*found_key = key;
		*found_value = 1;
		key = 20;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		fprintf(out, "%d ", *found_key);
		*found_key = key;
		*found_value = 1;
		key = 30;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		fprintf(out, "%d ", *found_key);
		*found_key = key;
		*found_value = 1;
		key = 40;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		fprintf(out, "%d ", *found_key);
		*found_key = key;
		*found_value = 1;
		key = 50;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		fprintf(out, "%d ", *found_key);
		*found_key = key;
		*found_value = 1;

		DESTROY_HASHMAP(h);
	}

	while (run_test("nonconsecutive_buckets",
			"<0 0><0 0><30 900><0 0><0 0><0 0><10 100><0 0><0 0>"
			"<0 0><0 0><0 0><0 0><0 0><0 0><0 0>")) {
		struct int_hashmap h = {0};
		int key;
		int *found_key, *found_value;
		size_t i;

		INIT_HASHMAP(h, 16);

		key = 10;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		*found_key = key;
		*found_value = 100;
		key = 30;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		*found_key = key;
		*found_value = 900;

		for (i = 0; i < h.bucket_cnt; i++)
			fprintf(out, "<%d %d>", h.keys[i], h.values[i]);

		DESTROY_HASHMAP(h);
	}

	while (run_test("zero_out_bytes_in_free", "1 1 0")) {
		struct int_hashmap h = {0};
		int key;
		int *found_key, *found_value;

		INIT_HASHMAP(h, 16);

		key = 33;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		*found_key = key;
		*found_value = 42;

		DESTROY_HASHMAP(h);
		fprintf(out, "%d %d %zu", !h.keys, !h.values, h.bucket_cnt);
	}

	while (run_test("finds_existing_key", "<33 42><-101 99>")) {
		struct int_hashmap h = {0};
		int key;
		int *found_key, *found_value;

		INIT_HASHMAP(h, 16);

		key = 33;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		*found_key = key;
		*found_value = 42;

		key = -101;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		*found_key = key;
		*found_value = 99;

		key = 33;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		fprintf(out, "<%d %d>", *found_key, *found_value);

		key = -101;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		fprintf(out, "<%d %d>", *found_key, *found_value);
	}

	while (run_test("string_hashmap",
			"'' '' < 0>< 0><!FOOBAR 88>< 0>< 0>< 0>< 0>< 0>< 0>< 0>"
			"< 0>< 0><foobar! 44>< 0>< 0>< 0>")) {
		struct string_hashmap h = {0};
		char key[8];
		char (*found_key)[8];
		uint8_t *found_value;
		size_t i;

		INIT_HASHMAP(h, 16);

		strcpy(key, "foobar!");
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		fprintf(out, "'%s' ", *found_key);
		strcpy(*found_key, key);
		*found_value = 44;

		strcpy(key, "!FOOBAR");
		FIND_HASHMAP_ENTRY(h, key, found_key);
		found_value = VALUE_PTR_FOR_HASH_KEY(h, found_key);
		fprintf(out, "'%s' ", *found_key);
		strcpy(*found_key, key);
		*found_value = 88;

		for (i = 0; i < h.bucket_cnt; i++)
			fprintf(out, "<%s %d>", h.keys[i], h.values[i]);

		DESTROY_HASHMAP(h);
	}

	while (run_test("do_not_initialize_values_if_they_are_not_used",
			"h.values が設定済み: 0\n"
			"49\n"
			"h.values が設定済み: 0\n")) {
		struct {
			int *keys;
			void *values;
			size_t bucket_cnt;
		} h = {0};
		int key;
		int *found_key;

		INIT_HASHMAP(h, 16);

		key = 49;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		*found_key = key;

		key = 1011;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		*found_key = key;

		fprintf(out, "h.values が設定済み: %d\n", h.values != 0);
		key = 49;
		FIND_HASHMAP_ENTRY(h, key, found_key);
		fprintf(out, "%d\n", *found_key);
		fprintf(out, "h.values が設定済み: %d\n", h.values != 0);

		DESTROY_HASHMAP(h);
	}
}
