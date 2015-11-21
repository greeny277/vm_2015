/* $Id: disk_ctrl.h 9992 2014-10-15 11:42:43Z raimue $ */

#ifndef __DISK_CTRL_H_INCLUDED
#define __DISK_CTRL_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

#include "sig_host_bus.h"

#define DISK_MEM_BASE   0xD000
#define DISK_MEM_BORDER 0x400
#define DISK_MEM_BUF    0x200

#define DISK_BNR		0x0
#define DISK_BNR_1		0x1
#define DISK_BNR_2		0x2
#define DISK_BNR_3		0x3
#define DISK_ERR_REG	0x7
#define DISK_READ_WRITE 0xb

#define DISK_SIZE		5*1024*1024
#define BUF_SIZE		512

typedef struct disk_state {
	/** ports */
	struct sig_host_bus *port_host;

	/** state */
	uint8_t err_reg;
	/* Big endian representation */
	uint32_t bnr;
	uint8_t read_write;
	FILE *f;

	char buffer[BUF_SIZE];
} disk_state;

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
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
