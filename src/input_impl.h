#include "mapping.h"
#include "mapping_util.h"

struct input_flags {
	unsigned show_keyboard : 1;
	unsigned show_pending_and_converted : 1;
	unsigned show_cutoff_guide : 1;
	unsigned save_with_osc52 : 1;
};

int input_impl(struct mapping *, struct input_flags const *);
