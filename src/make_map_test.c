#include "commands.h"
#include "test_util.h"

int main(void)
{
	start_test(__FILE__, "romazi_mapping");
	{
		make_map(NULL, 0);
	}
	end_test(
		"!	一\n"
		"@	二\n"
		"#	三\n"
		"$	四\n"
		"%	五\n"
		"^	六\n"
		"&	七\n"
		"*	八\n"
		"(	九\n"
		")	十\n"
		"j	っ\n"
		"J	ッ\n"
		"f	ん\n"
		"F	ン\n"
		"ba	ば\n"
		"BA	バ\n"
		"bi	び\n"
		"BI	ビ\n"
		"bu	ぶ\n"
		"BU	ブ\n"
		"be	べ\n"
		"BE	ベ\n"
		"bo	ぼ\n"
		"BO	ボ\n"
		"BW7	ヴャ\n"
		"BW8	ヴュ\n"
		"BW9	ヴョ\n"
		"BWA	ヴァ\n"
		"BWI	ヴィ\n"
		"BWU	ヴ\n"
		"BWE	ヴェ\n"
		"BWO	ヴォ\n"
		"bya	びゃ\n"
		"BYA	ビャ\n"
		"byi	びぃ\n"
		"BYI	ビィ\n"
		"byu	びゅ\n"
		"BYU	ビュ\n"
		"bye	びぇ\n"
		"BYE	ビェ\n"
		"byo	びょ\n"
		"BYO	ビョ\n"
		"kya	きゃ\n"
		"KYA	キャ\n"
		"kyi	きぃ\n"
		"KYI	キィ\n"
		"kyu	きゅ\n"
		"KYU	キュ\n"
		"kye	きぇ\n"
		"KYE	キェ\n"
		"kyo	きょ\n"
		"KYO	キョ\n"
		"sya	しゃ\n"
		"SYA	シャ\n"
		"syi	しぃ\n"
		"SYI	シィ\n"
		"syu	しゅ\n"
		"SYU	シュ\n"
		"sye	しぇ\n"
		"SYE	シェ\n"
		"syo	しょ\n"
		"SYO	ショ\n"
		"zya	じゃ\n"
		"ZYA	ジャ\n"
		"zyi	じぃ\n"
		"ZYI	ジィ\n"
		"zyu	じゅ\n"
		"ZYU	ジュ\n"
		"zye	じぇ\n"
		"ZYE	ジェ\n"
		"zyo	じょ\n"
		"ZYO	ジョ\n"
		"tya	ちゃ\n"
		"TYA	チャ\n"
		"tyi	ちぃ\n"
		"TYI	チィ\n"
		"tyu	ちゅ\n"
		"TYU	チュ\n"
		"tye	ちぇ\n"
		"TYE	チェ\n"
		"tyo	ちょ\n"
		"TYO	チョ\n"
		"DYA	ヂャ\n"
		"DYI	ヂィ\n"
		"DYU	ヂュ\n"
		"DYE	ヂェ\n"
		"DYO	ヂョ\n"
		"TSA	ツァ\n"
		"TSI	ツィ\n"
		"TSE	ツェ\n"
		"TSO	ツォ\n"
		"THA	テャ\n"
		"THI	ティ\n"
		"THU	テュ\n"
		"THE	テェ\n"
		"THO	テョ\n"
		"DHA	デャ\n"
		"DHI	ディ\n"
		"DHU	デュ\n"
		"DHE	デェ\n"
		"DHO	デョ\n"
		"TWA	トァ\n"
		"TWI	トィ\n"
		"TWU	トゥ\n"
		"TWE	トェ\n"
		"TWO	トォ\n"
		"DWA	ドァ\n"
		"DWI	ドィ\n"
		"DWU	ドゥ\n"
		"DWE	ドェ\n"
		"DWO	ドォ\n"
		"nya	にゃ\n"
		"NYA	ニャ\n"
		"nyi	にぃ\n"
		"NYI	ニィ\n"
		"nyu	にゅ\n"
		"NYU	ニュ\n"
		"nye	にぇ\n"
		"NYE	ニェ\n"
		"nyo	にょ\n"
		"NYO	ニョ\n"
		"pya	ぴゃ\n"
		"PYA	ピャ\n"
		"pyi	ぴぃ\n"
		"PYI	ピィ\n"
		"pyu	ぴゅ\n"
		"PYU	ピュ\n"
		"pye	ぴぇ\n"
		"PYE	ピェ\n"
		"pyo	ぴょ\n"
		"PYO	ピョ\n"
		"ha	は\n"
		"HA	ハ\n"
		"hi	ひ\n"
		"HI	ヒ\n"
		"hu	ふ\n"
		"HU	フ\n"
		"he	へ\n"
		"HE	ヘ\n"
		"ho	ほ\n"
		"HO	ホ\n"
		"HW7	フャ\n"
		"HW8	フュ\n"
		"HW9	フョ\n"
		"HWA	ファ\n"
		"HWI	フィ\n"
		"HWE	フェ\n"
		"HWO	フォ\n"
		"HYA	ヒャ\n"
		"HYI	ヒィ\n"
		"HYU	ヒュ\n"
		"HYE	ヒェ\n"
		"HYO	ヒョ\n"
		"mya	みゃ\n"
		"MYA	ミャ\n"
		"myi	みぃ\n"
		"MYI	ミィ\n"
		"myu	みゅ\n"
		"MYU	ミュ\n"
		"mye	みぇ\n"
		"MYE	ミェ\n"
		"myo	みょ\n"
		"MYO	ミョ\n"
		"rya	りゃ\n"
		"RYA	リャ\n"
		"ryi	りぃ\n"
		"RYI	リィ\n"
		"ryu	りゅ\n"
		"RYU	リュ\n"
		"rye	りぇ\n"
		"RYE	リェ\n"
		"ryo	りょ\n"
		"RYO	リョ\n"
		"a	あ\n"
		"A	ア\n"
		"i	い\n"
		"I	イ\n"
		"u	う\n"
		"U	ウ\n"
		"e	え\n"
		"E	エ\n"
		"o	お\n"
		"O	オ\n"
		"XA	ァ\n"
		"XI	ィ\n"
		"XU	ゥ\n"
		"XE	ェ\n"
		"XO	ォ\n"
		"xxa	ぁ\n"
		"XXA	ァ\n"
		"xxi	ぃ\n"
		"XXI	ィ\n"
		"xxu	ぅ\n"
		"XXU	ゥ\n"
		"xxe	ぇ\n"
		"XXE	ェ\n"
		"xxo	ぉ\n"
		"XXO	ォ\n"
		"YE	イェ\n"
		"ka	か\n"
		"KA	カ\n"
		"ki	き\n"
		"KI	キ\n"
		"ku	く\n"
		"KU	ク\n"
		"ke	け\n"
		"KE	ケ\n"
		"ko	こ\n"
		"KO	コ\n"
		",a	ゕ\n"
		",A	ヵ\n"
		",e	ゖ\n"
		",E	ヶ\n"
		"ga	が\n"
		"GA	ガ\n"
		"gi	ぎ\n"
		"GI	ギ\n"
		"gu	ぐ\n"
		"GU	グ\n"
		"ge	げ\n"
		"GE	ゲ\n"
		"go	ご\n"
		"GO	ゴ\n"
		"GHA	ジャ\n"
		"GHI	ジィ\n"
		"GHU	ジュ\n"
		"GHE	ジェ\n"
		"GHO	ジョ\n"
		"GWA	グァ\n"
		"GWI	グィ\n"
		"GWU	グゥ\n"
		"GWE	グェ\n"
		"GWO	グォ\n"
		"gya	ぎゃ\n"
		"GYA	ギャ\n"
		"gyi	ぎぃ\n"
		"GYI	ギィ\n"
		"gyu	ぎゅ\n"
		"GYU	ギュ\n"
		"gye	ぎぇ\n"
		"GYE	ギェ\n"
		"gyo	ぎょ\n"
		"GYO	ギョ\n"
		"sa	さ\n"
		"SA	サ\n"
		"si	し\n"
		"SI	シ\n"
		"su	す\n"
		"SU	ス\n"
		"se	せ\n"
		"SE	セ\n"
		"so	そ\n"
		"SO	ソ\n"
		"KWA	クァ\n"
		"KWI	クィ\n"
		"KWU	クゥ\n"
		"KWE	クェ\n"
		"KWO	クォ\n"
		"za	ざ\n"
		"ZA	ザ\n"
		"zi	じ\n"
		"ZI	ジ\n"
		"zu	ず\n"
		"ZU	ズ\n"
		"ze	ぜ\n"
		"ZE	ゼ\n"
		"zo	ぞ\n"
		"ZO	ゾ\n"
		"ta	た\n"
		"TA	タ\n"
		"ti	ち\n"
		"TI	チ\n"
		"tu	つ\n"
		"TU	ツ\n"
		"TSU	ツ\n"
		"te	て\n"
		"TE	テ\n"
		"to	と\n"
		"TO	ト\n"
		"da	だ\n"
		"DA	ダ\n"
		"di	ぢ\n"
		"DI	ヂ\n"
		"du	づ\n"
		"DU	ヅ\n"
		"de	で\n"
		"DE	デ\n"
		"do	ど\n"
		"DO	ド\n"
		"na	な\n"
		"NA	ナ\n"
		"ni	に\n"
		"NI	ニ\n"
		"nu	ぬ\n"
		"NU	ヌ\n"
		"ne	ね\n"
		"NE	ネ\n"
		"no	の\n"
		"NO	ノ\n"
		"pa	ぱ\n"
		"PA	パ\n"
		"pi	ぴ\n"
		"PI	ピ\n"
		"pu	ぷ\n"
		"PU	プ\n"
		"pe	ぺ\n"
		"PE	ペ\n"
		"po	ぽ\n"
		"PO	ポ\n"
		"ma	ま\n"
		"MA	マ\n"
		"mi	み\n"
		"MI	ミ\n"
		"mu	む\n"
		"MU	ム\n"
		"me	め\n"
		"ME	メ\n"
		"mo	も\n"
		"MO	モ\n"
		"xx7	ゃ\n"
		"XX7	ャ\n"
		"xx8	ゅ\n"
		"XX8	ュ\n"
		"xx9	ょ\n"
		"XX9	ョ\n"
		"ya	や\n"
		"YA	ヤ\n"
		"yu	ゆ\n"
		"YU	ユ\n"
		"yo	よ\n"
		"YO	ヨ\n"
		"xxd	ゑ\n"
		"XXD	ヱ\n"
		"xxk	ゐ\n"
		"XXK	ヰ\n"
		"xxl	を\n"
		"XXL	ヲ\n"
		"xxc	ゑ゙\n"
		"xx,	ゐ゙\n"
		"xx.	を゙\n"
		"XXC	ヹ\n"
		"XX,	ヸ\n"
		"XX.	ヺ\n"
		"ra	ら\n"
		"RA	ラ\n"
		"ri	り\n"
		"RI	リ\n"
		"ru	る\n"
		"RU	ル\n"
		"re	れ\n"
		"RE	レ\n"
		"ro	ろ\n"
		"RO	ロ\n"
		"xxw	ゎ\n"
		"XXW	ヮ\n"
		"xxv	ゔ\n"
		"XXV	ヴ\n"
		"wa	わ\n"
		"WA	ワ\n"
		"WI	ウィ\n"
		"WE	ウェ\n"
		"wo	を\n"
		"WHA	ウァ\n"
		"WHI	ウィ\n"
		"WHU	ウゥ\n"
		"WHE	ウェ\n"
		"WHO	ウォ\n"
		"-	ー\n"
	);
	return 0;
}