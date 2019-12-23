#include <stdio.h>
#include <stdlib.h>

#define CONFIG_TESTS_IGNORE_NULL_BYTES 1
void config_tests(int flags_);

void start_test(const char *source_file, const char *name);
void end_test(const char *expected);
void end_test_expected_content_in_file(void);
void store_in_tmp_file(char const *str, char *tmp_file_template);
