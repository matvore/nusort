.PHONY: test clean

CFLAGS = \
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

ALL_CFLAGS=-pthread $(CFLAGS)

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
	mapping_util.o \
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
	mapping_util.h \
	rank_coverage.h \
	romazi.h \
	streams.h \
	test_util.h \
	util.h

%.o: %.c $(HDRS)
	$(CC) -c $(CPPFLAGS) $(ALL_CFLAGS) $<

nusort: $(SHARED_OBJS) main.o
	$(CC) $(ALL_CFLAGS) -o nusort $^

%_test: %_test.o $(SHARED_OBJS) test_util.o
	$(CC) $(ALL_CFLAGS) -o $@_bin $^
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
	kanji_db_test \
	kanji_distribution_test \
	kana_stats_util_test \
	keyboard_test \
	last_rank_contained_test \
	make_map_test \
	mapping_test \
	mapping_util_test \
	rank_coverage_test \
	romazi_test

clean:
	rm -f *_test_bin actual_test_out/*.out *.o
	rm -rf test_errors
	rm -f nusort
