.PHONY: test clean

CC = cc -Werror -Wall -Wdeclaration-after-statement -Wformat-security -Wno-format-zero-length -Wold-style-definition -Woverflow -Wpointer-arith -Wstrict-prototypes -Wunused -Wvla $(CC_FLAGS)

all: test nusort

SHARED_SRCS = \
	check_kanji_db_order.c \
	commands.c \
	free_kanji_keys.c \
	kanji_db.c \
	kanji_distribution.c \
	keyboard.c \
	last_rank_contained.c \
	make_map.c \
	mapping.c \
	rank_coverage.c \
	romazi.c \
	util.c

SRCS = $(SHARED_SRCS) main.c

HDRS = \
	commands.h \
	kanji_db.h \
	kanji_distribution.h \
	keyboard.h \
	mapping.h \
	rank_coverage.h \
	romazi.h \
	util.h

TEST_SRCS = $(SHARED_SRCS) test_util.c
TEST_HDRS = $(HDRS) test_util.h

nusort: $(HDRS) $(SRCS)
	$(CC) -o nusort $(SRCS)

check_kanji_db_order_test: check_kanji_db_order_test.c $(TEST_HDRS) $(TEST_SRCS)
	$(CC) -o $@_bin check_kanji_db_order_test.c $(TEST_SRCS)
	./$@_bin

keyboard_test: keyboard_test.c $(TEST_HDRS) $(TEST_SRCS)
	$(CC) -o $@_bin keyboard_test.c $(TEST_SRCS)
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

test: check_kanji_db_order_test \
	keyboard_test \
	last_rank_contained_test \
	make_map_test \
	rank_coverage_test \
	romazi_test

clean:
	rm -f *_test_bin *.testout
	rm -f nusort
