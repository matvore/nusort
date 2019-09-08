#include "commands.h"
#include "test_util.h"
#include "util.h"

#include <stdlib.h>

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
		"会", "信", "刀", "匸", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "日", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	"[ 一 ] 1 丁了万丈上下与丸久中予不世主両争乱事並乳乗乾互井亡交京人入今元介公仕他付代令以兄 (1142 . -3 . -3)\n"
	"[ 会 ] 2 会全共充先光仮仰仲件任企伊休伝作児兵伴伸伺似但位低住佐体何余使免舎併例具典供依価 (1552 . 3 . 0)\n"
	"[ 信 ] 3 信便係促保個修俺倉倍倒候借値党健停側備働催傷傾像僕億優償内円冊再最写冬冷凄凍処出 (1289 . 1 . 1)\n"
	"[ 刀 ] 4 分切刊列初判別利到制刷券刺刻則削前剤副割創劇力功加助努労効勉動務勝募勤勢勧包化北 (1150 . -1 . 0)\n"
	"[ 匸 ] 5 区医千午半卒協南博占印危即原去参又及友反収取受単営厳口可古台史右号司合各吉同名向 (969 . -3 . -3)\n"
	"[ 告 ] 6 告君否含吸吹呂味周呼命和品咲員唱商問善喜喫器噌回因団困囲図固国圏園土圧在地坂均坊 (1400 . 4 . 1)\n"
	"[ 型 ] 7 型城域執基埼堂報場塗塚塩塾境増壁壊士声売変夏夕外多夜夢大天太夫央失奇奈奏契奥奨奮 (1303 . 3 . 4)\n"
	"[ 女 ] 8 女奴好妙始妹妻姉委姿姫娘婚婦嫌嬉子字存季学宅宇守安完宗官宙定宜宝実客宣室宮害家容 (1224 . 0 . 4)\n"
	"[ 宿 ] 9 宿寂寄密富寒寝察寧審寺対寿封専将射導小少就尾局居届屋展属層履山岐岡岩岸島崎崩川州 (1339 . 2 . 6)\n"
	"[ 工 ] 0 工左巨差己巻市布希師席帯帰帳常幅幌幕干平年幸幹幼庁広床底店府度座庫庭康廃延建弁弊 (1237 . 0 . 6)\n"
	"[ 弋 ] q 式引弱張強弾当形彩影役彼待律後徒従得御復微徳徴心必志忘忙応快念怒怖思急性怪恋恐息恵 (1120 . -2 . 4)\n"
	"[ 悪 ] w 悪悟悠患悩悲情惑惜惨惹想愉意愚愛感慌態慎慮慢慣慰慶憂憧憩憲憶懐懸成我戒戦戯戴 (1811 . 18 . 22)\n"
	"[ 戸 ] r 戸戻房所手打払扱承技投抗折抜択抱押担拠拡持指挑挙振授掛採探接推掲描揃 (1066 . -7 . 15)\n"
	"[ 提 ] t 提揚換握援損携摘撃撮操支改攻放政故救敗教散数敵敷整文料断新方施旅族既 (1222 . 0 . 15)\n"
	"[ 日 ] y 日旧早曲旨旬更昇明易昔星映春昨昭是昼時書普替景晴暑暇暖暗暮暴曜月有服望朝期 (1131 . -2 . 13)\n"
	"[ 木 ] p 木未末本札材村束条来果杯東松板析林枚柄染査栄校株根格案桜梅森棚植検極 (1156 . -2 . 11)\n"
	"[ 業 ] s 業楽概構様模標権横樹橋機欄欠次欲歌止正武歩歯歳歴死残段殺母毎比毛氏民 (1023 . -2 . 9)\n"
	"[ 气 ] d 気水永求江池決沖沢河油治況泉泊法波泣注活洋洗津海派流浜浮浴消済液深混清 (999 . -10 . -1)\n"
	"[ 減 ] g 減渡温測港湖湯湾満準源滅漢演漫潟激濃瀬火災炎点為無然焼煮煙照熊熱燃爆 (1226 . -4 . -5)\n"
	"[ 爪 ] h 父片版牛物特犬犯状狙独猫率玉王現球理環甘生産用田由甲申男町画界留略異番 (1201 . -4 . -9)\n"
	"[ 疋 ] k 疑疲病症痛療癒発登白百的皆皮益盛監盤目直相省県真眠着督知短石砂研破確 (1040 . -6 . -15)\n"
	"[ 示 ] l 示礼社祈祉祐祖祝神祥票祭視禁福秀私秋科秒秘称移程税稚種稲穀稼稽稿穂積穏穫穴究空突窓 (1881 . 12 . -3)\n"
	"[ 立 ] ; 立竜童端競竹笑第筆等筋答策節算管箱範築簡籍米粉粧精糖系紀約紅納純紙級素索細紹終組経 (1306 . 1 . -2)\n"
	"[ 結 ] z 結紫絞絡給統絵絶継続維網綺総緑緒練緊線締編縁縄繁縮績繋織繰罪置美群義 (1338 . 3 . 1)\n"
	"[ 羽 ] x 羽翌習老考者耐耳聖聞聴職肉肌肩育背胸能脂脱脳腐腕腰腹臨自臭至致興舗舞航般船良色 (1219 . -2 . -1)\n"
	"[ 艸 ] c 花芸英若苦茶草荷華著菓菜落葉蔵薬藤虫融血行術街衛衣表袋被装裏補製複西要見規覚親覧観 (1004 . -5 . -6)\n"
	"[ 角 ] v 角解触言計討記訪設許訳診証評詞試詰話該詳認誌誕誘語誤説読調誰課談請論諸講謝識警議護 (1113 . -5 . -11)\n"
	"[ 谷 ] b 谷豆豊豚象豪負財販貧貨責貯貰貴買貸費貼賀賃資賛賞質賢購贈赤走起超越趣 (1388 . 2 . -9)\n"
	"[ 足 ] n 足距跡路踊踏躍身車軍転軽較載輝輩輪輸辛辞農辺込迎近返述迫迷追退送逃逆 (1209 . 0 . -9)\n"
	"[ 通 ] m 通透途速造連週進遅遊運過道達違遠遣適選遺避還那邪郎郡部郵郷都配酒酢酸 (1316 . 2 . -7)\n"
	"[ 里 ] , 里重野量金針釣鉄銀銘録鍋鏡鑑長門閉開間関閲闘阪防限降院除陰陸険陽隊階随際障隠隣 (1257 . 0 . -7)\n"
	"[ 隹 ] . 雄雅集雇雑難離雨雪雰雲電震霊露青静非面革靴韓音章響頂頃項順須預頑領頭頼題額顔類願顧 (1367 . 4 . -3)\n"
	"[ 風 ] / 風飛食飯飲飼飾養館首香馬駄駅駆駐騒験驚骨高髪鬼魂魅魔魚鮮鳥鳴鶏鹿麗麦麺麻黄黒鼻齢龍 (1380 . 2 . -1)\n"
	"各行平均位: 1253.9\n"
	"目標位:  1261\n"
	"合計漢字数:  1250\n"
	"exit: 0\n"
},

