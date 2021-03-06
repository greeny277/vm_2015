/* $Id: cpu.h 9992 2014-10-15 11:42:43Z raimue $ */

#ifndef __CPU_H_INCLUDED
#define __CPU_H_INCLUDED

#include <setjmp.h>
#include <signal.h>

#include "sig_host_bus.h"
#include "sig_boolean.h"

//vector size in the interrupt and global descriptor table in bytes
#define VECTOR_SIZE 8

typedef enum modrm_mod_bits {
	NO_DISPLACEMENT,
	DISPLACEMENT_8,
	DISPLACEMENT_32,
	REGISTER
} modrm_mod_bits;

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

//these enums have to be in the same order as the segment_registers
//in cpu_state: cs, ds, ss, es, fs, gs;
typedef enum cpu_segment {
	EXTRA,
	CODE,
	STACK,
	DATA,
	GENERAL_PURPOSE1,
	GENERAL_PURPOSE2,
	NOCHECK
} cpu_segment;

/** evaluate one instruction
  * @param s cpu instance
  * @return false if the cpu suspended, true if another instruction
  *         can be executed.
  */
extern bool
cpu_step(void *s);

/** create a cpu
  * @param port_host host bus instance
  * @param pic_to_cpu_bool bus interface connecting pic and cpu
  * @return cpu instance.
  */
extern void *
cpu_create(struct sig_host_bus *port_host);

/** destroy a cpu instance
  * @param s cpu instance
  */
extern void
cpu_destroy(void *s);

#define DATA_ACCESSED 1
#define DATA_WRITEABLE 2
#define DATA_EXPANDDOWN 4
#define CODE_ACCESSED 1
#define CODE_READABLE 2
#define CODE_CONFORMING 4
#define DATACODE 8

typedef struct segment_register {
	uint16_t public_part;
	uint32_t base_addr;
	uint32_t limit;
	uint8_t  type;
} segment_register;

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
	segment_register es, cs, ss, ds, fs, gs;

	/**
	 * EFLAGS register
	 * Bit Number   Meaning                 Shortcut
	 * 11           Overflow Flag             OF
	 *  9           Interrupt enable Flag     IF
	 *  7           Sign Flag                 SF
	 *  6           Zero Flag                 ZF
	 *  4           Auxiliary Carry Flag      AF
	 *  2           Parity Flag               PF
	 *  0           Carry Flag                CF
	 */
	uint32_t eflags;

	bool interrupt_raised;

	/**
	 * IDTR BASE and LIMIT
	 */
	uint32_t idtr_base;
	uint16_t idtr_limit;

	/**
	 * GDT BASE and LIMIT
	 */
	uint32_t gdtr_base;
	uint16_t gdtr_limit;

	void *saved_state;
	jmp_buf jump_target_new_instruction;
} cpu_state;

typedef enum flag {
	OVERFLOW_FLAG = 11,
	INTERRUPT_FLAG = 9,
	SIGN_FLAG =      7,
	ZERO_FLAG =      6,
	AUX_CARRY_FLAG = 4,
	PARITY_FLAG =    2,
	CARRY_FLAG =     0
} flag;

/*
 * This struct can be used for decoding SIB and ModRM byte
 */
typedef struct modsib {
	/* Operand 1 Reg: Can be used as SRC or DEST */
	uint32_t *mod_reg;

	/* Operand 2 Reg/Mem: Can be used as SRC or DEST */
	uint32_t *mod_rm;

	/* Defines address mode or scale mode */
	modrm_mod_bits addr_or_scale_mode;

	/* Save register name, valid or not */
	cpu_register mod_reg_name;
	cpu_register mod_rm_name;
} modsib;

typedef enum addr_type {
	REGISTER_WORD,
	REGISTER_HIGH,
	REGISTER_LOW,
	MEMORY
} addr_type;

typedef struct op_addr {
	/*decoded value of the reg and r/m bits*/
	uint32_t* reg;
	uint32_t* regmem_reg;
	uint32_t  regmem_mem;

	/*Value of reg bits for determining opcode */
	uint8_t reg_value;

	/*type of the reg and r/m bits' decoded values*/
	addr_type reg_type;
	addr_type regmem_type;
} op_addr;


/** raise the interrupt flag
  * @param instance cpu instance
  */
void
cpu_interrupt(cpu_state *instance);

#endif /* __CPU_H_INCLUDED */
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
