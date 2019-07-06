.PHONY: test clean

CC = cc -Werror -Wall -Wdeclaration-after-statement -Wformat-security -Wno-format-zero-length -Wold-style-definition -Woverflow -Wpointer-arith -Wstrict-prototypes -Wunused -Wvla

all: test nusort

SHARED_SRCS = romazi.c kanji_db.c util.c last_rank_contained.c check_kanji_db_order.c commands.c rank_coverage.c
SRCS = $(SHARED_SRCS) main.c
HDRS = commands.h kanji_db.h romazi.h util.h rank_coverage.h

nusort: $(HDRS) $(SRCS)
	$(CC) -o nusort $(SRCS)

last_rank_contained_test: last_rank_contained_test.c $(HDRS) $(SHARED_SRCS)
	$(CC) -o $@_bin last_rank_contained_test.c $(SHARED_SRCS)
	./$@_bin

rank_coverage_test: rank_coverage_test.c $(HDRS) $(SHARED_SRCS)
	$(CC) -o $@_bin rank_coverage_test.c $(SHARED_SRCS)
	./$@_bin

test: last_rank_contained_test rank_coverage_test

clean:
	rm -f *_test_bin
	rm -f nusort
