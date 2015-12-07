#ifndef __PIC_H_INCLUDED
#define __PIC_H_INCLUDED

#include "sig_host_bus.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>


typedef struct pic_state {
	/** ports */
	struct sig_host_bus *port_host;
	struct sig_boolean *pic_to_cpu_bool;
} pic_state;

/** create a memory instance
  * @param port_host port to host bus.
  * @return memory instance.
  */
extern void *
pic_create(struct sig_host_bus *port_host, struct sig_boolean *pic_to_cpu_bool);

/** destroy a memory instance.
  * @param s memory instance.
  */
extern void
pic_destroy(void *s);

#endif /* __PIC_H_INCLUDED */
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
