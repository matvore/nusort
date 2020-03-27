#ifndef CHARS_H
#define CHARS_H

#define LAST_BYTE_OF_COMBINING_DAKUTEN '\x99'
#define COMBINING_DAKUTEN "\xe3\x82\x99"

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

int is_complete_utf8(char first_char, int size);

#endif
