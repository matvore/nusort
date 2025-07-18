#include "residual_stroke_count.h"
#include "util.h"

static const uint8_t counts[] = {
	0, /* 一 */
	1, /* 丁 */
	2, /* 万 */
	3, /* 中 */
	4, /* 世 */
	5, /* 両 */
	6, /* 乱 */
	7, /* 事 */
	8, /* 乗 */
	9, /* 乘 */
	10, /* 乾 */
	12, /* 亂 */
	0, /* 二 */
	1, /* 于 */
	2, /* 云 */
	4, /* 亘 */
	5, /* 亜 */
	6, /* 些 */
	0, /* 亠 */
	1, /* 亡 */
	2, /* 亢 */
	4, /* 交 */
	5, /* 亨 */
	6, /* 享 */
	7, /* 亭 */
	8, /* 亳 */
	11, /* 亶 */
	0, /* 人 */
	2, /* 今 */
	3, /* 以 */
	4, /* 会 */
	5, /* 作 */
	6, /* 使 */
	7, /* 信 */
	8, /* 個 */
	9, /* 健 */
	10, /* 備 */
	11, /* 働 */
	12, /* 像 */
	13, /* 億 */
	14, /* 儒 */
	15, /* 優 */
	16, /* 儲 */
	0, /* 儿 */
	2, /* 元 */
	3, /* 兄 */
	4, /* 先 */
	5, /* 児 */
	6, /* 免 */
	8, /* 党 */
	9, /* 兜 */
	0, /* 入 */
	2, /* 內 */
	4, /* 全 */
	6, /* 兩 */
	0, /* 八 */
	2, /* 公 */
	4, /* 共 */
	5, /* 兵 */
	6, /* 具 */
	8, /* 兼 */
	0, /* 冂 */
	2, /* 内 */
	3, /* 囘 */
	4, /* 再 */
	7, /* 冑 */
	0, /* 冖 */
	2, /* 冗 */
	3, /* 写 */
	7, /* 冠 */
	8, /* 冤 */
	9, /* 冨 */
	0, /* 冫 */
	3, /* 冬 */
	4, /* 冲 */
	5, /* 冴 */
	6, /* 冽 */
	8, /* 凄 */
	13, /* 凛 */
	14, /* 凝 */
	0, /* 几 */
	1, /* 凡 */
	3, /* 処 */
	4, /* 凪 */
	9, /* 凰 */
	10, /* 凱 */
	0, /* 凵 */
	2, /* 凶 */
	3, /* 出 */
	6, /* 函 */
	0, /* 刀 */
	1, /* 刃 */
	2, /* 分 */
	3, /* 刊 */
	4, /* 刑 */
	5, /* 初 */
	6, /* 到 */
	7, /* 前 */
	8, /* 剤 */
	9, /* 剪 */
	10, /* 割 */
	12, /* 劃 */
	13, /* 劇 */
	14, /* 劔 */
	0, /* 力 */
	3, /* 功 */
	4, /* 劣 */
	5, /* 助 */
	6, /* 効 */
	7, /* 勁 */
	8, /* 勉 */
	9, /* 勒 */
	10, /* 勝 */
	11, /* 勢 */
	13, /* 勲 */
	0, /* 勹 */
	1, /* 勺 */
	2, /* 勾 */
	3, /* 包 */
	4, /* 匈 */
	7, /* 匍 */
	9, /* 匐 */
	0, /* 匕 */
	2, /* 化 */
	3, /* 北 */
	9, /* 匙 */
	0, /* 匸 */
	2, /* 区 */
	3, /* 匝 */
	4, /* 匠 */
	5, /* 医 */
	8, /* 匿 */
	9, /* 區 */
	11, /* 匯 */
	12, /* 匱 */
	13, /* 匳 */
	0, /* 十 */
	1, /* 千 */
	2, /* 卅 */
	3, /* 卉 */
	4, /* 卍 */
	6, /* 卒 */
	7, /* 南 */
	10, /* 博 */
	0, /* 卜 */
	3, /* 占 */
	6, /* 卦 */
	0, /* 卩 */
	3, /* 卯 */
	4, /* 印 */
	5, /* 即 */
	6, /* 卷 */
	7, /* 卸 */
	8, /* 卿 */
	0, /* 厂 */
	2, /* 厄 */
	7, /* 厘 */
	8, /* 原 */
	10, /* 厩 */
	12, /* 厭 */
	0, /* 厶 */
	3, /* 去 */
	6, /* 参 */
	9, /* 參 */
	0, /* 又 */
	1, /* 叉 */
	2, /* 及 */
	6, /* 叔 */
	7, /* 叙 */
	14, /* 叡 */
	16, /* 叢 */
	0, /* ⺍ */
	5, /* 鼡 */
	6, /* 単 */
	8, /* 巣 */
	9, /* 営 */
	14, /* 厳 */
	0, /* 口 */
	2, /* 可 */
	3, /* 合 */
	4, /* 告 */
	5, /* 味 */
	6, /* 品 */
	7, /* 員 */
	8, /* 商 */
	9, /* 善 */
	10, /* 嗄 */
	11, /* 嘉 */
	12, /* 器 */
	13, /* 嘯 */
	14, /* 嚇 */
	15, /* 嚢 */
	17, /* 嚴 */
	18, /* 嚼 */
	19, /* 囎 */
	21, /* 囓 */
	0, /* 囗 */
	2, /* 囚 */
	3, /* 回 */
	4, /* 図 */
	5, /* 固 */
	6, /* 圀 */
	7, /* 圃 */
	8, /* 國 */
	9, /* 圏 */
	10, /* 園 */
	11, /* 圖 */
	0, /* 土 */
	2, /* 圧 */
	3, /* 在 */
	4, /* 坂 */
	5, /* 坡 */
	6, /* 型 */
	7, /* 埃 */
	8, /* 埜 */
	9, /* 報 */
	10, /* 塩 */
	11, /* 塵 */
	12, /* 墜 */
	13, /* 墾 */
	14, /* 壕 */
	16, /* 壜 */
	0, /* 士 */
	1, /* 壬 */
	3, /* 壮 */
	4, /* 声 */
	8, /* 壷 */
	9, /* 壹 */
	11, /* 壽 */
	0, /* 夂 */
	6, /* 変 */
	7, /* 夏 */
	0, /* 夕 */
	2, /* 外 */
	3, /* 多 */
	5, /* 夜 */
	8, /* 夠 */
	10, /* 夢 */
	11, /* 夥 */
	0, /* 大 */
	1, /* 天 */
	2, /* 央 */
	3, /* 夷 */
	4, /* 夾 */
	5, /* 奄 */
	6, /* 契 */
	7, /* 套 */
	9, /* 奢 */
	10, /* 奧 */
	11, /* 奪 */
	13, /* 奮 */
	0, /* 女 */
	2, /* 奴 */
	3, /* 好 */
	4, /* 妊 */
	5, /* 始 */
	6, /* 姜 */
	7, /* 娑 */
	8, /* 娶 */
	9, /* 婿 */
	10, /* 媽 */
	11, /* 嫡 */
	12, /* 嬉 */
	13, /* 嬢 */
	14, /* 嬬 */
	0, /* 子 */
	1, /* 孔 */
	2, /* 孕 */
	3, /* 字 */
	4, /* 孚 */
	5, /* 孟 */
	6, /* 孤 */
	7, /* 孫 */
	8, /* 孰 */
	9, /* 孱 */
	11, /* 孵 */
	13, /* 學 */
	14, /* 孺 */
	16, /* 孽 */
	19, /* 孿 */
	0, /* 宀 */
	2, /* 它 */
	3, /* 宅 */
	4, /* 宋 */
	5, /* 宕 */
	6, /* 客 */
	7, /* 宮 */
	8, /* 宿 */
	9, /* 富 */
	10, /* 寛 */
	11, /* 察 */
	12, /* 審 */
	16, /* 寵 */
	17, /* 寶 */
	0, /* 寸 */
	3, /* 寺 */
	4, /* 対 */
	6, /* 封 */
	7, /* 将 */
	8, /* 將 */
	9, /* 尊 */
	11, /* 對 */
	12, /* 導 */
	0, /* 小 */
	1, /* 少 */
	3, /* 当 */
	5, /* 尚 */
	0, /* 尢 */
	1, /* 尤 */
	5, /* 尭 */
	9, /* 就 */
	0, /* 尸 */
	1, /* 尺 */
	2, /* 尻 */
	3, /* 尽 */
	4, /* 尾 */
	5, /* 居 */
	6, /* 屋 */
	7, /* 展 */
	9, /* 属 */
	11, /* 層 */
	12, /* 履 */
	18, /* 屬 */
	0, /* 屮 */
	1, /* 屯 */
	0, /* 山 */
	3, /* 屹 */
	4, /* 岐 */
	5, /* 岡 */
	6, /* 峙 */
	7, /* 峨 */
	8, /* 崇 */
	9, /* 嵌 */
	10, /* 嵩 */
	11, /* 嶋 */
	13, /* 嶼 */
	14, /* 嶺 */
	17, /* 巌 */
	20, /* 巖 */
	0, /* 川 */
	3, /* 州 */
	0, /* 工 */
	2, /* 左 */
	4, /* 巫 */
	7, /* 差 */
	0, /* 己 */
	1, /* 巴 */
	6, /* 巻 */
	9, /* 巽 */
	0, /* 巾 */
	2, /* 市 */
	3, /* 帆 */
	4, /* 希 */
	5, /* 帖 */
	6, /* 帝 */
	7, /* 師 */
	8, /* 帳 */
	9, /* 帽 */
	10, /* 幌 */
	12, /* 幟 */
	14, /* 幫 */
	0, /* 干 */
	2, /* 平 */
	3, /* 年 */
	5, /* 幸 */
	10, /* 幹 */
	0, /* 幺 */
	1, /* 幻 */
	2, /* 幼 */
	6, /* 幽 */
	9, /* 幾 */
	0, /* 广 */
	2, /* 広 */
	3, /* 庄 */
	4, /* 庇 */
	5, /* 底 */
	6, /* 度 */
	7, /* 座 */
	8, /* 庵 */
	9, /* 廃 */
	10, /* 廉 */
	11, /* 廓 */
	12, /* 廚 */
	16, /* 廬 */
	22, /* 廳 */
	0, /* 廴 */
	4, /* 廷 */
	5, /* 延 */
	6, /* 建 */
	0, /* 廾 */
	1, /* 廿 */
	2, /* 弁 */
	4, /* 弄 */
	12, /* 弊 */
	0, /* 弋 */
	1, /* 弌 */
	3, /* 式 */
	0, /* 弓 */
	1, /* 弔 */
	2, /* 弗 */
	3, /* 弛 */
	4, /* 弟 */
	5, /* 弥 */
	6, /* 弧 */
	7, /* 弱 */
	8, /* 張 */
	9, /* 弾 */
	11, /* 彅 */
	12, /* 彈 */
	13, /* 彊 */
	14, /* 彌 */
	19, /* 彎 */
	0, /* 彐 */
	8, /* 彗 */
	10, /* 彙 */
	0, /* 彡 */
	4, /* 形 */
	6, /* 彦 */
	8, /* 彩 */
	9, /* 彭 */
	11, /* 彰 */
	12, /* 影 */
	0, /* 彳 */
	4, /* 彷 */
	5, /* 彼 */
	6, /* 待 */
	7, /* 徒 */
	8, /* 得 */
	9, /* 御 */
	10, /* 微 */
	11, /* 徳 */
	12, /* 徹 */
	13, /* 徽 */
	0, /* 心 */
	1, /* 必 */
	3, /* 忌 */
	4, /* 忠 */
	5, /* 怒 */
	6, /* 恋 */
	7, /* 悪 */
	8, /* 悲 */
	9, /* 想 */
	10, /* 態 */
	11, /* 慮 */
	12, /* 憊 */
	13, /* 憶 */
	14, /* 懲 */
	16, /* 懸 */
	17, /* 懺 */
	18, /* 懿 */
	19, /* 戀 */
	0, /* 戈 */
	1, /* 戊 */
	2, /* 戌 */
	3, /* 我 */
	4, /* 或 */
	7, /* 戚 */
	8, /* 戟 */
	9, /* 戦 */
	10, /* 截 */
	11, /* 戮 */
	12, /* 戰 */
	13, /* 戴 */
	0, /* 戸 */
	3, /* 戻 */
	4, /* 所 */
	5, /* 扁 */
	6, /* 扇 */
	7, /* 扈 */
	8, /* 扉 */
	0, /* 手 */
	1, /* 扎 */
	2, /* 打 */
	3, /* 扱 */
	4, /* 扮 */
	5, /* 担 */
	6, /* 持 */
	7, /* 振 */
	8, /* 授 */
	9, /* 提 */
	10, /* 損 */
	11, /* 摘 */
	12, /* 撒 */
	13, /* 撻 */
	14, /* 擢 */
	15, /* 擲 */
	17, /* 攘 */
	18, /* 攜 */
	19, /* 攣 */
	20, /* 攪 */
	0, /* 支 */
	0, /* 攴 */
	2, /* 收 */
	3, /* 改 */
	4, /* 放 */
	5, /* 政 */
	6, /* 效 */
	7, /* 救 */
	8, /* 敢 */
	9, /* 数 */
	10, /* 敲 */
	11, /* 敵 */
	12, /* 整 */
	13, /* 斂 */
	14, /* 斃 */
	0, /* 文 */
	8, /* 斌 */
	0, /* 斗 */
	6, /* 料 */
	7, /* 斜 */
	10, /* 斡 */
	0, /* 斤 */
	1, /* 斥 */
	4, /* 斧 */
	7, /* 斬 */
	8, /* 斯 */
	9, /* 新 */
	14, /* 斷 */
	0, /* 方 */
	4, /* 於 */
	5, /* 施 */
	6, /* 旅 */
	7, /* 族 */
	10, /* 旗 */
	14, /* 旛 */
	0, /* 无 */
	5, /* 既 */
	0, /* 日 */
	1, /* 旧 */
	2, /* 早 */
	3, /* 更 */
	4, /* 昇 */
	5, /* 星 */
	6, /* 時 */
	7, /* 曹 */
	8, /* 普 */
	9, /* 會 */
	10, /* 暢 */
	11, /* 暫 */
	12, /* 曇 */
	13, /* 曖 */
	14, /* 曜 */
	15, /* 曝 */
	0, /* 月 */
	2, /* 有 */
	4, /* 服 */
	6, /* 朔 */
	7, /* 望 */
	8, /* 朝 */
	13, /* 朦 */
	16, /* 朧 */
	0, /* 木 */
	1, /* 未 */
	2, /* 朱 */
	3, /* 杉 */
	4, /* 果 */
	5, /* 枯 */
	6, /* 格 */
	7, /* 械 */
	8, /* 森 */
	9, /* 業 */
	10, /* 概 */
	11, /* 標 */
	12, /* 機 */
	13, /* 橿 */
	14, /* 檸 */
	15, /* 檻 */
	16, /* 欄 */
	17, /* 櫻 */
	19, /* 欒 */
	21, /* 欝 */
	0, /* 欠 */
	2, /* 次 */
	4, /* 欣 */
	7, /* 欲 */
	8, /* 欺 */
	10, /* 歌 */
	11, /* 歎 */
	13, /* 歟 */
	17, /* 歡 */
	0, /* 止 */
	1, /* 正 */
	2, /* 此 */
	3, /* 步 */
	4, /* 武 */
	5, /* 歪 */
	9, /* 歳 */
	10, /* 歴 */
	12, /* 歷 */
	14, /* 歸 */
	0, /* 歹 */
	2, /* 死 */
	5, /* 殆 */
	6, /* 殉 */
	8, /* 殖 */
	14, /* 殯 */
	17, /* 殲 */
	0, /* 殳 */
	4, /* 殴 */
	5, /* 段 */
	6, /* 殷 */
	7, /* 殻 */
	9, /* 殿 */
	11, /* 毅 */
	0, /* 母 */
	2, /* 毎 */
	4, /* 毒 */
	0, /* 比 */
	5, /* 毘 */
	0, /* 毛 */
	4, /* 毟 */
	7, /* 毫 */
	8, /* 毯 */
	0, /* 氏 */
	1, /* 民 */
	0, /* 气 */
	2, /* 気 */
	6, /* 氣 */
	0, /* 水 */
	1, /* 氷 */
	2, /* 氾 */
	3, /* 汎 */
	4, /* 決 */
	5, /* 法 */
	6, /* 活 */
	7, /* 流 */
	8, /* 済 */
	9, /* 減 */
	10, /* 準 */
	11, /* 滲 */
	12, /* 漑 */
	13, /* 激 */
	14, /* 濟 */
	15, /* 濫 */
	16, /* 瀕 */
	17, /* 瀾 */
	19, /* 灘 */
	22, /* 灣 */
	0, /* 火 */
	2, /* 灯 */
	3, /* 灸 */
	4, /* 炉 */
	5, /* 炬 */
	6, /* 烈 */
	7, /* 烹 */
	8, /* 焙 */
	9, /* 煌 */
	10, /* 熊 */
	11, /* 熱 */
	12, /* 熾 */
	13, /* 燥 */
	14, /* 燻 */
	15, /* 爆 */
	17, /* 爛 */
	0, /* 爪 */
	4, /* 采 */
	8, /* 爲 */
	13, /* 爵 */
	0, /* 父 */
	9, /* 爺 */
	0, /* 爻 */
	7, /* 爽 */
	10, /* 爾 */
	0, /* 爿 */
	0, /* 片 */
	4, /* 版 */
	8, /* 牌 */
	9, /* 牒 */
	0, /* 牙 */
	0, /* 牛 */
	2, /* 牝 */
	3, /* 牡 */
	4, /* 牧 */
	5, /* 牲 */
	6, /* 特 */
	7, /* 牽 */
	8, /* 犀 */
	13, /* 犠 */
	0, /* 犬 */
	2, /* 犯 */
	3, /* 状 */
	4, /* 狂 */
	5, /* 狗 */
	6, /* 独 */
	7, /* 狼 */
	8, /* 猊 */
	9, /* 猥 */
	10, /* 猾 */
	11, /* 獄 */
	12, /* 獣 */
	13, /* 獨 */
	14, /* 獰 */
	16, /* 獺 */
	0, /* 玄 */
	6, /* 率 */
	0, /* 玉 */
	3, /* 玖 */
	4, /* 玩 */
	5, /* 玲 */
	6, /* 珠 */
	7, /* 理 */
	8, /* 琥 */
	9, /* 瑕 */
	10, /* 瑠 */
	11, /* 璃 */
	13, /* 環 */
	14, /* 璽 */
	0, /* 瓜 */
	11, /* 瓢 */
	0, /* 瓦 */
	6, /* 瓶 */
	11, /* 甑 */
	13, /* 甕 */
	0, /* 甘 */
	4, /* 甚 */
	6, /* 甜 */
	0, /* 生 */
	6, /* 産 */
	7, /* 甥 */
	0, /* 用 */
	2, /* 甫 */
	0, /* 田 */
	2, /* 男 */
	3, /* 画 */
	4, /* 界 */
	5, /* 畔 */
	6, /* 畢 */
	7, /* 番 */
	8, /* 當 */
	10, /* 畿 */
	14, /* 疆 */
	0, /* 疋 */
	7, /* 疎 */
	9, /* 疑 */
	0, /* 疒 */
	4, /* 疥 */
	5, /* 疱 */
	6, /* 疵 */
	7, /* 痘 */
	8, /* 痰 */
	9, /* 瘍 */
	10, /* 瘡 */
	11, /* 瘻 */
	12, /* 療 */
	13, /* 癒 */
	16, /* 癪 */
	17, /* 癬 */
	19, /* 癲 */
	0, /* 癶 */
	4, /* 癸 */
	7, /* 登 */
	0, /* 白 */
	1, /* 百 */
	3, /* 的 */
	4, /* 皆 */
	5, /* 畠 */
	6, /* 皐 */
	7, /* 皓 */
	0, /* 皮 */
	10, /* 皺 */
	0, /* 皿 */
	3, /* 盂 */
	4, /* 盃 */
	5, /* 益 */
	6, /* 盒 */
	8, /* 盞 */
	9, /* 盡 */
	10, /* 監 */
	11, /* 盧 */
	12, /* 盪 */
	0, /* 目 */
	3, /* 盲 */
	4, /* 相 */
	5, /* 真 */
	6, /* 眷 */
	8, /* 睛 */
	9, /* 睾 */
	10, /* 瞑 */
	11, /* 瞞 */
	12, /* 瞥 */
	13, /* 瞬 */
	0, /* 矛 */
	4, /* 矜 */
	0, /* 矢 */
	3, /* 知 */
	4, /* 矧 */
	5, /* 矩 */
	7, /* 短 */
	8, /* 矮 */
	12, /* 矯 */
	0, /* 石 */
	4, /* 砂 */
	5, /* 砠 */
	6, /* 砦 */
	7, /* 硝 */
	8, /* 碁 */
	9, /* 碑 */
	10, /* 確 */
	11, /* 磔 */
	12, /* 磯 */
	13, /* 礎 */
	14, /* 礙 */
	15, /* 礫 */
	0, /* 示 */
	1, /* 礼 */
	3, /* 社 */
	4, /* 祇 */
	5, /* 祝 */
	6, /* 票 */
	7, /* 祷 */
	8, /* 禁 */
	9, /* 禅 */
	12, /* 禧 */
	13, /* 禮 */
	14, /* 禰 */
	0, /* 禸 */
	4, /* 禹 */
	8, /* 禽 */
	0, /* 禾 */
	2, /* 私 */
	3, /* 秊 */
	4, /* 秋 */
	5, /* 秘 */
	6, /* 移 */
	7, /* 稀 */
	8, /* 稔 */
	9, /* 種 */
	10, /* 稼 */
	11, /* 穆 */
	13, /* 穢 */
	14, /* 穩 */
	0, /* 穴 */
	2, /* 究 */
	3, /* 空 */
	4, /* 穿 */
	5, /* 窄 */
	6, /* 窒 */
	8, /* 窟 */
	9, /* 窩 */
	10, /* 窮 */
	11, /* 窺 */
	12, /* 竃 */
	13, /* 竄 */
	16, /* 竈 */
	0, /* 立 */
	5, /* 站 */
	6, /* 章 */
	7, /* 竢 */
	9, /* 竪 */
	15, /* 競 */
	0, /* 竹 */
	2, /* 竺 */
	3, /* 竿 */
	4, /* 笑 */
	5, /* 第 */
	6, /* 筆 */
	7, /* 節 */
	8, /* 箆 */
	9, /* 箪 */
	10, /* 築 */
	11, /* 簀 */
	12, /* 簡 */
	13, /* 簾 */
	14, /* 簪 */
	15, /* 籃 */
	16, /* 籟 */
	17, /* 籤 */
	0, /* 米 */
	3, /* 籾 */
	4, /* 粉 */
	5, /* 粒 */
	6, /* 粟 */
	7, /* 糀 */
	8, /* 粽 */
	9, /* 糊 */
	10, /* 糖 */
	11, /* 糞 */
	12, /* 糧 */
	0, /* 糸 */
	1, /* 糺 */
	3, /* 約 */
	4, /* 納 */
	5, /* 経 */
	6, /* 結 */
	7, /* 続 */
	8, /* 綜 */
	9, /* 緘 */
	10, /* 繁 */
	11, /* 績 */
	12, /* 織 */
	13, /* 繰 */
	14, /* 繻 */
	15, /* 纈 */
	0, /* 缶 */
	4, /* 缺 */
	0, /* 网 */
	5, /* 罠 */
	8, /* 罪 */
	9, /* 罰 */
	10, /* 罵 */
	11, /* 罹 */
	14, /* 羅 */
	0, /* 羊 */
	3, /* 美 */
	5, /* 羞 */
	6, /* 着 */
	7, /* 群 */
	13, /* 羹 */
	0, /* 羽 */
	4, /* 翁 */
	5, /* 翌 */
	6, /* 翔 */
	8, /* 翠 */
	9, /* 翫 */
	10, /* 翰 */
	11, /* 翳 */
	12, /* 翻 */
	14, /* 耀 */
	0, /* 老 */
	2, /* 考 */
	4, /* 者 */
	0, /* 而 */
	3, /* 耐 */
	0, /* 耒 */
	4, /* 耕 */
	0, /* 耳 */
	3, /* 耶 */
	4, /* 耽 */
	5, /* 聊 */
	7, /* 聖 */
	8, /* 聚 */
	11, /* 聯 */
	12, /* 職 */
	16, /* 聽 */
	0, /* 聿 */
	7, /* 肆 */
	8, /* 肇 */
	0, /* 肉 */
	2, /* 肋 */
	3, /* 肖 */
	4, /* 股 */
	5, /* 背 */
	6, /* 胱 */
	7, /* 脚 */
	8, /* 脹 */
	9, /* 腹 */
	10, /* 膜 */
	11, /* 膚 */
	12, /* 膨 */
	13, /* 膿 */
	14, /* 臍 */
	15, /* 臓 */
	18, /* 臟 */
	0, /* 臣 */
	2, /* 臥 */
	11, /* 臨 */
	0, /* 自 */
	3, /* 臭 */
	0, /* 至 */
	4, /* 致 */
	8, /* 臺 */
	0, /* 臼 */
	6, /* 與 */
	7, /* 舅 */
	9, /* 興 */
	11, /* 舊 */
	0, /* 舌 */
	4, /* 舐 */
	6, /* 舒 */
	9, /* 舗 */
	10, /* 舘 */
	0, /* 舛 */
	6, /* 舜 */
	8, /* 舞 */
	0, /* 舟 */
	4, /* 舩 */
	5, /* 船 */
	7, /* 艇 */
	9, /* 艘 */
	13, /* 艤 */
	15, /* 艦 */
	0, /* 艮 */
	1, /* 良 */
	11, /* 艱 */
	0, /* 色 */
	13, /* 艶 */
	0, /* 艸 */
	3, /* 芋 */
	4, /* 花 */
	5, /* 英 */
	6, /* 茗 */
	7, /* 荷 */
	8, /* 著 */
	9, /* 落 */
	10, /* 蒸 */
	11, /* 蓼 */
	12, /* 蔵 */
	13, /* 薄 */
	14, /* 薩 */
	15, /* 藤 */
	16, /* 蘭 */
	0, /* 虍 */
	2, /* 虎 */
	3, /* 虐 */
	4, /* 虔 */
	5, /* 處 */
	6, /* 虛 */
	7, /* 虜 */
	8, /* 虡 */
	0, /* 虫 */
	3, /* 虹 */
	4, /* 蚊 */
	5, /* 蛆 */
	6, /* 蛙 */
	7, /* 蛸 */
	8, /* 蜜 */
	9, /* 蝕 */
	10, /* 螂 */
	11, /* 螺 */
	12, /* 蟠 */
	13, /* 蟷 */
	14, /* 蠕 */
	15, /* 蠢 */
	16, /* 蠧 */
	18, /* 蠶 */
	19, /* 蠻 */
	0, /* 血 */
	6, /* 衆 */
	0, /* 行 */
	3, /* 衍 */
	5, /* 衒 */
	6, /* 街 */
	7, /* 衙 */
	9, /* 衝 */
	10, /* 衛 */
	0, /* 衣 */
	2, /* 表 */
	4, /* 衰 */
	5, /* 袈 */
	6, /* 袱 */
	7, /* 裏 */
	8, /* 裨 */
	9, /* 複 */
	10, /* 褥 */
	11, /* 褻 */
	13, /* 襖 */
	14, /* 襦 */
	16, /* 襲 */
	17, /* 襴 */
	0, /* 西 */
	3, /* 要 */
	12, /* 覆 */
	13, /* 覇 */
	0, /* 見 */
	4, /* 規 */
	5, /* 覗 */
	8, /* 覢 */
	9, /* 親 */
	10, /* 覧 */
	11, /* 観 */
	13, /* 覺 */
	17, /* 觀 */
	0, /* 角 */
	6, /* 觜 */
	13, /* 觸 */
	16, /* 觾 */
	0, /* 言 */
	2, /* 訂 */
	3, /* 訊 */
	4, /* 訛 */
	5, /* 訴 */
	6, /* 詢 */
	7, /* 認 */
	8, /* 調 */
	9, /* 諦 */
	10, /* 講 */
	11, /* 謬 */
	12, /* 證 */
	13, /* 譬 */
	14, /* 譴 */
	15, /* 讀 */
	16, /* 變 */
	17, /* 讓 */
	0, /* 谷 */
	0, /* 豆 */
	6, /* 豊 */
	21, /* 豔 */
	0, /* 豕 */
	4, /* 豚 */
	5, /* 象 */
	7, /* 豪 */
	9, /* 豫 */
	0, /* 豸 */
	3, /* 豹 */
	5, /* 貂 */
	7, /* 貌 */
	0, /* 貝 */
	2, /* 負 */
	3, /* 財 */
	4, /* 販 */
	5, /* 貯 */
	6, /* 賂 */
	7, /* 賑 */
	8, /* 質 */
	9, /* 賢 */
	10, /* 購 */
	11, /* 贄 */
	12, /* 贋 */
	14, /* 贔 */
	15, /* 贖 */
	0, /* 赤 */
	4, /* 赦 */
	7, /* 赫 */
	0, /* 走 */
	2, /* 赳 */
	3, /* 起 */
	5, /* 超 */
	7, /* 趙 */
	8, /* 趣 */
	10, /* 趨 */
	0, /* 足 */
	4, /* 趾 */
	5, /* 跋 */
	6, /* 践 */
	7, /* 踊 */
	8, /* 踏 */
	9, /* 踵 */
	10, /* 蹊 */
	11, /* 蹙 */
	12, /* 蹲 */
	13, /* 躁 */
	14, /* 躊 */
	15, /* 躑 */
	16, /* 躙 */
	0, /* 身 */
	3, /* 躬 */
	4, /* 躯 */
	5, /* 躰 */
	8, /* 躺 */
	9, /* 躾 */
	0, /* 車 */
	1, /* 軋 */
	2, /* 軌 */
	3, /* 軒 */
	4, /* 軟 */
	5, /* 軸 */
	6, /* 較 */
	7, /* 輔 */
	8, /* 輛 */
	9, /* 輯 */
	10, /* 輿 */
	11, /* 轉 */
	12, /* 轍 */
	14, /* 轟 */
	15, /* 轡 */
	16, /* 轣 */
	0, /* 辛 */
	6, /* 辞 */
	7, /* 辣 */
	9, /* 辨 */
	0, /* 辰 */
	3, /* 辱 */
	6, /* 農 */
	0, /* 辵 */
	2, /* 辺 */
	3, /* 巡 */
	4, /* 迎 */
	5, /* 述 */
	6, /* 迷 */
	7, /* 通 */
	8, /* 週 */
	9, /* 道 */
	10, /* 違 */
	11, /* 適 */
	12, /* 遲 */
	13, /* 避 */
	14, /* 邇 */
	15, /* 邊 */
	19, /* 邏 */
	0, /* 邑 */
	4, /* 那 */
	5, /* 邪 */
	6, /* 郎 */
	7, /* 郡 */
	8, /* 部 */
	11, /* 鄙 */
	12, /* 鄧 */
	0, /* 酉 */
	2, /* 酊 */
	3, /* 酌 */
	4, /* 酔 */
	5, /* 酢 */
	6, /* 酩 */
	7, /* 酵 */
	8, /* 醂 */
	9, /* 醍 */
	10, /* 醜 */
	11, /* 醫 */
	13, /* 醸 */
	0, /* 釆 */
	4, /* 釈 */
	5, /* 釉 */
	13, /* 釋 */
	0, /* 里 */
	2, /* 重 */
	4, /* 野 */
	5, /* 量 */
	0, /* 金 */
	2, /* 釘 */
	3, /* 釣 */
	4, /* 鈍 */
	5, /* 鈴 */
	6, /* 鉾 */
	7, /* 鋏 */
	8, /* 鋸 */
	9, /* 鍋 */
	10, /* 鎌 */
	11, /* 鏃 */
	12, /* 鐓 */
	13, /* 鐵 */
	15, /* 鑑 */
	19, /* 鑼 */
	20, /* 鑿 */
	0, /* 長 */
	0, /* 門 */
	2, /* 閃 */
	3, /* 閉 */
	4, /* 開 */
	5, /* 閘 */
	6, /* 関 */
	7, /* 閲 */
	8, /* 閻 */
	9, /* 闇 */
	10, /* 闘 */
	11, /* 關 */
	12, /* 闡 */
	13, /* 闢 */
	0, /* 阜 */
	3, /* 阡 */
	4, /* 阪 */
	5, /* 阻 */
	6, /* 限 */
	7, /* 降 */
	8, /* 陪 */
	9, /* 陽 */
	10, /* 隔 */
	11, /* 際 */
	13, /* 隣 */
	14, /* 隱 */
	0, /* 隶 */
	8, /* 隷 */
	0, /* 隹 */
	2, /* 隻 */
	3, /* 雀 */
	4, /* 雁 */
	5, /* 雉 */
	6, /* 雌 */
	8, /* 雕 */
	9, /* 雖 */
	10, /* 雙 */
	11, /* 離 */
	0, /* 雨 */
	3, /* 雪 */
	4, /* 雰 */
	5, /* 電 */
	6, /* 需 */
	7, /* 震 */
	8, /* 霍 */
	9, /* 霜 */
	11, /* 霧 */
	12, /* 霰 */
	13, /* 露 */
	14, /* 霾 */
	16, /* 靂 */
	0, /* 青 */
	5, /* 靖 */
	6, /* 静 */
	8, /* 靜 */
	0, /* 非 */
	7, /* 靠 */
	11, /* 靡 */
	0, /* 面 */
	0, /* 革 */
	3, /* 靭 */
	4, /* 靴 */
	5, /* 靼 */
	6, /* 鞋 */
	7, /* 鞘 */
	8, /* 鞠 */
	9, /* 鞭 */
	13, /* 韃 */
	0, /* 韋 */
	8, /* 韓 */
	10, /* 韜 */
	0, /* 韭 */
	3, /* 韮 */
	0, /* 音 */
	5, /* 韶 */
	10, /* 韻 */
	11, /* 響 */
	0, /* 頁 */
	2, /* 頂 */
	3, /* 項 */
	4, /* 頑 */
	5, /* 頗 */
	6, /* 頬 */
	7, /* 頭 */
	8, /* 顆 */
	9, /* 題 */
	10, /* 願 */
	12, /* 顧 */
	14, /* 顯 */
	15, /* 顰 */
	0, /* 風 */
	3, /* 颪 */
	5, /* 颯 */
	11, /* 飄 */
	0, /* 飛 */
	0, /* 食 */
	2, /* 飢 */
	4, /* 飩 */
	5, /* 飼 */
	6, /* 養 */
	7, /* 餐 */
	8, /* 館 */
	10, /* 饂 */
	11, /* 饅 */
	12, /* 饒 */
	13, /* 饗 */
	0, /* 首 */
	0, /* 香 */
	9, /* 馥 */
	11, /* 馨 */
	0, /* 馬 */
	2, /* 馭 */
	3, /* 馳 */
	4, /* 駁 */
	5, /* 駈 */
	6, /* 駱 */
	7, /* 駿 */
	8, /* 騎 */
	9, /* 騙 */
	10, /* 騰 */
	11, /* 驅 */
	12, /* 驕 */
	13, /* 驛 */
	14, /* 驟 */
	16, /* 驢 */
	19, /* 驪 */
	0, /* 骨 */
	6, /* 骸 */
	9, /* 髄 */
	11, /* 髏 */
	13, /* 髑 */
	0, /* 高 */
	0, /* 髟 */
	4, /* 髣 */
	5, /* 髯 */
	6, /* 髭 */
	8, /* 鬆 */
	11, /* 鬘 */
	12, /* 鬚 */
	14, /* 鬢 */
	0, /* 鬥 */
	6, /* 鬨 */
	0, /* 鬯 */
	19, /* 鬱 */
	0, /* 鬲 */
	0, /* 鬼 */
	4, /* 魁 */
	5, /* 魃 */
	8, /* 魍 */
	11, /* 魑 */
	0, /* 魚 */
	4, /* 魯 */
	5, /* 鮎 */
	6, /* 鮟 */
	7, /* 鯉 */
	8, /* 鯔 */
	9, /* 鰆 */
	10, /* 鰤 */
	11, /* 鰺 */
	12, /* 鰹 */
	13, /* 鱗 */
	16, /* 鱸 */
	0, /* 鳥 */
	2, /* 鳩 */
	3, /* 鳳 */
	4, /* 鴇 */
	5, /* 鴛 */
	6, /* 鴻 */
	7, /* 鵐 */
	8, /* 鵡 */
	9, /* 鶤 */
	10, /* 鶯 */
	12, /* 鷲 */
	13, /* 鷺 */
	17, /* 鸚 */
	19, /* 鸞 */
	0, /* 鹵 */
	8, /* 鹸 */
	0, /* 鹿 */
	8, /* 麒 */
	12, /* 麟 */
	0, /* 麦 */
	4, /* 麩 */
	8, /* 麹 */
	9, /* 麺 */
	0, /* 麻 */
	3, /* 麼 */
	7, /* 麿 */
	0, /* 黄 */
	0, /* 黍 */
	3, /* 黎 */
	5, /* 黏 */
	11, /* 黐 */
	0, /* 黒 */
	3, /* 墨 */
	5, /* 黛 */
	11, /* 黴 */
	0, /* 黹 */
	0, /* 黽 */
	12, /* 鼈 */
	0, /* 鼎 */
	0, /* 鼓 */
	0, /* 鼠 */
	5, /* 鼬 */
	0, /* 鼻 */
	0, /* 斉 */
	3, /* 斎 */
	7, /* 齎 */
	0, /* 齒 */
	2, /* 齔 */
	5, /* 齟 */
	6, /* 齧 */
	7, /* 齬 */
	0, /* 龍 */
	0, /* 龜 */
	0, /* 龠 */
};

int residual_stroke_count(struct kanji_entry const *k)
{
	return residual_stroke_count_from_rsc_sort_key(k->rsc_sort_key);
}

int residual_stroke_count_from_rsc_sort_key(unsigned rsc_sort_key)
{
	unsigned index = rsc_sort_key - 1;
	if (index >= sizeof(counts))
		DIE(0, "%u >= %zu", index, sizeof(counts));
	return counts[index];
}

int largest_residual_stroke_count(void)
{
	int i;
	int result = -1;

	for (i = 0; i < sizeof(counts); i++)
		if (counts[i] > result)
			result = counts[i];

	return result;
}
