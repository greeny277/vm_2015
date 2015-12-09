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

struct _pic_state;

typedef struct pic_connection {
	struct _pic_state *pic_instance;
	uint8_t int_no;
} pic_connection;

typedef struct _pic_state {
	/** ports */
	struct sig_host_bus *port_host;
	struct cpu_state *cpu;

	uint8_t interrupt_mask;
	uint8_t interrupt_vector_byte_base;

	uint8_t cur_icw_byte_no;
	uint8_t cur_ocw_byte_no;

	uint8_t irr;

	//bitmask of populated lines for sanity checks
	uint8_t connected_lines;
	pic_connection connections[8];
} pic_state;


/** request connection to the PIC
  *
  * @param pic_state the PIC instance
  * @param int_no the line to request connection to
  * @param conn the pointer to write the pointer to the connection object to
  * @return true on success, false if the line was already assigned or int_no is too high
  */
bool
pic_connect(pic_state *pic_instance, uint8_t int_no, pic_connection **conn);

/** raise an interrupt
 *
 * @param conn a pointer to a connection object retrieved with pic_connect
 * @return true if the interrupt was raised, false if it is masked and on error
 */
bool pic_interrupt(pic_connection *conn);

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
