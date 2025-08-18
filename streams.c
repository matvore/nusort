#include "streams.h"

#include <stdio.h>
#include <string.h>

FILE *out, *err, *in;
int debugout = 0;

const char *termeol(void) {return debugout ? ":eol" : "\x1b[0K";}

void output_readable(const char *s, unsigned len)
{
	if (len == -1) len = strlen(s);

	if (!debugout) { fwrite(s, len, 1, out); return; }

	for (; len; s++, len--) {
		if (*s=='\b')	fputs("<bs>",	out); else
		if (*s=='\x1b')	fputs("<es>",	out); else
				fputc(*s,	out);
	}
}