{
	"typical_kugiri_kanji_test_rank_sort",
	{
		"会", "信", "刀", "匸", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "日", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	"[ 一 ] 1 人事入中上今下以付他不代予仕世介公主元京交万了乗久井並両与丸争丁丈互乳兄令乱乾亡 (1142 . -3 . -3)\n"
	"[ 会 ] 2 会作使全体価何先件住供位休企伝光共例具任低似児仲充典余免佐伊依兵伸仮舎伴併伺但仰 (1552 . 3 . 0)\n"
	"[ 信 ] 3 出最内信保写円像個備便係優再値健修処僕側働俺冷催倍倒党停借倉冬傷候凄冊億償傾促凍 (1289 . 1 . 1)\n"
	"[ 刀 ] 4 分前動利力加化初切別務制北勝効判割募助削労券勢勤勉創則功到列包勧劇剤刺刊刻刷努副 (1150 . -1 . 0)\n"
	"[ 匸 ] 5 合取名同可参受向単原営口友古号各半収台及反医区去協南千印史右午吉又司占厳危卒博即 (969 . -3 . -3)\n"
	"[ 告 ] 6 品商地問国回味在員図告和含周土園器団命君囲呼善困喜因圧固呂吸坂吹咲均否喫坊圏唱噌 (1400 . 4 . 1)\n"
	"[ 型 ] 7 大報場変外売多天基域夏夜型声士境増太夫失夢城夕堂奈奥契奏壊塩央壁塗奇塚奨埼執塾奮 (1303 . 3 . 4)\n"
	"[ 女 ] 8 定子学実家安好女容客始字宅存室完害宮婚守委嫌嬉季姿婦娘官宝妙宇姉宣宜妻姫奴宗妹宙 (1224 . 0 . 4)\n"
	"[ 宿 ] 9 対小山屋少川届島専宿展寄局富岡居寝導属崎州密寺察就岩将履寒尾層審寿射岸寧崩封岐寂 (1339 . 2 . 6)\n"
	"[ 工 ] 0 年店度市常広帯庫平工帰建座康師希差幸布府左巻庭席帳弁幅己底干幼延廃床幹弊幕庁巨幌 (1237 . 0 . 6)\n"
	"[ 弋 ] q 思当後性心必引応強式得待役張形彼念御急影忘復快恋徒息従恵律恐志弾弱徴怖微忙彩怒徳怪 (1120 . -2 . 4)\n"
	"[ 悪 ] w 情感成意戦愛想悪態我悩憶惑慮慣慢悲患懐懸憲悟憧憩慶慎憂愚戯惜愉戒惨戴慰悠慌惹 (1811 . 18 . 22)\n"
	"[ 戸 ] r 手所持探投掲指戻択技払承振接戸担扱打押抜拡採掛描推授挙折房揃抱拠挑抗 (1066 . -7 . 15)\n"
	"[ 提 ] t 方新料文数教旅支施提政携断換放改援整撮族撃故敵散攻敗操損既敷救握揚摘 (1222 . 0 . 15)\n"
	"[ 日 ] y 日時書月明期更有映早朝望曲昨普替曜景春星暮服晴昼昔是暗暑昇易昭旧暇暖旬暴旨 (1131 . -2 . 13)\n"
	"[ 木 ] p 本検来格果東条材校案木村株板査札未末松根極森枚植林柄梅桜染析栄杯束棚 (1156 . -2 . 11)\n"
	"[ 業 ] s 業楽様機次正民残歩構毎止比権段歌死標欲氏母歳横歴橋毛歯武殺模概欠樹欄 (1023 . -2 . 9)\n"
	"[ 气 ] d 気法注活水決海流求済消治況泉沢泊派深洗洋波浜沖江油混池清津泣浴液河永浮 (999 . -10 . -1)\n"
	"[ 減 ] g 無点然満演温焼準為熱火減激源渡照湯港爆測災漫濃瀬燃煙煮湖漢潟湾炎滅熊 (1226 . -4 . -5)\n"
	"[ 爪 ] h 用生物理画現特番産田状界版町男申由異環球王率独犬父玉留略猫甘片牛犯甲狙 (1201 . -4 . -9)\n"
	"[ 疋 ] k 発的目知登真確着相県直白研病皆石療痛省盛症疲短破監百盤眠疑益癒皮督砂 (1040 . -6 . -15)\n"
	"[ 示 ] l 示社私空種稿神科究移程福税視積票禁突祭称秋秘礼稼祝窓穴祉秀秒祈稲稚祖穂穫祥祐穏稽穀 (1881 . 12 . -3)\n"
	"[ 立 ] ; 索立約等第経細組笑素終紹管答簡系策米納算級築紙籍節精端競筋紀箱純範粉筆紅竹童粧糖竜 (1306 . 1 . -2)\n"
	"[ 結 ] z 結美続線置総編絞絡義絵絶緒給練統織継縄締群績緑維綺縮罪繰緊縁紫網繋繁 (1338 . 3 . 1)\n"
	"[ 羽 ] x 自者能考良色育聞職習致舗肉般興聴舞肌背船老腹航脱脳羽翌耳腕臭脂至聖耐胸腐臨肩腰 (1219 . -2 . -1)\n"
	"[ 艸 ] c 見行表要覧規花落観術製葉著西親英覚装茶薬苦若藤補荷街複芸菜裏華血草袋融蔵被菓衛衣虫 (1004 . -5 . -6)\n"
	"[ 角 ] v 記言話認設読詳語解調計評説試議証護識談論誰講訳許請角誌訪課診討該謝誕触警詰誘詞諸誤 (1113 . -5 . -11)\n"
	"[ 谷 ] b 買質資販購象起費負赤豊走越超貸賃谷責賞豆財貴趣貨賀貼贈貯豪貰豚貧賛賢 (1388 . 2 . -9)\n"
	"[ 足 ] n 送近込車載追返身転足農辺軽路較迎退軍逆迷輸逃輪躍辞跡辛述踏距輩輝迫踊 (1209 . 0 . -9)\n"
	"[ 通 ] m 通部選連配道達違運過都週進適速造遊酒途郎遠遅遣郵酸遺邪那避郡透郷還酢 (1316 . 2 . -7)\n"
	"[ 里 ] , 間関開金録長野際重限除院門量降険防銀阪鉄障階閉釣闘隊陽里針陸隣隠閲鑑鍋銘鏡随陰 (1257 . 0 . -7)\n"
	"[ 隹 ] . 集電面題願音順頭類非頂項顔難雑額頃離雨青頼頑章響韓静雪革雰震領露須雄雲霊靴顧預雇雅 (1367 . 4 . -3)\n"
	"[ 風 ] / 高食風験駅館飲馬飛香黒魔養飯魚首鳥鮮魅麗齢髪鹿驚骨黄駄駐麻飼飾鳴駆麺龍麦鬼鼻騒鶏魂 (1380 . 2 . -1)\n"
	"各行平均位: 1253.9\n"
	"目標位:  1261\n"
	"合計漢字数:  1250\n"
	"exit: 0\n"
},

{
	"kanji_not_in_db",
	{
		"会", "信", "刀", "匸", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "话", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	"[ 话 ] は区切り漢字に指定されているけれど、KANJI配列に含まれていない。\n"
	"exit: 2\n",
},

{
	"kanji_not_valid_kugiri",
	{
		"会", "信", "刀", "匸", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "俯", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	"[ 俯 ] は区切り漢字として使えません。\n"
	"exit: 3\n",
},

{
	"automaticly_find_kugiri",
	{},
	"[ 一 ] 1 人事入中上今下以付他不代予仕世介公主元京交万了乗久井並両与丸争丁丈互乳兄令乱乾亡 (1142 . -3 . -3)\n"
	"[ 会 ] 2 会作使全体価何先件住供位休企伝光共例具任低似児仲充典余免佐伊依兵伸仮舎伴併伺但仰 (1552 . 3 . 0)\n"
	"[ 信 ] 3 出最内信保写円像個備便係優再値健修処僕側働俺冷催倍倒党停借倉冬傷候凄冊億償傾促凍 (1289 . 1 . 1)\n"
	"[ 刀 ] 4 分前動利力加化初切別務制北勝効判割募助削労券勢勤勉創則功到列包勧劇剤刺刊刻刷努副 (1150 . -1 . 0)\n"
	"[ 匸 ] 5 取可参受単原営口友古号半収台及反医区去協南千印史右午又司占厳危卒博即厚卵句却匹双 (1371 . 3 . 3)\n"
	"[ 合 ] 6 品合商地問国回名味同在員向図告和含各周土園器団命君囲呼善吉困喜因圧固呂吸坂吹咲均 (1025 . -2 . 1)\n"
	"[ 型 ] 7 大報場変外売多天好女基域夏夜型声士境増太夫失夢城夕堂奈奥契奏壊塩央壁妙塗奇奴塚奨 (1249 . -1 . 0)\n"
	"[ 始 ] 8 定子学実家安容客始字宅存室宿完害寄宮婚守委嫌嬉季姿密婦娘官宝宇姉宣宜妻姫宗妹宙宴 (1309 . 1 . 1)\n"
	"[ 富 ] 9 対小山屋少川届島工専展局富岡居寝差導属崎左巻州寺察就岩己将履寒尾層審寿射岸寧巨崩 (1238 . -1 . 0)\n"
	"[ 巾 ] 0 年店度市引常広強式帯庫平帰張建座康師希幸布府庭席帳弁幅弱底干幼延廃床幹弟弊幕庁幌 (1237 . 0 . 0)\n"
	"[ 弾 ] q 思当後性心必応得待役形悪彼念御急影忘復快恋徒悩息従恵律恐志弾徴怖微忙患彩怒徳怪徹恥 (1298 . 1 . 1)\n"
	"[ 悲 ] w 情手感成意所投戦愛戻想択技払態承戸担扱打押抜我拡折憶房惑慮抱慣拠慢悲抗才懐懸 (1170 . 0 . 1)\n"
	"[ 持 ] r 持探掲指支提携換放改振接援撮撃採掛描攻推授挙操損揃挑捨掃括排捕握揚摘 (1222 . -1 . 0)\n"
	"[ 政 ] t 日方新料文数明教更旅施早政断曲整族故敵散敗昔昇既易旧敷救旬旨旦敬斜敏 (1522 . 3 . 3)\n"
	"[ 星 ] y 本時書来月期有条材映木村朝望昨普替札未末曜景春星暮服晴昼是暗暑昭暇暖暴束晩 (1211 . 0 . 3)\n"
	"[ 果 ] p 業楽検様機格果東校案構株板権査標横松根極森枚橋模概植林柄梅桜染析栄樹 (1019 . -4 . -1)\n"
	"[ 欄 ] s 気水次正民求残歩毎止比段歌死欲氏母歳歴毛歯武殺江池永欠汗欄汚歓汁毒欧 (1297 . 1 . 0)\n"
	"[ 決 ] d 法注活決海流済消治況泉沢泊派深洗洋波浜沖油混清津泣浴液河浮沿添涙浦涼渋 (1149 . -1 . -1)\n"
	"[ 減 ] g 無点然満演温焼準為火減激源渡照湯港測災漫濃瀬煙煮湖漢潟湾炎滅滞漏炭滑 (1267 . 1 . 0)\n"
	"[ 熊 ] h 用生物理現特産田状版申由環球熱王率独犬父玉猫甘爆片牛犯燃甲狙獲熊献玄珍 (1240 . -1 . -1)\n"
	"[ 男 ] k 発的目画登番相界県直白町男病皆異療痛省盛症留略疲監百盤疑益癒皮看皇盟 (1307 . 2 . 1)\n"
	"[ 真 ] l 示社私知真確着種稿神研科究石移程福税視積票禁祭称秋短秘破礼稼眠祝督砂瞬礎眼磨穴祉秀 (1164 . -1 . 0)\n"
	"[ 空 ] ; 索立約等第経細組笑素終空紹管答簡系策米納算級築紙籍節精突端競筋紀箱純窓範粉筆紅竹童 (1143 . -2 . -2)\n"
	"[ 結 ] z 結美続線置総編絞絡義絵絶緒給練統織継縄締群績羽緑維綺縮罪繰緊縁紫網繋 (1317 . 2 . 0)\n"
	"[ 翌 ] x 自者能考良育聞職習致舗肉般興聴舞肌背船老腹航脱脳翌耳腕臭脂至聖耐胸腐臨肩腰脚臣 (1258 . 0 . 0)\n"
	"[ 色 ] c 行表要色花落術製葉著西英装茶薬苦若藤補荷街複芸菜裏華血草袋融蔵被菓衛衣虫薄裁荒裕薦 (1260 . 0 . 0)\n"
	"[ 見 ] v 見記言話覧認設読詳語解規計評説観試証親覚訳許角誌訪診討該誕触詰誘詞誤詩託訴誠訓誇訂 (1459 . 3 . 3)\n"
	"[ 調 ] b 買質調資販象議護識費談負論豊誰講貸賃谷責賞請課豆財貴貨謝警賀貼諸貯豪 (1167 . -4 . -1)\n"
	"[ 購 ] n 近込車載返身購転足農起辺軽路赤較走越超迎軍輸趣輪躍辞跡辛贈踏距輩輝踊 (1209 . 0 . -1)\n"
	"[ 述 ] m 送通選連道追達違運過週進適速造遊途郎遠遅遣退逆迷逃遺邪那述避郡迫透還 (1299 . 1 . 0)\n"
	"[ 部 ] , 間関部開金録長配野重限都除院門量降酒険防銀阪鉄閉釣闘里郵酸針陸閲鑑郷鍋銘鏡陰鈴 (1268 . 1 . 1)\n"
	"[ 陽 ] . 集電面音際順頭非頂項難雑頃離雨青頼頑章響障階韓静雪隊陽革雰震領露隣隠須雄雲霊随靴預 (1321 . 1 . 2)\n"
	"[ 題 ] / 高食題風願験類駅館飲顔馬額飛香黒魔養飯魚首鳥鮮魅麗齢髪鹿驚骨黄駄駐麻飼飾鳴駆麺龍麦 (1231 . -3 . -1)\n"
	"各行平均位: 1253.3\n"
	"目標位:  1261\n"
	"合計漢字数:  1250\n"
	"exit: 0\n"
},

{
	"typical_kugiri_kanji_with_showing_count_test",
	{
		"-s", "-n",
		"会", "信", "刀", "匸", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "日", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	"[ 一 ] 1 丁了万丈上下与丸久中予不世主両争乱事並乳乗乾互井亡交京人入今元介公仕他付代令以兄 (1142 . -3 . -3 . 40)\n"
	"[ 会 ] 2 会全共充先光仮仰仲件任企伊休伝作児兵伴伸伺似但位低住佐体何余使免舎併例具典供依価 (1552 . 3 . 0 . 40)\n"
	"[ 信 ] 3 信便係促保個修俺倉倍倒候借値党健停側備働催傷傾像僕億優償内円冊再最写冬冷凄凍処出 (1289 . 1 . 1 . 40)\n"
	"[ 刀 ] 4 分切刊列初判別利到制刷券刺刻則削前剤副割創劇力功加助努労効勉動務勝募勤勢勧包化北 (1150 . -1 . 0 . 40)\n"
	"[ 匸 ] 5 区医千午半卒協南博占印危即原去参又及友反収取受単営厳口可古台史右号司合各吉同名向 (969 . -3 . -3 . 40)\n"
	"[ 告 ] 6 告君否含吸吹呂味周呼命和品咲員唱商問善喜喫器噌回因団困囲図固国圏園土圧在地坂均坊 (1400 . 4 . 1 . 40)\n"
	"[ 型 ] 7 型城域執基埼堂報場塗塚塩塾境増壁壊士声売変夏夕外多夜夢大天太夫央失奇奈奏契奥奨奮 (1303 . 3 . 4 . 40)\n"
	"[ 女 ] 8 女奴好妙始妹妻姉委姿姫娘婚婦嫌嬉子字存季学宅宇守安完宗官宙定宜宝実客宣室宮害家容 (1224 . 0 . 4 . 40)\n"
	"[ 宿 ] 9 宿寂寄密富寒寝察寧審寺対寿封専将射導小少就尾局居届屋展属層履山岐岡岩岸島崎崩川州 (1339 . 2 . 6 . 40)\n"
	"[ 工 ] 0 工左巨差己巻市布希師席帯帰帳常幅幌幕干平年幸幹幼庁広床底店府度座庫庭康廃延建弁弊 (1237 . 0 . 6 . 40)\n"
	"[ 弋 ] q 式引弱張強弾当形彩影役彼待律後徒従得御復微徳徴心必志忘忙応快念怒怖思急性怪恋恐息恵 (1120 . -2 . 4 . 41)\n"
	"[ 悪 ] w 悪悟悠患悩悲情惑惜惨惹想愉意愚愛感慌態慎慮慢慣慰慶憂憧憩憲憶懐懸成我戒戦戯戴 (1811 . 18 . 22 . 38)\n"
	"[ 戸 ] r 戸戻房所手打払扱承技投抗折抜択抱押担拠拡持指挑挙振授掛採探接推掲描揃 (1066 . -7 . 15 . 34)\n"
	"[ 提 ] t 提揚換握援損携摘撃撮操支改攻放政故救敗教散数敵敷整文料断新方施旅族既 (1222 . 0 . 15 . 34)\n"
	"[ 日 ] y 日旧早曲旨旬更昇明易昔星映春昨昭是昼時書普替景晴暑暇暖暗暮暴曜月有服望朝期 (1131 . -2 . 13 . 37)\n"
	"[ 木 ] p 木未末本札材村束条来果杯東松板析林枚柄染査栄校株根格案桜梅森棚植検極 (1156 . -2 . 11 . 34)\n"
	"[ 業 ] s 業楽概構様模標権横樹橋機欄欠次欲歌止正武歩歯歳歴死残段殺母毎比毛氏民 (1023 . -2 . 9 . 34)\n"
	"[ 气 ] d 気水永求江池決沖沢河油治況泉泊法波泣注活洋洗津海派流浜浮浴消済液深混清 (999 . -10 . -1 . 35)\n"
	"[ 減 ] g 減渡温測港湖湯湾満準源滅漢演漫潟激濃瀬火災炎点為無然焼煮煙照熊熱燃爆 (1226 . -4 . -5 . 34)\n"
	"[ 爪 ] h 父片版牛物特犬犯状狙独猫率玉王現球理環甘生産用田由甲申男町画界留略異番 (1201 . -4 . -9 . 35)\n"
	"[ 疋 ] k 疑疲病症痛療癒発登白百的皆皮益盛監盤目直相省県真眠着督知短石砂研破確 (1040 . -6 . -15 . 34)\n"
	"[ 示 ] l 示礼社祈祉祐祖祝神祥票祭視禁福秀私秋科秒秘称移程税稚種稲穀稼稽稿穂積穏穫穴究空突窓 (1881 . 12 . -3 . 41)\n"
	"[ 立 ] ; 立竜童端競竹笑第筆等筋答策節算管箱範築簡籍米粉粧精糖系紀約紅納純紙級素索細紹終組経 (1306 . 1 . -2 . 41)\n"
	"[ 結 ] z 結紫絞絡給統絵絶継続維網綺総緑緒練緊線締編縁縄繁縮績繋織繰罪置美群義 (1338 . 3 . 1 . 34)\n"
	"[ 羽 ] x 羽翌習老考者耐耳聖聞聴職肉肌肩育背胸能脂脱脳腐腕腰腹臨自臭至致興舗舞航般船良色 (1219 . -2 . -1 . 39)\n"
	"[ 艸 ] c 花芸英若苦茶草荷華著菓菜落葉蔵薬藤虫融血行術街衛衣表袋被装裏補製複西要見規覚親覧観 (1004 . -5 . -6 . 41)\n"
	"[ 角 ] v 角解触言計討記訪設許訳診証評詞試詰話該詳認誌誕誘語誤説読調誰課談請論諸講謝識警議護 (1113 . -5 . -11 . 41)\n"
	"[ 谷 ] b 谷豆豊豚象豪負財販貧貨責貯貰貴買貸費貼賀賃資賛賞質賢購贈赤走起超越趣 (1388 . 2 . -9 . 34)\n"
	"[ 足 ] n 足距跡路踊踏躍身車軍転軽較載輝輩輪輸辛辞農辺込迎近返述迫迷追退送逃逆 (1209 . 0 . -9 . 34)\n"
	"[ 通 ] m 通透途速造連週進遅遊運過道達違遠遣適選遺避還那邪郎郡部郵郷都配酒酢酸 (1316 . 2 . -7 . 34)\n"
	"[ 里 ] , 里重野量金針釣鉄銀銘録鍋鏡鑑長門閉開間関閲闘阪防限降院除陰陸険陽隊階随際障隠隣 (1257 . 0 . -7 . 39)\n"
	"[ 隹 ] . 雄雅集雇雑難離雨雪雰雲電震霊露青静非面革靴韓音章響頂頃項順須預頑領頭頼題額顔類願顧 (1367 . 4 . -3 . 41)\n"
	"[ 風 ] / 風飛食飯飲飼飾養館首香馬駄駅駆駐騒験驚骨高髪鬼魂魅魔魚鮮鳥鳴鶏鹿麗麦麺麻黄黒鼻齢龍 (1380 . 2 . -1 . 41)\n"
	"各行平均位: 1253.9\n"
	"目標位:  1261\n"
	"合計漢字数:  1250\n"
	"exit: 0\n"
},

{
	"automaticly_find_kugiri_hide_kanji",
	{"-k"},
	"[ 一 ] 1 (1142 . -3 . -3)\n"
	"[ 会 ] 2 (1552 . 3 . 0)\n"
	"[ 信 ] 3 (1289 . 1 . 1)\n"
	"[ 刀 ] 4 (1150 . -1 . 0)\n"
	"[ 匸 ] 5 (1371 . 3 . 3)\n"
	"[ 合 ] 6 (1025 . -2 . 1)\n"
	"[ 型 ] 7 (1249 . -1 . 0)\n"
	"[ 始 ] 8 (1309 . 1 . 1)\n"
	"[ 富 ] 9 (1238 . -1 . 0)\n"
	"[ 巾 ] 0 (1237 . 0 . 0)\n"
	"[ 弾 ] q (1298 . 1 . 1)\n"
	"[ 悲 ] w (1170 . 0 . 1)\n"
	"[ 持 ] r (1222 . -1 . 0)\n"
	"[ 政 ] t (1522 . 3 . 3)\n"
	"[ 星 ] y (1211 . 0 . 3)\n"
	"[ 果 ] p (1019 . -4 . -1)\n"
	"[ 欄 ] s (1297 . 1 . 0)\n"
	"[ 決 ] d (1149 . -1 . -1)\n"
	"[ 減 ] g (1267 . 1 . 0)\n"
	"[ 熊 ] h (1240 . -1 . -1)\n"
	"[ 男 ] k (1307 . 2 . 1)\n"
	"[ 真 ] l (1164 . -1 . 0)\n"
	"[ 空 ] ; (1143 . -2 . -2)\n"
	"[ 結 ] z (1317 . 2 . 0)\n"
	"[ 翌 ] x (1258 . 0 . 0)\n"
	"[ 色 ] c (1260 . 0 . 0)\n"
	"[ 見 ] v (1459 . 3 . 3)\n"
	"[ 調 ] b (1167 . -4 . -1)\n"
	"[ 購 ] n (1209 . 0 . -1)\n"
	"[ 述 ] m (1299 . 1 . 0)\n"
	"[ 部 ] , (1268 . 1 . 1)\n"
	"[ 陽 ] . (1321 . 1 . 2)\n"
	"[ 題 ] / (1231 . -3 . -1)\n"
	"各行平均位: 1253.3\n"
	"目標位:  1261\n"
	"合計漢字数:  1250\n"
	"exit: 0\n"
},

{
	"typical_kugiri_kanji_with_showing_count_and_hide_kanji_test",
	{
		"-k", "-n",
		"会", "信", "刀", "匸", "告", "型", "女", "宿", "工", "弋",
		"悪", "戸", "提", "日", "木", "業", "气", "減", "爪", "疋",
		"示", "立", "結", "羽", "艸", "角", "谷", "足", "通", "里",
		"隹", "風",
	},
	"[ 一 ] 1 (1142 . -3 . -3 . 40)\n"
	"[ 会 ] 2 (1552 . 3 . 0 . 40)\n"
	"[ 信 ] 3 (1289 . 1 . 1 . 40)\n"
	"[ 刀 ] 4 (1150 . -1 . 0 . 40)\n"
	"[ 匸 ] 5 (969 . -3 . -3 . 40)\n"
	"[ 告 ] 6 (1400 . 4 . 1 . 40)\n"
	"[ 型 ] 7 (1303 . 3 . 4 . 40)\n"
	"[ 女 ] 8 (1224 . 0 . 4 . 40)\n"
	"[ 宿 ] 9 (1339 . 2 . 6 . 40)\n"
	"[ 工 ] 0 (1237 . 0 . 6 . 40)\n"
	"[ 弋 ] q (1120 . -2 . 4 . 41)\n"
	"[ 悪 ] w (1811 . 18 . 22 . 38)\n"
	"[ 戸 ] r (1066 . -7 . 15 . 34)\n"
	"[ 提 ] t (1222 . 0 . 15 . 34)\n"
	"[ 日 ] y (1131 . -2 . 13 . 37)\n"
	"[ 木 ] p (1156 . -2 . 11 . 34)\n"
	"[ 業 ] s (1023 . -2 . 9 . 34)\n"
	"[ 气 ] d (999 . -10 . -1 . 35)\n"
	"[ 減 ] g (1226 . -4 . -5 . 34)\n"
	"[ 爪 ] h (1201 . -4 . -9 . 35)\n"
	"[ 疋 ] k (1040 . -6 . -15 . 34)\n"
	"[ 示 ] l (1881 . 12 . -3 . 41)\n"
	"[ 立 ] ; (1306 . 1 . -2 . 41)\n"
	"[ 結 ] z (1338 . 3 . 1 . 34)\n"
	"[ 羽 ] x (1219 . -2 . -1 . 39)\n"
	"[ 艸 ] c (1004 . -5 . -6 . 41)\n"
	"[ 角 ] v (1113 . -5 . -11 . 41)\n"
	"[ 谷 ] b (1388 . 2 . -9 . 34)\n"
	"[ 足 ] n (1209 . 0 . -9 . 34)\n"
	"[ 通 ] m (1316 . 2 . -7 . 34)\n"
	"[ 里 ] , (1257 . 0 . -7 . 39)\n"
	"[ 隹 ] . (1367 . 4 . -3 . 41)\n"
	"[ 風 ] / (1380 . 2 . -1 . 41)\n"
	"各行平均位: 1253.9\n"
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
		int arg_count, exit_code;

		start_test(__FILE__, test_cases[test_i].name);

		for (arg_count = 0; test_cases[test_i].args[arg_count];
		     arg_count++) {}
		exit_code = print_last_rank_contained(
			test_cases[test_i].args, arg_count);
		fprintf(out, "exit: %d\n", exit_code);

		end_test(test_cases[test_i].expected);
	}
	return 0;
}
