/*
 * 康熙字典で214個があり、日本語特有の部首 (⺍) が1個あるので、
 * 全部で 215 個あります。
 *
 *						   215
 * 一丨丶丿乙亅		を同一の部首と見なしす。   - 5
 * 匚匸			を同一の部首と見なしす。   - 1
 * 夊夂			を同一の部首と見なしす。   - 1
 * 日曰			を同一の部首と見なしす。   - 1
 * 						 = 207
 */
#define RADICAL_COUNT 207

/*
 * [rsc_key_start, rsc_key_end) で指定された範囲で利用されている部首を取得する
 * のに使います。
 *
 * 使い方:
 *	struct radical_coverage c = {
 *		.rsc_key_start = 5,
 *		.rsc_key_start = 15,
 *	};
 *	while (1) {
 *		radical_coverage_next(&c);
 *		if (radical_coverage_done(&c))
 *			break;
 *
 *		// kanji_db()[c.current] が部首
 *	}
 */
struct radical_coverage {
	unsigned rsc_key_start, rsc_key_end;

	/* kanji_db() へのインデックス */
	unsigned current;

	/* current の画数 */
	unsigned stroke_cnt;

	unsigned rad_i;
};

static inline int radical_coverage_done(struct radical_coverage *cov) {
	return cov->rad_i == 0xffff;
}

void radical_coverage_next(struct radical_coverage *);
