#include "io_decoder.h"


static bool
io_decoder_inb(void *s, uint32_t addr, uint8_t *valp){return true;};

static bool
io_decoder_outb(void *s, uint32_t addr, uint8_t valp){return true;};

static void
io_decoder_set(void *s, bool val){};


void *
iodecoder_create(struct sig_host_bus *port_host, struct sig_boolean *sig_boolean)
{
	iodecoder_state *iodecoder_state;
	static const struct sig_host_bus_funcs hf = {
		.inb = io_decoder_inb,
		.writeb = io_decoder_outb
	};

	static const struct sig_boolean_funcs bf = {
		.set = io_decoder_set
	};

	iodecoder_state = malloc(sizeof(struct iodecoder_state));
	assert(iodecoder_state != NULL);

	iodecoder_state->port_host = port_host;
	iodecoder_state->boolean_host = sig_boolean;

	sig_host_bus_connect(port_host, iodecoder_state, &hf);
	sig_boolean_connect(sig_boolean, iodecoder_state, &bf);

	return iodecoder_state;
}

void
iodecoder_destroy(void *_iodecoder_state)
{
	free(_iodecoder_state);
}