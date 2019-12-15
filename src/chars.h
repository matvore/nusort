#ifndef CHARS_H
#define CHARS_H

/*
 * 平仮名範囲の全コードポイントが片仮名範囲の全コードポイントと一対一関係。他に
 * も片仮名っぽいコードポイントもあるけれど（例：ヺ）、片仮名範囲外にある。
 */
#define CODEPOINT_RANGE_HIRAGANA 0
#define CODEPOINT_RANGE_KATAKANA 1

/* 該当する平仮名コードポイントのない片仮名。 */
#define CODEPOINT_RANGE_KATAKANA_SPECIAL 2

#define CODEPOINT_RANGE_OTHER 3

int codepoint_range(char const *c);

#endif
