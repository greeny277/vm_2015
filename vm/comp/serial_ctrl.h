/* $Id: serial_ctrl.h 9992 2014-10-15 11:42:43Z raimue $ */

#ifndef __SERIAL_CTRL_H_INCLUDED
#define __SERIAL_CTRL_H_INCLUDED

#include "sig_host_bus.h"
#include "sig_boolean.h"

/** create a serial controller
  * @param port_host host bus instance.
  * @param port_int interrupt output signal.
  * @return serial controller instance
  */
extern void *
serial_ctrl_create(
	struct sig_host_bus *port_host,
	struct sig_boolean *port_int
);

/** destroy a serial controller instance.
  * @param s serial controller instance.
  */
extern void
serial_ctrl_destroy(void *s);

#endif /* __SERIAL_CTRL_H_INCLUDED */
