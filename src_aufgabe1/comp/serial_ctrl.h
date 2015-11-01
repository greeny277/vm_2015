/* $Id: serial_ctrl.h 10016 2014-10-15 21:40:13Z raimue $ */

#ifndef __SERIAL_CTRL_H_INCLUDED
#define __SERIAL_CTRL_H_INCLUDED

#include "sig_host_bus.h"

/** create a serial controller
  * @param port_host host bus instance.
  * @return serial controller instance
  */
extern void *
serial_ctrl_create(struct sig_host_bus *port_host);

/** destroy a serial controller instance.
  * @param s serial controller instance.
  */
extern void
serial_ctrl_destroy(void *s);

#endif /* __SERIAL_CTRL_H_INCLUDED */
