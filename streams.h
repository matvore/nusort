#include <stdio.h>

/* テストの為、stdin, stdout, stderr の代わりに使う標準ストリーム */
extern FILE *in, *out, *err;

/* "\x1b]" ... などVT100暗号の一部をテキスト形式で出力する */
extern int debugout;

const char *termeol(void);

void output_readable(const char *s, unsigned len);
