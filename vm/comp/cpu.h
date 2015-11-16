/* $Id: cpu.h 9992 2014-10-15 11:42:43Z raimue $ */

#ifndef __CPU_H_INCLUDED
#define __CPU_H_INCLUDED

#include "sig_host_bus.h"

#define NO_DISPLACEMENT 0
#define DISPLACEMENT_8  1
#define DISPLACEMENT_32 2
#define REGISTER        3

typedef enum cpu_register {
	EAX,
	ECX,
	EDX,
	EBX,

	ESP,
	EBP,
	ESI,
	EDI,

	AL,
	CL,
	DL,
	BL,

	AH,
	CH,
	DH,
	BH
} cpu_register;


/** evaluate one instruction
  * @param s cpu instance
  * @return false if the cpu suspended, true if another instruction
  *         can be executed.
  */
extern bool
cpu_step(void *s);

/** create a cpu
  * @param port_host host bus instance
  * @return cpu instance.
  */
extern void *
cpu_create(struct sig_host_bus *port_host);

/** destroy a cpu instance
  * @param s cpu instance
  */
extern void
cpu_destroy(void *s);

typedef struct cpu_state {
	/** ports */
	struct sig_host_bus *port_host;

	/** state */
	/** Register beginning with letter 'e' have length of 32 bits */
	uint32_t eip;
	/** registers must be in the same order as in `enum cpu_register`*/
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;

	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;

	/** Segment registers */
	uint16_t cs, ds, es, fs, gs, ss;

	/**
	 * EFLAGS register
	 * Bit Number   Meaning                 Shortcut
	 * 11           Overflow Flag             OF
	 *  7           Sign Flag                 SF
	 *  6           Zero Flag                 ZF
	 *  4           Auxiliary Carry Flag      AF
	 *  2           Parity Flag               PF
	 *  0           Carry Flag                CF
	 */
	uint32_t eflags;
} cpu_state;

/*
 * This struct can be used for decoding SIB and ModRM byte
 */
typedef struct modsib {
	/* Operand 1 Reg: Can be used as SRC or DEST */
	uint32_t *op1;

	/* Operand 2 Reg/Mem: Can be used as SRC or DEST */
	uint32_t *op2;

	/* Defines address mode or scale mode */
	uint8_t addr_or_scale_mode;

	/* Save register name, valid or not */
	uint8_t op1_name;
	uint8_t op2_name;
} modsib;

typedef struct op_addr {
	/* Contains pointer to register except for immidiate values */
	uint32_t *op1_reg;
	/* For immidiate */
	uint32_t op1_const;
	bool is_op1_high;

	/* Contains pointer to register in case of REGISTER addressment */
	uint32_t *op2_reg;
	/* Contains address in case of memory addressment */
	uint32_t op2_mem;

	/* High or Low Byte in case of 8 bit */
	bool is_op2_high;
} op_addr;

#endif /* __CPU_H_INCLUDED */
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
