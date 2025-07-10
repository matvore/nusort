.PHONY: test clean

CFLAGS = \
	-Werror \
	-Wall \
	-Wdeclaration-after-statement \
	-Wmissing-prototypes \
	-Wformat-security \
	-Wno-format-zero-length \
	-Wno-parentheses \
	-Wold-style-definition \
	-Woverflow \
	-Wpointer-arith \
	-Wstrict-prototypes \
	-Wunused \
	-Wvla \
	$(EXTRA_CFLAGS)

ALL_CFLAGS=-pthread $(CFLAGS)

all: test nusort

SHARED_OBJS = \
	obj/chars.o \
	obj/check_kanji_db_order.o \
	obj/dict_guide.o \
	obj/free_kanji_keys.o \
	obj/h2k.o \
	obj/input.o \
	obj/input_impl.o \
	obj/kana_stats.o \
	obj/kana_stats_util.o \
	obj/kanji_db.o \
	obj/kanji_db_chart.o \
	obj/kanji_distribution.o \
	obj/keyboard.o \
	obj/last_rank_contained.o \
	obj/longest_rsc_block.o \
	obj/make_map.o \
	obj/mapping.o \
	obj/mapping_util.o \
	obj/practice_set.o \
	obj/radicals.o \
	obj/rank_coverage.o \
	obj/residual_stroke_count.o \
	obj/romazi.o \
	obj/packetized_out.o \
	obj/rsc_sort_key.o \
	obj/streams.o \
	obj/util.o \
	obj/windows.o

HDRS = \
	chars.h \
	commands.h \
	dict_guide.h \
	input_impl.h \
	kana_stats_util.h \
	kanji_db.h \
	kanji_distribution.h \
	keyboard.h \
	mapping.h \
	mapping_util.h \
	radicals.h \
	rank_coverage.h \
	residual_stroke_count.h \
	romazi.h \
	streams.h \
	test_util.h \
	util.h \
	windows.h

obj/%.o: %.c $(HDRS)
	$(CC) -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $<

nusort: $(SHARED_OBJS) main.c
	$(CC) $(ALL_CFLAGS) -o nusort $^

%_test_bin: %_test.o $(SHARED_OBJS) test_util.o
	$(CC) $(ALL_CFLAGS) -o $@ $^

%_test: %_test_bin
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
	kanji_db_chart_test \
	kanji_distribution_test \
	kana_stats_util_test \
	keyboard_test \
	last_rank_contained_test \
	longest_rsc_block_test \
	make_map_test \
	mapping_test \
	mapping_util_test \
	practice_set_test \
	radicals_test \
	rank_coverage_test \
	resid_sc_3rd_key_test \
	romazi_test \
	rpc_mode_test

clean:
	rm -f *_test_bin actual_test_out/*.out *.o
	rm -rf test_errors
	rm -f nusort
