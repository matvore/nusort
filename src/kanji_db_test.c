#include "kanji_db.h"
#include "streams.h"
#include "test_util.h"
#include "util.h"

static int cutoff_type(char const *k)
{
	struct kanji_entry const *e = kanji_db_lookup(k);
	if (e)
		return e->cutoff_type;
	fprintf(err, "見つからない: %s\n", k);
	return 0;
}

static void print_if_cutoff(char const *k)
{
	if (cutoff_type(k))
		fprintf(out, "%s\n", k);
}

static void verify_rsc_order(char const *a, char const *b, char const *c)
{
	struct kanji_entry const *k1 = kanji_db_lookup(a);
	struct kanji_entry const *k2 = kanji_db_lookup(b);
	struct kanji_entry const *k3 = kanji_db_lookup(c);
	if (k1->rsc_sort_key >= k2->rsc_sort_key ||
	    k3->rsc_sort_key <= k2->rsc_sort_key)
		fprintf(out, "%d, %d, %d\n",
			 k1->rsc_sort_key, k2->rsc_sort_key,
			 k3->rsc_sort_key);
}

int main(void)
{
	set_test_source_file(__FILE__);

	start_test("lookup_non_kanji");
	{
		if (kanji_db_lookup("?"))
			fputs("漢字ではない文字がデータベースにある", out);
	}
	end_test("");

	start_test("lookup_kanji");
	{
		struct kanji_entry const *e = kanji_db_lookup("格");
		if (e)
			fputs(e->c, out);
	}
	end_test("格");

	start_test("rittou_hen_7_stroke_cutoff");
	{
		print_if_cutoff("剃");
		print_if_cutoff("則");
		print_if_cutoff("削");
		print_if_cutoff("剋");
		print_if_cutoff("剌");
		print_if_cutoff("前");
	}
	end_test("前\n");

	start_test("tree_rad_with_11_resid_strokes_bad_cutoffs");
	{
		print_if_cutoff("槻");
		print_if_cutoff("槽");
		print_if_cutoff("槿");
		print_if_cutoff("樂");
		print_if_cutoff("樅");
		print_if_cutoff("樋");
		print_if_cutoff("樓");
		print_if_cutoff("樗");
		print_if_cutoff("樟");
		print_if_cutoff("樣");
	}
	end_test("");

	start_test("tree_rad_with_11_resid_strokes_good_cutoffs");
	{
		int cutoff_count = 0;
		if (cutoff_type("標"))
			cutoff_count++;
		if (cutoff_type("権"))
			cutoff_count++;
		if (cutoff_type("横"))
			cutoff_count++;
		fprintf(out, "%d", cutoff_count);
	}
	end_test("1");

	start_test("include_sude_no_tsukuri_and_munyou");
	{
		struct kanji_entry const *e = kanji_db_lookup("无");
		if (!e)
			fprintf(out, "ない\n");
		e = kanji_db_lookup("旡");
		if (!e)
			fprintf(out, "ない\n");
	}
	end_test("");

	start_test("sude_no_tsukuri_and_munyou_cutoff_type");
	{
		fprintf(out, "%d\n", cutoff_type("无"));
		fprintf(out, "%d\n", cutoff_type("旡"));
	}
	end_test("2\n0\n");

	start_test("kunshou_no_shou_has_correct_rad");
	{
		struct kanji_entry const *a = kanji_db_lookup("立");
		struct kanji_entry const *b = kanji_db_lookup("章");
		struct kanji_entry const *c = kanji_db_lookup("竹");

		if (!a || !b || !c)
			fputs("ない\n", out);
		else if (a->rsc_sort_key < b->rsc_sort_key &&
			 b->rsc_sort_key < c->rsc_sort_key)
			fputs("ok\n", out);
	}
	end_test("ok\n");

	start_test("kunshou_no_shou_is_after_simplified_dragon_with_diff_rsc");
	{
		struct kanji_entry const *a = kanji_db_lookup("竜");
		struct kanji_entry const *b = kanji_db_lookup("章");

		if (!a || !b)
			fputs("ない\n", out);
		else if (a->rsc_sort_key < b->rsc_sort_key)
			fputs("ok\n", out);
	}
	end_test("ok\n");

	start_test("u_7adf_is_same_rsc_as_kunshou_no_shou");
	{
		struct kanji_entry const *a = kanji_db_lookup("竟");
		struct kanji_entry const *b = kanji_db_lookup("章");

		if (!a || !b)
			fputs("ない\n", out);
		else if (a->rsc_sort_key == b->rsc_sort_key)
			fputs("ok\n", out);
	}
	end_test("ok\n");

	start_test("rsc_sorted_has_ascending_rsc_sort_key");
	{
		int k;
		struct kanji_entry const *db = kanji_db();
		uint16_t const *rsc = kanji_db_rsc_sorted();

		for (k = 1; k < kanji_db_nr(); k++) {
			if (db[rsc[k]].rsc_sort_key <
			    db[rsc[k - 1]].rsc_sort_key)
				fprintf(out, "%s", db[rsc[k]].c);
		}
	}
	end_test("");

	start_test("can_find_rsc_sorted_index");
	{
		fprintf(
			out, "%s",
			kanji_db()[
				kanji_db_rsc_sorted()[
					kanji_db_rsc_index(
						kanji_db_lookup("一"))]].c);
		fprintf(
			out, "%s",
			kanji_db()[
				kanji_db_rsc_sorted()[
					kanji_db_rsc_index(
						kanji_db_lookup("楽"))]].c);
	}
	end_test("一楽");

	start_test("has_mu_radical");
	{
		struct kanji_entry const *mu = kanji_db_lookup("厶");
		if (!mu)
			fputs("ない\n", out);
	}
	end_test("");

	start_test("mottomo_has_iwaku_radical");
	{
		verify_rsc_order("日", "最", "月");
	}
	end_test("");

	start_test("okasu_has_iwaku_radical");
	{
		verify_rsc_order("日", "冒", "月");
	}
	end_test("");

	start_test("tooth_shintaiji_is_tooth_radicalits_own_radical");
	{
		if (kanji_db_lookup("齒")->rsc_sort_key >
		    kanji_db_lookup("歯")->rsc_sort_key)
			fputs("失敗", out);
	}
	end_test("");

	start_test("radicals_have_right_cutoff_type");
	{
		struct kanji_entry const *e;

		e = kanji_db_lookup("皿");
		if (e->cutoff_type != 2)
			fputs(e->c, out);
	}
	end_test("");

	start_test("ataru_is_small_radical");
	{
		verify_rsc_order("少", "当", "尤");
	}
	end_test("");

	start_test("all_one_stroke_rads_have_same_rsc_key");
	{
		char const *k[] = {"丨", "丶", "丿", "乙", 0};
		unsigned expected = kanji_db_lookup("一")->rsc_sort_key;
		int i;
		for (i = 0; k[i]; i++) {
			struct kanji_entry const *e = kanji_db_lookup(k[i]);

			if (!e) {
				fprintf(out, "kanji_db で見つからない: %s\n",
					k[i]);
				continue;
			}

			if (e->rsc_sort_key != expected)
				fputs(k[i], out);
		}
	}
	end_test("");

	start_test("has_all_radicals");
	{
		char const *rads[] = {
			"一", "丨", "丶", "丿", "乙", "亅", "二", "亠", "人",
			"儿", "入", "八", "冂", "冖", "冫", "几", "凵", "刀",
			"力", "勹", "匕", "匚", "匸", "十", "卜", "卩", "厂",
			"厶", "又", "口", "囗", "土", "士", "夂", "夊", "夕",
			"大", "女", "子", "宀", "寸", "小", "尢", "尸", "屮",
			"山", "巛", "工", "己", "巾", "干", "幺", "广", "廴",
			"廾", "弋", "弓", "彐", "彡", "彳", "心", "戈", "戶",
			"手", "支", "攴", "文", "斗", "斤", "方", "无", "日",
			"曰", "月", "木", "欠", "止", "歹", "殳", "毋", "比",
			"毛", "氏", "气", "水", "火", "爪", "父", "爻", "爿",
			"片", "牙", "牛", "犬", "玄", "玉", "瓜", "瓦", "甘",
			"生", "用", "田", "疋", "疒", "癶", "白", "皮", "皿",
			"目", "矛", "矢", "石", "示", "禸", "禾", "穴", "立",
			"竹", "米", "糸", "缶", "网", "羊", "羽", "老", "而",
			"耒", "耳", "聿", "肉", "臣", "自", "至", "臼", "舌",
			"舛", "舟", "艮", "色", "艸", "虍", "虫", "血", "行",
			"衣", "襾", "見", "角", "言", "谷", "豆", "豕", "豸",
			"貝", "赤", "走", "足", "身", "車", "辛", "辰", "辵",
			"邑", "酉", "釆", "里", "金", "長", "門", "阜", "隶",
			"隹", "雨", "靑", "非", "面", "革", "韋", "韭", "音",
			"頁", "風", "飛", "食", "首", "香", "馬", "骨", "高",
			"髟", "鬥", "鬯", "鬲", "鬼", "魚", "鳥", "鹵", "鹿",
			"麥", "麻", "黃", "黍", "黑", "黹", "黽", "鼎", "鼓",
			"鼠", "鼻", "齊", "齒", "龍", "龜", "龠", "⺍", 0,
		};
		int i;
		for (i = 0; rads[i]; i++) {
			if (!kanji_db_lookup(rads[i]))
				fprintf(out, "kanji_db で見つからない: %s\n",
					rads[i]);
		}
	}
	end_test("");
}
