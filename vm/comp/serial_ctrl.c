/* $Id: serial_ctrl.c 9992 2014-10-15 11:42:43Z raimue $
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3 
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */

#include "serial_ctrl.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define SERIAL_MEM_BASE 0xD800
#define SERIAL_MEM_MASK 0x000F

struct cpssp {
	/** ports */
	struct sig_host_bus *port_host;

	/** state */
};

static bool
serial_ctrl_readb(void *_cpssp, uint32_t addr, uint8_t *valp)
{
	if ((addr & ~SERIAL_MEM_MASK) != SERIAL_MEM_BASE) {
		return false;
	}
	
	/* currently hardwired to ground */
	*valp = 0;

	return true;
}

static bool
serial_ctrl_writeb(void *_cpssp, uint32_t addr, uint8_t val)
{
	if ((addr & ~SERIAL_MEM_MASK) != SERIAL_MEM_BASE) {
		return false;
	}


	switch (addr & SERIAL_MEM_MASK) {
	case 0x0: /* write to stdout */
		fprintf(stdout, "%c", val);
		break;

	default: /* not defined */
		break;
	}

	return true;
}

void *
serial_ctrl_create(struct sig_host_bus *port_host)
{
	struct cpssp *cpssp;
	static const struct sig_host_bus_funcs hf = {
		.readb = serial_ctrl_readb,
		.writeb = serial_ctrl_writeb
	};

	cpssp = malloc(sizeof(struct cpssp));
	assert(cpssp != NULL);
	cpssp->port_host = port_host;

	sig_host_bus_connect(port_host, cpssp, &hf);

	return cpssp;
}

void 
serial_ctrl_destroy(void *_cpssp)
{
	free(_cpssp);
}
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
