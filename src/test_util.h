#include <stdio.h>
#include <stdlib.h>

void start_test(const char *source_file, const char *name);
void end_test(const char *expected);
void end_test_expected_content_in_file(void);
void store_in_tmp_file(char const *str, char *tmp_file_template);
