extern int use_packetized_out;
void add_packetized_out(const char *s, unsigned len);
void add_packetized_out_null_terminated(const char *s);
void dump_packetized_out(void);
