#include "commands.h"
#include "util.h"

#include <stdlib.h>

static void verify_contents(const char *file, const char *contents)
{
	size_t contents_i = 0;
	FILE *actual_stream = xfopen(file, "r");
	char buffer[512];

	while (contents[contents_i]) {
		size_t read_len;

		if (!xfgets(buffer, sizeof(buffer), actual_stream))
			goto failure;

		read_len = strlen(buffer);

		if (strncmp(buffer, contents + contents_i, read_len))
			goto failure;
		contents_i += read_len;
	}

	if (xfgets(buffer, sizeof(buffer), actual_stream))
		/* contentsがfileの内容より短い */
		goto failure;
	xfclose(actual_stream);

	return;
failure:
	fprintf(stderr, "actual: %s\nexpected:\n%s\n", file, contents);
	exit(1);
}

static struct {
	const char *name;
	const char *args[40];
	const char *expected;
} test_cases[] = {

{
	"not_enough_args",
	{"一"},
	"32個の区切り漢字を必するけれど、1個が渡された。\n"
	"exit: 1\n"
},

{
	"typical_kugiri_kanji_test",
	{
		"-s",
		"会", "信", "刀", "匚", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "日", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	"[ 一 ] 1 丁了万丈上下与丸久中予不世主両争乱事並乳乗乾互井亡交京人入今元介公仕他付代令以兄 (1142 . -3 . -3)\n"
	"[ 会 ] 2 会全共充先光仮仰仲件任企伊休伝作児兵伴伸伺似但位低住佐体何余使免舎併例具典供依価 (1552 . 3 . 0)\n"
	"[ 信 ] 3 信便係促保個修俺倉倍倒候借値党健停側備働催傷傾像僕億優償内円冊再最写冬冷凄凍処出 (1289 . 1 . 1)\n"
	"[ 刀 ] 4 分切刊列初判別利到制刷券刺刻則削前剤副割創劇力功加助努労効勉動務勝募勤勢勧包化北 (1150 . -1 . 0)\n"
	"[ 匚 ] 5 区医千午半卒協南単博占印危即厚原厳去参又及友反収取受口可古台史右号司合各吉同名向 (975 . -2 . -2)\n"
	"[ 告 ] 6 告君否含吸吹呂味周呼命和品咲員唱商問善喜喫営器回因団困囲図固国圏園土圧在地坂均坊 (1398 . 3 . 1)\n"
	"[ 型 ] 7 型城域執基埼堂報場塗塚塩塾境増壁壊士声売変夏夕外多夜夢大天太夫央失奇奈奏契奥奨奮 (1303 . 3 . 4)\n"
	"[ 女 ] 8 女奴好妙妹妻姉始委姿姫娘婚婦嫌嬉子字存季学宅宇守安完宗官宙定宜宝実客宣室宮害家容 (1224 . 0 . 4)\n"
	"[ 宿 ] 9 宿寂寄密富寒寝察寧審寺対寿封専将射導小少就尾局居届屋展属層履山岐岡岩岸島崎崩川州 (1339 . 2 . 6)\n"
	"[ 工 ] 0 工左巨差己巻市布希師席帯帰帳常幅幌幕干平年幸幹幼庁広床底店府度座庫庭康廃延建弁弊 (1237 . 0 . 6)\n"
	"[ 弋 ] q 式引弱張強弾当形彩影役彼待律後徒従得御復微徳徴心必志忘忙応快念怒怖思急性怪恋恐息恵 (1120 . -2 . 4)\n"
	"[ 悪 ] w 悪悟悠患悩悲情惑惜惨惹想愉意愚愛感慌態慎慮慢慣慰慶憂憧憩憲憶懐懸成我戒戦戯戴 (1811 . 18 . 22)\n"
	"[ 戸 ] r 戸戻房所手打払扱承技投抗折抜択抱押担拠拡持指挑挙振授掛採探接推掲描揃 (1066 . -7 . 15)\n"
	"[ 提 ] t 提揚換握援損携摘撃撮操支改攻放政故救敗教散数敵敷整文料断新方施旅族既 (1222 . 0 . 15)\n"
	"[ 日 ] y 日旧早曲旨旬更昇明易昔星映春昨昭是昼時書普替景晴暑暇暖暗暮暴曜月有服望朝期 (1131 . -2 . 13)\n"
	"[ 木 ] p 木未末本札材村束条来杯東松板析林枚果柄染査栄校株根格案桜梅森棚植検極 (1156 . -2 . 11)\n"
	"[ 業 ] s 業楽概構様模標権横樹橋機欄欠次欲歌止正武歩歯歳歴死残段殺母毎比毛氏民 (1023 . -2 . 9)\n"
	"[ 气 ] d 気水永求江池決沖沢河油治況泉泊法波泣注活洋洗津海派流浜浮浴消済液深混清 (999 . -10 . -1)\n"
	"[ 減 ] g 減渡温測港湖湯湾満準源滅漢演漫潟激濃瀬火災炎点為無然焼煮煙照熊熱燃爆 (1226 . -4 . -5)\n"
	"[ 爪 ] h 父片版牛物特犬犯状狙独猫率玉王現球理環甘生産用田由甲申男町画界留略異番 (1201 . -4 . -9)\n"
	"[ 疋 ] k 疑疲病症痛療癒発登白百的皆皮益盛監盤目直相省県真眠着督知短石砂研破確 (1040 . -6 . -15)\n"
	"[ 示 ] l 示礼社祈祉祐祖祝神祥票祭視禁福秀私秋科秒秘称移程税稚種稲穀稼稽稿穂積穏穫穴究空突窓 (1881 . 12 . -3)\n"
	"[ 立 ] ; 立童端競竹笑第筆等筋答策節算管箱範築簡籍米粉粧精糖系紀約紅納純紙級素索細紹終組経紫 (1261 . 0 . -3)\n"
	"[ 結 ] z 結絞絡給統絵絶継続維網綺総緑緒練緊線締編縁縄繁縮績繋織繰罪置署美群義 (1346 . 4 . 1)\n"
	"[ 羽 ] x 羽翌習老考者耐耳聖聞聴職肉肌肩育背胸能脂脱脳腐腕腰腹臨自臭至致興舗舞航般船良色 (1219 . -2 . -1)\n"
	"[ 艸 ] c 花芸英若苦茶草荷華著菓菜落葉蔵薬藤虫融血行術街衛衣表袋被装裏補製複西要見規覚親覧観 (1004 . -5 . -6)\n"
	"[ 角 ] v 角解触言計討記訪設許訳診証評詞試詰話該詳認誌誕誘語誤説読調誰課談請論諸講謝識警議護 (1113 . -5 . -11)\n"
	"[ 谷 ] b 谷豆豊豚象豪負財販貧貨責貯貰貴買貸費貼賀賃資賛賞質賢購贈赤走起超越趣 (1388 . 2 . -9)\n"
	"[ 足 ] n 足距跡路踊踏躍身車軍転軽較載輝輩輪輸辛辞農辺込迎近返迫述迷追退送逃逆 (1209 . 0 . -9)\n"
	"[ 通 ] m 通透途速造連週進遅遊運過道達違遠遣適選遺避還那邪郎郡部郵郷都配酒酢酸 (1316 . 2 . -7)\n"
	"[ 里 ] , 里重野量金針釣鉄銀銘録鍋鏡鑑長門閉開間関閲闘阪防限降院除陰陸険陽隊階随際障隠隣 (1257 . 0 . -7)\n"
	"[ 隹 ] . 雄雅集雇雑難離雨雪雰雲電震霊露青静非面革靴韓音章響頂頃項順須預頑領頭頼題額顔類願顧 (1367 . 4 . -3)\n"
	"[ 風 ] / 風飛食飯飲飼飾養館首香馬駄駅駆駐騒験驚骨高髪鬼魂魅魔魚鮮鳥鳴鶏鹿麗麦麺麻黄黒鼻齢龍 (1380 . 2 . -1)\n"
	"各行平均位: 1252.9\n"
	"目標位:  1261\n"
	"合計漢字数:  1250\n"
	"exit: 0\n"
},

};

int main(void)
{
	size_t test_i;
	for (test_i = 0; test_i < sizeof(test_cases) / sizeof(*test_cases);
	     test_i++) {
		char actual_fn[512];
		FILE *actual;
		int exit_code, arg_count;

		if (sizeof(actual_fn) <=
		    snprintf(actual_fn, sizeof(actual_fn),
			     "last_rank_contained.%s.testout",
			     test_cases[test_i].name)) {
			fprintf(stderr, "バファが短すぎます\n");
			exit(1);
		}
		actual = xfopen(actual_fn, "w");
		out = err = actual;
		for (arg_count = 0; test_cases[test_i].args[arg_count];
		     arg_count++) {}
		exit_code = print_last_rank_contained(
			test_cases[test_i].args, arg_count);
		fprintf(actual, "exit: %d\n", exit_code);
		xfclose(actual);
		out = stdout;
		err = stderr;

		verify_contents(actual_fn, test_cases[test_i].expected);
	}
	return 0;
}
