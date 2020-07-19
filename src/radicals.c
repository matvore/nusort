#include "kanji_db.h"
#include "radicals.h"
#include "util.h"

static struct {
	/* 部首の kanji_db() へのインデックス */
	unsigned ki;

	unsigned stroke_cnt;
} radicals[RADICAL_COUNT];

static void prepare(void)
{
	unsigned i, count = 0, stroke_cnt = 0;

	if (radicals[1].ki)
		return;

	if (kanji_from_rsc_index(0)->cutoff_type != 3)
		DIE(0, "【一】の区切りタイプが設定されていない。");

	for (i = 0; count != RADICAL_COUNT; i++) {
		struct kanji_entry const *k = kanji_from_rsc_index(i);

		if (k->cutoff_type < 2)
			continue;

		radicals[count].ki = k - kanji_db();
		if (k->cutoff_type == 3)
			stroke_cnt++;
		radicals[count].stroke_cnt = stroke_cnt;
		count++;
	}
}

void radical_coverage_next(struct radical_coverage *cov)
{
	if (cov->rsc_key_end < cov->rsc_key_start)
		DIE(0, "%u < %u", cov->rsc_key_end, cov->rsc_key_start);
	if (cov->rad_i == 0xffff)
		DIE(0, "既に radical_coverage_done 状態です。");

	prepare();

	while (cov->rad_i < RADICAL_COUNT) {
		cov->current = radicals[cov->rad_i].ki;
		cov->stroke_cnt = radicals[cov->rad_i].stroke_cnt;
		cov->rad_i++;

		if (kanji_db()[cov->current].rsc_sort_key >= cov->rsc_key_end)
			break;
		if (cov->rad_i < RADICAL_COUNT) {
			unsigned end = kanji_db()[radicals[cov->rad_i].ki]
				.rsc_sort_key;
			if (end <= cov->rsc_key_start)
				continue;
		}

		return;
	}

	cov->rad_i = 0xffff;
}
