#include "memory.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#define RAM_SIZE 32*1024

#define RAM_MEM_BASE 0x0000

struct cpssp {
	/** ports */
	struct sig_host_bus *port_host;

	/** state */
	char ram[RAM_SIZE];
};

static bool
ram_readb(void *_cpssp, uint32_t addr, uint8_t *valp){
	struct cpssp* cpssp = (struct cpssp*) _cpssp;
	uint32_t offset = addr - RAM_MEM_BASE;

	if(offset > (RAM_MEM_BASE + RAM_SIZE)){
		return false;
	}

	*valp = cpssp->ram[addr];

	return true;
}

static bool
ram_writeb(void *_cpssp, uint32_t addr, uint8_t val){
	struct cpssp* cpssp = (struct cpssp*) _cpssp;
	uint32_t offset = addr - RAM_MEM_BASE;

	if(offset > (RAM_MEM_BASE + RAM_SIZE)){
		return false;
	}

	cpssp->ram[addr] = val;
	return true;
}

void *
memory_create(struct sig_host_bus *port_host)
{
	struct cpssp *cpssp;
	static const struct sig_host_bus_funcs hf = {
		.readb = ram_readb,
		.writeb = ram_writeb
	};

	cpssp = malloc(sizeof(struct cpssp));
	assert(cpssp != NULL);

	memset(cpssp->ram, 0, RAM_SIZE);
	cpssp->port_host = port_host;

	sig_host_bus_connect(port_host, cpssp, &hf);

	return cpssp;
}

void
memory_destroy(void *_cpssp)
{
	free(_cpssp);
}
