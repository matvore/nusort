.PHONY: test clean

CFLAGS = \
	-pthread \
	-Werror \
	-Wall \
	-Wdeclaration-after-statement \
	-Wmissing-prototypes \
	-Wformat-security \
	-Wno-format-zero-length \
	-Wold-style-definition \
	-Woverflow \
	-Wpointer-arith \
	-Wstrict-prototypes \
	-Wunused \
	-Wvla

all: test nusort

SHARED_OBJS = \
	chars.o \
	check_kanji_db_order.o \
	free_kanji_keys.o \
	h2k.o \
	input.o \
	input_impl.o \
	kana_stats.o \
	kana_stats_util.o \
	kanji_db.o \
	kanji_distribution.o \
	keyboard.o \
	last_rank_contained.o \
	make_map.o \
	mapping.o \
	rank_coverage.o \
	romazi.o \
	streams.o \
	util.o

HDRS = \
	chars.h \
	commands.h \
	input_impl.h \
	kana_stats_util.h \
	kanji_db.h \
	kanji_distribution.h \
	keyboard.h \
	mapping.h \
	rank_coverage.h \
	romazi.h \
	streams.h \
	test_util.h \
	util.h

%.o: %.c $(HDRS)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

nusort: $(SHARED_OBJS) main.o
	$(CC) $(CFLAGS) -o nusort $^

%_test: %_test.o $(SHARED_OBJS) test_util.o
	$(CC) $(CFLAGS) -o $@_bin $^
	@mkdir -p test_errors
	./$@_bin >test_errors/$@
	diff /dev/null test_errors/$@

test: \
	bsearch_test \
	check_kanji_db_order_test \
	hashmap_test \
	h2k_test \
	input_test \
	kana_stats_test \
	kana_stats_util_test \
	keyboard_test \
	last_rank_contained_test \
	make_map_test \
	rank_coverage_test \
	romazi_test

clean:
	rm -f *_test_bin *.testout *.o
	rm -rf test_errors
	rm -f nusort
