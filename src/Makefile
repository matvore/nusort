.PHONY: test clean

CC = cc -Werror -Wall -Wdeclaration-after-statement -Wformat-security -Wno-format-zero-length -Wold-style-definition -Woverflow -Wpointer-arith -Wstrict-prototypes -Wunused -Wvla $(CC_FLAGS)

all: test nusort

SHARED_SRCS = romazi.c kanji_db.c util.c last_rank_contained.c check_kanji_db_order.c commands.c rank_coverage.c free_kanji_keys.c make_map.c kanji_distribution.c
SRCS = $(SHARED_SRCS) main.c
HDRS = commands.h kanji_db.h romazi.h util.h rank_coverage.h
TEST_SRCS = $(SHARED_SRCS) test_util.c
TEST_HDRS = $(HDRS) test_util.h

nusort: $(HDRS) $(SRCS)
	$(CC) -o nusort $(SRCS)

check_kanji_db_order_test: check_kanji_db_order_test.c $(TEST_HDRS) $(TEST_SRCS)
	$(CC) -o $@_bin check_kanji_db_order_test.c $(TEST_SRCS)
	./$@_bin

last_rank_contained_test: last_rank_contained_test.c $(TEST_HDRS) $(TEST_SRCS)
	$(CC) -o $@_bin last_rank_contained_test.c $(TEST_SRCS)
	./$@_bin

make_map_test: make_map_test.c $(TEST_HDRS) $(TEST_SRCS)
	$(CC) -o $@_bin make_map_test.c $(TEST_SRCS)
	./$@_bin

rank_coverage_test: rank_coverage_test.c $(TEST_HDRS) $(TEST_SRCS)
	$(CC) -o $@_bin rank_coverage_test.c $(TEST_SRCS)
	./$@_bin

romazi_test: romazi_test.c $(TEST_HDRS) $(TEST_SRCS)
	$(CC) -o $@_bin romazi_test.c $(TEST_SRCS)
	./$@_bin

test: check_kanji_db_order_test last_rank_contained_test make_map_test rank_coverage_test romazi_test

clean:
	rm -f *_test_bin *.testout
	rm -f nusort
