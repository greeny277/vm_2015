#ifndef __IODECODER_H_INCLUDED
#define __IODECODER_H_INCLUDED

#include "sig_host_bus.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#define PIC_BEGIN 0x0020
#define PIC_END   0x003f

typedef struct io_decoder_state {
	/** ports */
	struct sig_host_bus *port_host;
} io_decoder_state;

/** create a memory instance
  * @param port_host port to host bus.
  * @return memory instance.
  */
extern void *
io_decoder_create(struct sig_host_bus *port_host);

/** destroy a memory instance.
  * @param s memory instance.
  */
extern void
io_decoder_destroy(void *s);

#endif /* __IODECODER_H_INCLUDED */
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
