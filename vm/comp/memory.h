/* $Id: memory.h 9992 2014-10-15 11:42:43Z raimue $ */

#ifndef __MEMORY_H_INCLUDED
#define __MEMORY_H_INCLUDED

#include "sig_host_bus.h"

#define RAM_MEM_BASE 0x0000
#define RAM_SIZE     32*1024

typedef struct ram_state {
	/** ports */
	struct sig_host_bus *port_host;

	/** state */
	char ram[RAM_SIZE];
} ram_state;

/** create a memory instance 
  * @param port_host port to host bus.
  * @return memory instance.
  */
extern void *
memory_create(struct sig_host_bus *port_host);

/** destroy a memory instance.
  * @param s memory instance.
  */
extern void
memory_destroy(void *s);

#endif /* __MEMORY_H_INCLUDED */
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
