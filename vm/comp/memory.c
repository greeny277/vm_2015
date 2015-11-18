#include "memory.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>


static bool
ram_readb(void *_ram_state, uint32_t addr, uint8_t *valp){
	ram_state* ram_state = (struct ram_state*) _ram_state;
	uint32_t offset = addr - RAM_MEM_BASE;

	if(offset >= RAM_SIZE){
		return false;
	}

	*valp = ram_state->ram[offset];

	return true;
}

static bool
ram_writeb(void *_ram_state, uint32_t addr, uint8_t val){
	ram_state* ram_state = (struct ram_state*) _ram_state;
	uint32_t offset = addr - RAM_MEM_BASE;

	if(offset >= RAM_SIZE){
		return false;
	}

	ram_state->ram[offset] = val;
	return true;
}

void *
memory_create(struct sig_host_bus *port_host)
{
	ram_state *ram_state;
	static const struct sig_host_bus_funcs hf = {
		.readb = ram_readb,
		.writeb = ram_writeb
	};

	ram_state = malloc(sizeof(struct ram_state));
	assert(ram_state != NULL);

	memset(ram_state->ram, 0, RAM_SIZE);
	ram_state->port_host = port_host;

	sig_host_bus_connect(port_host, ram_state, &hf);

	return ram_state;
}

void
memory_destroy(void *_ram_state)
{
	free(_ram_state);
}
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
