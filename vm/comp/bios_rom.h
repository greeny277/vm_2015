/* $Id: bios_rom.h 9992 2014-10-15 11:42:43Z raimue $ */

#ifndef __BIOS_ROM_H_INCLUDED
#define __BIOS_ROM_H_INCLUDED

#include "sig_host_bus.h"

/** create a bios rom instance
  * @param port_host port to host bus
  * @param rf filename of rom file 
  * @return bios_rom instance.
  */
extern void *
bios_rom_create(struct sig_host_bus *port_host, const char *rf);


/** destroy a bios rom instance.
  * @param s bios rom instance.
  */
extern void
bios_rom_destroy(void *s);

#endif /* __BIOS_ROM_H_INCLUDED */
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
