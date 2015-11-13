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

	AH,
	AL,
	BH,
	BL,
	CH,
	CL,
	DH,
	DL
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
	/* Register beginning with letter 'e' have length of 32 bits */
	uint32_t eip;
	uint32_t ebp;
	uint32_t esp;
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
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
	//TODO
	bool is_op2_high;
} op_addr;

/* Method declarations
 * include static functions for documentation purposes!
 */
static cpu_register cpu_modrm_eval_register(cpu_state *cpu_state, cpu_register reg, uint32_t **reg_addr, bool is_8bit);
static bool         cpu_modrm_eval(cpu_state *cpu_state, modsib *mod, uint8_t byte, uint8_t is_8bit);

static bool cpu_decode_RM(cpu_state *cpu_state, op_addr *addr, bool is_8bit, bool has_imm);
static uint8_t cpu_get_byte_inc(cpu_state *);

static void cpu_compute_op_to_address(cpu_state *, uint8_t, uint32_t *, op_addr *);

static uint32_t cpu_read_32_bit_addr(cpu_state *cpu_state);
static uint32_t cpu_read_data_from_mem(cpu_state *cpu_state, uint32_t ram_addr);
static uint32_t cpu_read_byte_from_register(bool is_high, uint32_t *reg_addr);

static void cpu_write_byte_in_mem(cpu_state *cpu_state, uint8_t byte, uint32_t ram_addr);
static void cpu_write_byte_in_reg(uint8_t byte, uint32_t *reg_addr, bool is_high);

static void cpu_write_word_in_mem(cpu_state *cpu_state, uint32_t data, uint32_t ram_addr);
static void cpu_write_word_in_reg(uint32_t data, uint32_t *reg_addr);

static bool cpu_readb(void *_cpu_state, uint32_t addr, uint8_t *valp);
static bool cpu_writeb(void *_cpu_state, uint32_t addr, uint8_t val);

#endif /* __CPU_H_INCLUDED */
