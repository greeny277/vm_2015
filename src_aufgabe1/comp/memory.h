/* $Id: memory.h 9992 2014-10-15 11:42:43Z raimue $ */

#ifndef __MEMORY_H_INCLUDED
#define __MEMORY_H_INCLUDED

#include "sig_host_bus.h"

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
