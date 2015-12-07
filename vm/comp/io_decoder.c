#include "io_decoder.h"


static bool
io_decoder_inb(void *_io_state, uint32_t addr, uint8_t *valp){
	io_decoder_state *io_state = (struct io_decoder_state*) _io_state;
	uint32_t offset = addr-PIC_BEGIN;

	if(offset <= PIC_END){
		/* Address is in PIC range */
		*valp = sig_host_bus_read_io_dev(io_state->port_host, io_state, addr);
		return true;
	}
	
	return false;
}

static bool
io_decoder_outb(void *_io_state, uint32_t addr, uint8_t val){
	io_decoder_state *io_decoder_state = (struct io_decoder_state*) _io_state;
	uint32_t offset = addr-PIC_BEGIN;

	if(offset <= PIC_END){
		/* Address is in PIC range */
		sig_host_bus_write_io_dev(io_decoder_state->port_host, io_decoder_state, addr, val);
		return true;
	}
	
	return false;
}


void *
io_decoder_create(struct sig_host_bus *port_host)
{
	io_decoder_state *io_decoder_state;
	static const struct sig_host_bus_funcs hf = {
		.inb = io_decoder_inb,
		.writeb = io_decoder_outb
	};

	io_decoder_state = malloc(sizeof(struct io_decoder_state));
	assert(io_decoder_state != NULL);

	io_decoder_state->port_host = port_host;

	sig_host_bus_connect(port_host, io_decoder_state, &hf);

	return io_decoder_state;
}

void
io_decoder_destroy(void *_io_decoder_state)
{
	free(_io_decoder_state);
}
