#ifndef __IODECODER_H_INCLUDED
#define __IODECODER_H_INCLUDED

#include "sig_host_bus.h"
#include "sig_boolean.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#define IODECODER_MEM_BASE 0x0020

typedef struct iodecoder_state {
	/** ports */
	struct sig_host_bus *port_host;
	struct sig_boolean *boolean_host;
} iodecoder_state;

/** create a memory instance
  * @param port_host port to host bus.
  * @return memory instance.
  */
extern void *
iodecoder_create(struct sig_host_bus *port_host, struct sig_boolean *sig_boolean);

/** destroy a memory instance.
  * @param s memory instance.
  */
extern void
iodecoder_destroy(void *s);

#endif /* __IODECODER_H_INCLUDED */
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
