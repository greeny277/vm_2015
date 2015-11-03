#include <stdlib.h>
#include <assert.h>

#include "cpu.h"

struct cpssp {
	/** ports */
	struct sig_host_bus *port_host;

	/** state */
	/* Register beginning with letter 'e' have length of 32 bits */
	uint32_t eip;
};

bool
cpu_step(void *_cpssp)
{
	return false;
}

static bool
cpu_readb(void *_cpssp, uint32_t addr, uint8_t *valp){
	/* We dont read from CPU */

	return false;
}

static bool
cpu_writeb(void *_cpssp, uint32_t addr, uint8_t val){
	/* We dont write into CPU */

	return false;
}


void *
cpu_create(struct sig_host_bus *port_host)
{
	struct cpssp *cpssp;
	static const struct sig_host_bus_funcs hf = {
		.readb = cpu_readb,
		.writeb = cpu_writeb
	};

	cpssp = malloc(sizeof(struct cpssp));
	assert(cpssp != NULL);

	cpssp->port_host = port_host;
	/* Set base pointer to start address of ROM.
	 * The address is hardcoded, like in real hardware.
	 */
	cpssp->eip = 0xE000;

	sig_host_bus_connect(port_host, cpssp, &hf);
	return cpssp;
}

void
cpu_destroy(void *_cpssp)
{
	free(_cpssp);
}
