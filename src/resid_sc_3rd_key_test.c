#include <inttypes.h>
#include <string.h>

#include "kanji_db.h"
#include "mapping.h"
#include "residual_stroke_count.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static struct kanji_entry const *lookup(
	struct key_mapping_array const *arr, Orig orig)
{
	struct key_mapping *me;
	struct kanji_entry const *ke;

	BSEARCH(me, arr->el, arr->cnt, code_cmp(me->orig, orig));
	if (!me) return NULL;

	ke = kanji_db_lookup(me->conv);
	if (!ke) {
		print_mapping(me, stderr);
		DIE(0, "漢字がデータベースで見つからなった");
	}

	return ke;
}

int main(void)
{
	set_test_source_file(__FILE__);

	while (run_test("various_pref", "")) {
		struct mapping m;
		struct {
			char const *pref;
			char third_key;
			short sc, min_match;
			unsigned allow_0_sc : 1;
		} *tc, test_cases[] = {
			{
				.pref = "j ", .third_key = 'q', .sc = 1,
				.min_match = 2, .allow_0_sc = 1,
			},
			{
				.pref = "m ", .third_key = 'q', .sc = 1,
				.min_match = 2, .allow_0_sc = 1,
			},
			{ .pref="m ", .third_key='w', .sc=2, .min_match=5 },
			{ .pref=", ", .third_key='e', .sc=3, .min_match=3 },
			{ .pref=", ", .third_key='r', .sc=4, .min_match=4 },
			{0},
		};
		unsigned arri, matches;
		struct kanji_entry const *ke;
		struct key_mapping const *km;
		int resid_sc;

		for (tc = test_cases; tc->pref; tc++) {
			m = (struct mapping){
				.include_kanji = 1,
				.resid_sc_3rd_key = 1,
			};
			expect_ok(mapping_populate(&m));
			expect_ok(mapping_lazy_populate(&m, tc->pref));

			matches = 0;
			for (arri = 0; arri < m.arr.cnt; arri++) {
				km = m.arr.el + arri;

				if (arri &&
				    code_cmp(km[-1].orig, km[0].orig) >= 0)
					fprintf(out,
					        "順番が違います: %s >= %s\n", 
						km[-1].orig, km[0].orig);

				if (strlen(km->orig) != 4) continue;
				if (tc->third_key != km->orig[2]) continue;
				matches++;
				if (strncmp(km->orig, tc->pref, 2)) {
					print_mapping(km, out);
					fputs(" プレフィックスが間違ってる\n",
					      out);
					continue;
				}

				ke = kanji_db_lookup(km->conv);
				if (!ke)
					DIE(0,
					    "kanji_db で見つからなかった: %s",
					    km->conv);

				resid_sc = residual_stroke_count(ke);
				if (resid_sc == tc->sc) continue;
				if (!resid_sc && tc->allow_0_sc) continue;

				print_mapping(km, out);
				fputc('\n', out);
			}

			if (matches < tc->min_match)
				fprintf(out, "matches = %u (%s%c)\n",
					matches, tc->pref, tc->third_key);

			destroy_mapping(&m);
		}
	}

	while (run_test("no_dup_kanji", "")) {
		struct mapping m = {
			.include_kanji = 1,
			.resid_sc_3rd_key = 1,
		};
		uint8_t *kcnt;
		unsigned mapi, matchn, kcnti, cnt;
		struct kanji_entry const *ke;

		expect_ok(mapping_populate(&m));
		expect_ok(mapping_lazy_populate(&m, "a "));
		expect_ok(mapping_lazy_populate(&m, "b "));
		expect_ok(mapping_lazy_populate(&m, "c "));
		expect_ok(mapping_lazy_populate(&m, "d "));

		kcnt = xcalloc(kanji_db_nr(), sizeof(*kcnt));
		matchn = 0;
		for (mapi = 0; mapi < m.arr.cnt; mapi++) {
			ke = kanji_db_lookup(m.arr.el[mapi].conv);
			if (!ke) continue;

			matchn++;
			kcnt[ke - kanji_db()]++;
		}

		if (matchn < 20) fprintf(out, "matchn: %u\n", matchn);
		for (kcnti = 0; kcnti < kanji_db_nr(); kcnti++) {
			cnt = kcnt[kcnti];
			if (cnt > 1)
				fprintf(out, "dup: %s x %u\n",
					kanji_db()[kcnti].c, cnt);
		}

		destroy_mapping(&m);
		free(kcnt);
	}

	while (run_test("first_key_1_no_dup_codes", "")) {
		struct mapping m = {
			.include_kanji = 1,
			.resid_sc_3rd_key = 1,
		};
		expect_ok(mapping_populate(&m));
		expect_ok(mapping_lazy_populate(&m, "1 "));
		destroy_mapping(&m);
	}

	while (run_test("correct_range", "")) {
		struct {
			char const *pref;
			unsigned rsc_ndx_lo, rsc_ndx_hi;
		} *tc, test_cases[] = {
			{
				.pref = ", ",
				.rsc_ndx_lo = kanji_db_nr()/2,
				.rsc_ndx_hi = kanji_db_nr(),
			},
			{
				.pref = "2 ",
				.rsc_ndx_lo = 0,
				.rsc_ndx_hi = kanji_db_nr()/2,
			},
			{0},
		};

		struct mapping m;
		unsigned start_cnt, mi, rsc_ndx, matches;
		Orig *orig;
		Conv *conv;
		struct kanji_entry const *ke;

		for (tc = test_cases; tc->pref; tc++) {
			m = (struct mapping){
				.include_kanji = 1,
				.resid_sc_3rd_key = 1,
			};
			expect_ok(mapping_populate(&m));
			start_cnt = m.arr.cnt;
			expect_ok(mapping_lazy_populate(&m, tc->pref));
			if (start_cnt == m.arr.cnt)
				fprintf(out,
					"入力コードが追加されなかった: %u\n",
					start_cnt);

			matches = 0;
			for (mi = 0; mi < m.arr.cnt; mi++) {
				orig = &m.arr.el[mi].orig;
				conv = &m.arr.el[mi].conv;

				if (strlen(*orig) != 4) continue;
				if (strncmp(*orig, tc->pref, 2)) {
					fprintf(stderr,
						"想定外のコード: %s\n", *orig);
					continue;
				}

				matches++;
				ke = kanji_db_lookup(*conv);
				if (!ke) DIE(0, "!ke: %s", *conv);

				rsc_ndx = kanji_db_rsc_index(ke);
				if (rsc_ndx < tc->rsc_ndx_lo ||
				    rsc_ndx >= tc->rsc_ndx_hi)
					fprintf(out,
						"rsc_ndx が想定外: "
						"%s->%s (%u)\n",
						*orig, *conv, rsc_ndx);
			}
			if (matches < 5)
				fprintf(out, "matches = %u\n", matches);
			destroy_mapping(&m);
		}
	}

	while (run_test("full_set", "")) {
		struct mapping m;
		int keyi, inc_romazi;
		char pref[3];
		struct romazi_config rom;
		unsigned start_cnt;

		for (inc_romazi = 0; inc_romazi < 2; inc_romazi++) {
			m = (struct mapping) {
				.include_kanji = 1,
				.resid_sc_3rd_key = 1,
			};
			rom = (struct romazi_config) {0};

			if (inc_romazi) get_romazi_codes(&rom, &m.arr);
			start_cnt = m.arr.cnt;

			expect_ok(mapping_populate(&m));
			for (keyi = 0; keyi < KANJI_KEY_COUNT; keyi++) {
				pref[0] = KEY_INDEX_TO_CHAR_MAP[keyi];
				pref[1] = ' ';
				pref[2] = 0;
				expect_ok(mapping_lazy_populate(&m, pref));
			}

			if (m.arr.cnt - start_cnt != kanji_db_nr())
				fprintf(out, "%zu - %u != %u\n",
					m.arr.cnt, start_cnt, kanji_db_nr());

			destroy_mapping(&m);
		}
	}

	while (run_test("ranking_ergonomics", "")) {
		struct {
			unsigned inc_romazi : 1;
			unsigned six_is_rh : 1;

			struct romazi_config rom;
			char const *pref;
			char sc_cell;
		} *tc, test_cases[] = {
			{ .pref = "q ", .sc_cell = 'q' },
			{
				.inc_romazi = 1,
				.pref = "1 ", .sc_cell = 'q',
			},
			{
				.inc_romazi = 1,
				.rom = { .optimize_keystrokes = 1 },
				.pref = "x ",
				.sc_cell = 'i',
			},
			{
				.inc_romazi = 1,
				.rom = { .optimize_keystrokes = 1 },
				.pref = "x ",
				.sc_cell = 't',
			},
			{
				.inc_romazi = 1,
				.rom = { .optimize_keystrokes = 1 },
				.pref = "r ",
				.sc_cell = 'j',
				.six_is_rh = 1,
			},
			{0},
		};

		struct mapping m;
		Orig code1, code2;
		struct kanji_entry const *ke1, *ke2;
		int kij, ki, kj, ergo_lt;

		for (tc = test_cases; tc->pref; tc++) {
			m = (struct mapping){
				.include_kanji = 1,
				.resid_sc_3rd_key = 1,
				.six_is_rh = tc->six_is_rh,
			};

			if (tc->inc_romazi) get_romazi_codes(&tc->rom, &m.arr);
			expect_ok(mapping_populate(&m));

			expect_ok(mapping_lazy_populate(&m, tc->pref));

			memcpy(code1, tc->pref, 2);
			code1[2] = tc->sc_cell;
			memcpy(code2, tc->pref, 2);
			code2[2] = tc->sc_cell;

			kij = MAPPABLE_CHAR_COUNT * MAPPABLE_CHAR_COUNT;
			while (kij--) {
				ki = kij % MAPPABLE_CHAR_COUNT;
				kj = kij / MAPPABLE_CHAR_COUNT;

				code1[3] = KEY_INDEX_TO_CHAR_MAP[ki];
				code2[3] = KEY_INDEX_TO_CHAR_MAP[kj];

				ke1 = lookup(&m.arr, code1);
				ke2 = lookup(&m.arr, code2);
				if (!ke1 || !ke2 || ke1 == ke2) continue;

				ergo_lt = !!ergonomic_lt(
					code1+2, code2+2, tc->six_is_rh);

				if ((ke1->ranking < ke2->ranking) == ergo_lt)
					continue;

				fprintf(out, "%s->%s (%"PRIu16") "
					     "%s->%s (%"PRIu16")\n",
					code1, ke1->c, ke1->ranking,
					code2, ke2->c, ke2->ranking);
			}

			destroy_mapping(&m);
		}
	}
}
