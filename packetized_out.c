#include "packetized_out.h"

#include "streams.h"

#include <string.h>

static char pack_out[255];
static unsigned pack_out_sz;

int use_packetized_out;

static unsigned full_packets;

static void flush(void)
{
	if (!pack_out_sz) return;

	if (use_packetized_out && !debugout) {
		fputc('\x04', out);
		fputc(pack_out_sz, out);
	}

	output_readable(pack_out, pack_out_sz);

	pack_out_sz = 0;
}

void packout(const char *s, unsigned len)
{
	if (len == -1) len = strlen(s);

	while (len) {
		unsigned cplen;

		if (pack_out_sz == sizeof(pack_out)) {
			flush();
			full_packets+=1;
		}

		if (len + pack_out_sz > sizeof(pack_out))
			cplen = sizeof(pack_out) - pack_out_sz;
		else
			cplen = len;

		memcpy(pack_out + pack_out_sz, s, cplen);
		pack_out_sz += cplen;
		len -= cplen;
		s += cplen;
	}
}

void flush_packet(void)
{
	unsigned sz = pack_out_sz;

	flush();

	if (!use_packetized_out || !debugout)	return;
	if (!full_packets && !sz)		return;

	fprintf(out, "{rpc %u full packets + %u bytes}", full_packets, sz);
	full_packets = 0;
}
