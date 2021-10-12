#include "packetized_out.h"

#include "streams.h"

#include <string.h>


static char pack_out[255];
static unsigned pack_out_sz;

int use_packetized_out;

void add_packetized_out(const char *s, unsigned len)
{
	if (len == -1) len = strlen(s);

	while (len) {
		unsigned cplen;

		if (pack_out_sz == sizeof(pack_out)) dump_packetized_out();

		if (len + pack_out_sz > sizeof(pack_out))
			cplen = sizeof(pack_out) - pack_out_sz;
		else
			cplen = len;

		memcpy(pack_out + pack_out_sz, s, cplen);
		pack_out_sz += cplen;
		len -= cplen;
		s += cplen;
	}

	if (!use_packetized_out) dump_packetized_out();
}

void add_packetized_out_null_terminated(const char *s)
{
	add_packetized_out(s, strlen(s));
}

void dump_packetized_out(void)
{
	if (!pack_out_sz) return;

	if (use_packetized_out) {
		fputc('\x04', out);
		fputc(pack_out_sz, out);
	}

	fwrite(pack_out, pack_out_sz, 1, out);
	pack_out_sz = 0;
}
