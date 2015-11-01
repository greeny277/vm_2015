/* $Id: disk_ctrl.h 9992 2014-10-15 11:42:43Z raimue $ */

#ifndef __DISK_CTRL_H_INCLUDED
#define __DISK_CTRL_H_INCLUDED

#include "sig_host_bus.h"

/** create a disk controller 
  * @param port_host port to host bus
  * @param df filename of disk
  * @return disk controller instance
  */
extern void *
disk_ctrl_create(struct sig_host_bus *port_host, const char *df);

/** destroy a disk controller instance 
  * @param s disk controller instance
  */
extern void
disk_ctrl_destroy(void *s);

#endif /* __DISK_CTRL_H_INCLUDED */
