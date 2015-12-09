#ifndef __PIC_H_INCLUDED
#define __PIC_H_INCLUDED

#include "sig_host_bus.h"
#include "cpu.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define IC4 0
#define SNGL 1
#define LTIM 3

#define T3 3

#define uPM 0
#define AEOI 1
#define SFNM 4

typedef struct pic_state {
	/** ports */
	struct sig_host_bus *port_host;
	struct cpu_state *cpu;

	uint8_t interrupt_mask;
	uint8_t interrupt_vector_byte_base;

	uint8_t cur_icw_byte_no;
	uint8_t cur_ocw_byte_no;

	uint8_t irr;
} pic_state;

/** create a memory instance
  * @param port_host port to host bus.
  * @return memory instance.
  */
extern void *
pic_create(struct sig_host_bus *port_host, cpu_state *cpu_instance);

/** destroy a memory instance.
  * @param s memory instance.
  */
extern void
pic_destroy(void *s);

#endif /* __PIC_H_INCLUDED */
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
