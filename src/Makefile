.PHONY: test clean

CC = cc -Werror -Wall -Wdeclaration-after-statement -Wformat-security -Wno-format-zero-length -Wold-style-definition -Woverflow -Wpointer-arith -Wstrict-prototypes -Wunused -Wvla

all: test nusort

nusort: main.c romazi.c kanji_db.c util.c last_rank_contained.c check_kanji_db_order.c
	$(CC) -o nusort $^

test: last_rank_contained_test.c
	$(CC) last_rank_contained_test.c
	./a.out

clean:
	rm -f a.out
	rm -f nusort
