#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* DEBUG LIB */
#include <stdio.h>

#include "cpu.h"

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#define EIGHT_BIT true
#define IMMEDIATE true

#define HIGH_BYTE true

void *cpu_create(struct sig_host_bus *port_host);
void  cpu_destroy(void *_cpu_state);

static cpu_register cpu_modrm_eval_register(cpu_state *cpu_state, cpu_register reg, uint32_t **reg_addr, bool is_8bit);
static bool         cpu_modrm_eval(cpu_state *cpu_state, modsib *mod, uint8_t byte, uint8_t is_8bit);

static bool cpu_decode_RM(cpu_state *cpu_state, op_addr *addr, bool is_8bit, bool has_imm);
static void cpu_compute_op_to_address(cpu_state *cpu_state, uint8_t mode, uint32_t *addr, op_addr *op);

static uint8_t  cpu_read_byte_from_reg(uint32_t *reg_addr, bool is_high);
static uint8_t  cpu_read_word_from_reg(uint32_t *reg_addr);
static uint8_t  cpu_read_byte_from_ram(cpu_state *cpu_state);
static uint32_t cpu_read_word_from_ram(cpu_state *cpu_state);

static uint8_t  cpu_peek_byte_from_ram(cpu_state *cpu_state, uint32_t ram_addr);
static uint32_t cpu_peek_word_from_ram(cpu_state *cpu_state, uint32_t ram_addr);

static void cpu_write_byte_in_reg(uint8_t byte, uint32_t *reg_addr, bool is_high);
static void cpu_write_word_in_reg(uint32_t data, uint32_t *reg_addr);
static void cpu_write_byte_in_ram(cpu_state *cpu_state, uint8_t byte, uint32_t ram_addr);
static void cpu_write_word_in_ram(cpu_state *cpu_state, uint32_t word, uint32_t ram_addr);

static bool cpu_readb(void *_cpu_state, uint32_t addr, uint8_t *valp);
static bool cpu_writeb(void *_cpu_state, uint32_t addr, uint8_t val);

static void cpu_set_carry_sub(cpu_state *cpu_state, uint32_t, uint32_t);

static void cpu_set_carry_add(cpu_state *cpu_state, uint32_t first_summand, uint32_t second_summand);
/** @brief "constructor" of the cpu
 *
 *  @param port_host  the port the cpu is connected to
 */
void *
cpu_create(struct sig_host_bus *port_host) {
	struct cpu_state *cpu_state;
	static const struct sig_host_bus_funcs hf = {
		.readb = cpu_readb,
		.writeb = cpu_writeb
	};

	cpu_state = malloc(sizeof(struct cpu_state));
	assert(cpu_state != NULL);

	cpu_state->port_host = port_host;
	/* Set base pointer to start address of ROM.
	 * The address is hardcoded, like in real hardware.
	 */
	cpu_state->eip = 0xE000;

	sig_host_bus_connect(port_host, cpu_state, &hf);
	return cpu_state;
}

/** @brief "destructor" of the cpu
 *
 *  @param _cpu_state  the cpu instance
 */
void
cpu_destroy(void *_cpu_state) {
	free(_cpu_state);
}

/** @brief Set carry bit in eflag for addition
 *
 *  @param first_summand the first operand of the additon
 *  @param second_summand the second operand of the additon
 */
static void cpu_set_carry_add(cpu_state *cpu_state, uint32_t first_summand, uint32_t second_summand){
	if(first_summand + second_summand < first_summand){
		cpu_state->eflags |= 0x01;
	} else {
		cpu_state->eflags &= ~0x01;
	}
}
/** @brief Set carry bit in eflag for subtraction
 *
 *  @param minuend the first operand of the subtraction
 *  @param subtrahend the second operand of the subtraction
 */
static void cpu_set_carry_sub(cpu_state *cpu_state, uint32_t minuend, uint32_t subtrahend){
	if(minuend < subtrahend){
		cpu_state->eflags |= 0x01;
	} else {
		cpu_state->eflags &= ~0x01;
	}
}
/** @brief Interpret the reg bits of the mod-reg-r/m byte
 *
 * @param cpu_state  CPU instance
 *
 * @param cpu_register  The register to address
 *
 * @param reg_addr  Pointer that holds the register
 *     address when register was valid
 *
 * @return The register which was read on success
 *         0xff in case of an error
 */
static cpu_register
cpu_modrm_eval_register(cpu_state *cpu_state, cpu_register reg, uint32_t **reg_addr, bool is_8bit) {
	//unlikely: gcc macro specifying that the 'then' path is unlikely
	if(unlikely(reg > EDI))
		return 0xff;

	if(likely(!is_8bit)){
		*reg_addr = &(cpu_state->eax)+reg;
		return reg;
	}else{
		if(reg > 0b011)
			*reg_addr = &(cpu_state->eax)+reg-4;
		else
			*reg_addr = &(cpu_state->eax)+reg;
		return reg+8;
	}
}

/** @brief  Interpret the mod-reg-r/m byte
 * Bits 7..6  Scale or Addressing Mode
 * Bits 5..3  Register of first operand
 * Bits 2..0  Register of second operand
 *
 * @param cpu_state    CPU instance
 * @param mod      Pointer to structure where results will be saved in
 * @param byte     byte that will be analysed
 * @param is_8bit  Is it an 8 bit instruction
 *
 * @return         True on success.
 *                 False in case of a huge programming mistake.
 */
static bool
cpu_modrm_eval(cpu_state *cpu_state, modsib *mod, uint8_t byte, uint8_t is_8bit) {
	uint32_t *op1;
	uint32_t *op2;

	mod->op1_name = cpu_modrm_eval_register(cpu_state, byte & (0x7), &op1, is_8bit);
	if( -1 == mod->op1_name) {
		return false;
	}

	mod->op2_name = cpu_modrm_eval_register(cpu_state, (byte >> 3) & 0x7, &op2, is_8bit);
	if( -1 == mod->op2_name) {
		return false;
	}

	mod->addr_or_scale_mode = (byte >> 6) & (0x7);
	mod->op1 = op1;
	mod->op2 = op2;

	return true;
}

/** @brief Read and interpret MOD_RM, SIB and DISPLACEMENT bytes
 * Furthermore compute the addresses of operand 1 and 2. Either
 * as pointer to a register or virtual address for the RAM component
 *
 * @param cpu_state    CPU instance
 *
 * @param addr     Pointer to structure where virtual address
 *                 of memory or register addresses of the cpu
 *                 instance will be saved in.
 *
 * @param is_8bit  Is it an 8 bit instruction
 *
 * @return         True when everything works fine.
 *                 False when an error occured.
 */
static bool
cpu_decode_RM(cpu_state *cpu_state, op_addr *addr, bool is_8bit, bool has_imm) {
	uint8_t mod_rm = 0;

	/* Eval MOD_RM Byte */
	mod_rm = cpu_read_byte_from_ram(cpu_state);
	modsib s_modrm;
	memset(&mod_rm, 0, sizeof(modsib));

	if(false == cpu_modrm_eval(cpu_state, &s_modrm, mod_rm, is_8bit)){
		return false;
	}

	/* Set high flag in 8bit mode */
	if(is_8bit){
		if(s_modrm.op1_name == AL || s_modrm.op1_name == BL ||
				s_modrm.op1_name == CL || s_modrm.op1_name == DL){
			addr->is_op1_high = !HIGH_BYTE;
		} else {
			addr->is_op1_high = HIGH_BYTE;
		}

		if(s_modrm.op2_name == AL || s_modrm.op2_name == BL ||
				s_modrm.op2_name == CL || s_modrm.op2_name == DL){
			addr->is_op1_high = !HIGH_BYTE;
		} else {
			addr->is_op1_high = HIGH_BYTE;
		}
	}

	/* Check if SIB byte follows */
	if(s_modrm.op2_name == ESP && s_modrm.addr_or_scale_mode != REGISTER){
		/* Define variables for SIB byte */
		uint32_t *base;
		uint32_t *index;

		uint8_t sib = 0;
		uint8_t scale = 0;


		/* Read next byte increment eip */
		sib = cpu_read_byte_from_ram(cpu_state);
		modsib s_sib;
		memset(&s_sib,0,sizeof(modsib));

		if(false == cpu_modrm_eval(cpu_state, &s_sib, sib, is_8bit)){
				return false;
		}

		scale = s_sib.addr_or_scale_mode;
		scale = 1 << scale;
		base = s_sib.op2;
		index = s_sib.op1;

		/* Compute base address for op2 */
		uint32_t base_op2;
		if(s_sib.op1_name == ESP) {
			/* Index is not used */
			base_op2 = *base;

		} else if (s_sib.op2_name == EBP){
			/* Base is not used */
			base_op2= *index * scale;

		} else {
			base_op2 = *base + (*index * scale);
		}
		/* Remember: addressing mode is unequal to REGISTER! */
		cpu_compute_op_to_address(cpu_state, s_modrm.addr_or_scale_mode, &base_op2, addr);

	} else if(s_modrm.op2_name == EBP && s_modrm.addr_or_scale_mode == NO_DISPLACEMENT){
		/* Special case: Reject op2 and read in a 32 Bit displacement instead. */
		uint32_t base_0 = 0;
		cpu_compute_op_to_address(cpu_state, DISPLACEMENT_32, &base_0, addr);
	} else {
		/* Compute address of op2 */
		cpu_compute_op_to_address(cpu_state, s_modrm.addr_or_scale_mode, s_modrm.op2, addr);
	}

	/* Set op1 to reg address or as a constant value */
	if(has_imm && is_8bit){
			addr->op1_const = cpu_read_byte_from_ram(cpu_state);
	} else if(has_imm){
			addr->op1_const = cpu_read_word_from_ram(cpu_state);
	} else{
			addr->op1_reg = s_modrm.op1;
	}
	return true;
}

/** @brief          Compute address of operand 2 subjected to the addressing mode
 *
 * @param cpu_state CPU instance
 *
 * @param mode      Contains addressing mode
 *
 * @param addr      Pointer that contains the base address. Usually
 *                  a pointer to CPU own register.
 */
static void
cpu_compute_op_to_address(cpu_state *cpu_state, uint8_t mode, uint32_t *addr, op_addr *op) {
	uint8_t displ1;
	uint32_t displacement_complete;
	switch(mode){
		case NO_DISPLACEMENT:
			/* Indirection with no displacement */
			op->op2_mem = *addr;
			return;

		case DISPLACEMENT_8:
			/* Read one extra byte from bus */
			displ1 = cpu_read_byte_from_ram(cpu_state);
			/* Indirection with 8 bit displacement */
			op->op2_mem = displ1 + (*addr);
			return;

		case DISPLACEMENT_32:
			/* Indirection with 32 bit displacement */

			/* Attention: Lowest byte will be read first */
			displacement_complete = cpu_read_word_from_ram(cpu_state);
			op->op2_mem = displacement_complete + (*addr);
			return;
		case REGISTER:
			op->op2_reg = addr;
			return;
	}
	return;
}

/*
 *  Reading and peeking functions
 */
/** @brief read a register and return the high or low byte
 *
 * @param reg_addr  the address of the register to read
 * @param is_high   whether to return the high byte
 *
 * @return the byte read
 */
static uint8_t
cpu_read_byte_from_reg(uint32_t *reg_addr, bool is_high) {
	if(is_high) {
		return (*reg_addr >> 8) & 0xff;
	}

	return *reg_addr & 0xff;
}

/** @brief read a register and return the word
 *
 * @param reg_addr  the address of the register to read
 *
 * @return the byte read
 */
static uint8_t
cpu_read_word_from_reg(uint32_t *reg_addr) {
	return *reg_addr;
}

/** @brief read byte at the instruction pointer's address from RAM and increment IP
 *
 * @param cpu_state CPU instance
 *
 * @return the byte read
 */
static uint8_t
cpu_read_byte_from_ram(cpu_state *cpu_state) {
	uint8_t next_byte = sig_host_bus_readb(cpu_state->port_host, (void *)cpu_state, cpu_state->eip);
	cpu_state->eip = cpu_state->eip + 1;

	return next_byte;
}

/** @brief read a word (4 byte) at the instruction pointer's address from RAM
 *         and increment IP by 4
 *
 * @param cpu_state CPU instance
 *
 * @return the word read
*/
static uint32_t
cpu_read_word_from_ram(cpu_state *cpu_state) {
		uint8_t displ1, displ2, displ3, displ4;
		uint32_t displacement_complete;

		displ1 = cpu_read_byte_from_ram(cpu_state);
		displ2 = cpu_read_byte_from_ram(cpu_state);
		displ3 = cpu_read_byte_from_ram(cpu_state);
		displ4 = cpu_read_byte_from_ram(cpu_state);

		displacement_complete = displ1;
		displacement_complete |= (displ2 << 8);
		displacement_complete |= (displ3 << 16);
		displacement_complete |= (displ4 << 24);

		return displacement_complete;
}

/** @brief read byte at given address from RAM. keeps IP untouched.
 *
 * @param cpu_state CPU instance
 * @param ram_addr  the address to read at
 *
 * @return the byte read
 */
static uint8_t
cpu_peek_byte_from_ram(cpu_state *cpu_state, uint32_t ram_addr) {
	return sig_host_bus_readb(cpu_state->port_host, cpu_state, ram_addr);
}

/** @brief read a word (4 byte) at given address from RAM. keeps IP untouched.
 *
 * @param cpu_state CPU instance
 * @param ram_addr  the address to read at
 *
 * @return the word read
 */
static uint32_t
cpu_peek_word_from_ram(cpu_state *cpu_state, uint32_t ram_addr) {

	uint32_t data;
	uint8_t byte1, byte2, byte3, byte4;
	byte1 = sig_host_bus_readb(cpu_state->port_host, cpu_state, ram_addr);
	ram_addr++;

	byte2 = sig_host_bus_readb(cpu_state->port_host, cpu_state, ram_addr);
	ram_addr++;

	byte3 = sig_host_bus_readb(cpu_state->port_host, cpu_state, ram_addr);
	ram_addr++;

	byte4 = sig_host_bus_readb(cpu_state->port_host, cpu_state, ram_addr);

	data = byte1;
	data |= (byte2 << 8);
	data |= (byte3 << 16);
	data |= (byte4 << 24);

	return data;
}


/*
 *  Writing functions
 */

/** @brief write a byte to a register's high or low byte
 *
 * @param byte      the byte to write
 * @param reg_addr  the address of the register to read
 * @param is_high   whether to write the high byte
 */
static void
cpu_write_byte_in_reg(uint8_t byte, uint32_t *reg_addr, bool is_high) {
	if(!is_high) {
		*reg_addr &= ~0xff;
		*reg_addr |= byte;
	} else {
		*reg_addr &= ~0xff00;
		*reg_addr |= (byte << 8);

	}
}

/** @brief write a word (4 byte) to a register
 *
 * @param byte      the byte to write
 * @param reg_addr  the address of the register to read
 */
static void
cpu_write_word_in_reg(uint32_t data, uint32_t *reg_addr) {
	*reg_addr = data;
}

/** @brief write a byte to the ram
 *
 * @param cpu_state  the cpu instance
 * @param byte       the byte to write
 * @param ram_addr   the address in the ram to write to
 */
static void
cpu_write_byte_in_ram(cpu_state *cpu_state, uint8_t byte, uint32_t ram_addr) {
	sig_host_bus_writeb(cpu_state->port_host, cpu_state, ram_addr, byte);
}

/** @brief write a word (4 byte) to the ram
 *
 * @param cpu_state  the cpu instance
 * @param word       the word to write
 * @param ram_addr   the address in the ram to write to
 */
static void
cpu_write_word_in_ram(cpu_state *cpu_state, uint32_t word, uint32_t ram_addr) {
	uint8_t byte = word & 0xff;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, ram_addr, byte);
	byte = (word >> 8) & 0xff;
	ram_addr++;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, ram_addr, byte);
	byte = (word >> 16) & 0xff;
	ram_addr++;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, ram_addr, byte);
	byte = (word >> 24) & 0xff;
	ram_addr++;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, ram_addr, byte);
}



bool
cpu_step(void *_cpu_state) {
	/* cast */
	cpu_state *cpu_state = (struct cpu_state *) _cpu_state;

	uint8_t op_code;

	uint8_t eight_bit_src;
	uint32_t four_byte_src;

	/* read the first byte from instruction pointer and increment ip
	 * afterards */
	op_code = cpu_read_byte_from_ram(cpu_state);

	op_addr s_op;
	memset(&s_op, 0, sizeof(op_addr));

	switch(op_code) {
			case 0x88:
				/* Copy r8 to r/m8 */
				if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT, !IMMEDIATE)){
					uint8_t src = cpu_read_byte_from_reg(s_op.op1_reg, s_op.is_op1_high);
					if(s_op.op2_reg != 0){
						/* Write in a register */
						cpu_write_byte_in_reg(src, s_op.op2_reg, s_op.is_op2_high);
					} else {
						/* Write in memory */
						cpu_write_byte_in_ram(cpu_state, src, s_op.op2_mem);
					}
					return true;
				}
				break;
			case 0x89:
				/* Copy r32 to r/m32 */
				if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT, !IMMEDIATE)){
					uint32_t src = *(s_op.op1_reg);
					if(s_op.op2_reg != 0){
						/* Write in a register */
						cpu_write_word_in_reg(src, s_op.op2_reg);
					} else {
						/* Write in memory */
						cpu_write_word_in_ram(cpu_state, src, s_op.op2_mem);
					}
					return true;
				}
				break;

			case 0x8A:
				/* Copy r/m8 to r8. */
				if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT, !IMMEDIATE)){
					uint8_t src;
					if(s_op.op2_reg != 0){
						/* Write in a register */
						src = cpu_read_byte_from_reg(s_op.op2_reg, s_op.is_op2_high);
					} else {
						/* Write in memory */
						src = cpu_peek_byte_from_ram(cpu_state, s_op.op2_mem);
					}
					cpu_write_byte_in_reg(src, s_op.op1_reg, s_op.is_op1_high);
					return true;
				}
				break;

			case 0x8B:
				/* Copy r/m32 to r32. */
				if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT, !IMMEDIATE)){
					uint32_t src;
					if(s_op.op2_reg != 0){
						/* Write in a register */
						src = cpu_read_word_from_reg(s_op.op2_reg);
					} else {
						/* Write in memory */
						src = cpu_peek_word_from_ram(cpu_state, s_op.op2_mem);
					}
					cpu_write_word_in_reg(src, s_op.op1_reg);
					return true;
				}
				break;

			case 0xA0:
			 /* Copy byte at (seg:offset) to AL */
				fprintf(stderr, "0xA0 is not implemented yet\n");
				break;

			case 0xA1:
			 /* Copy doubleword at (seg:offset) to EAX */
				fprintf(stderr, "0xA1 is not implemented yet\n");
				break;

			case 0xA2:
			 /* Copy AL to (seg:offset) */
				fprintf(stderr, "0xA2 is not implemented yet\n");
				break;

			case 0xA3:
			 /* Copy EAX to (seg:offset) */
				fprintf(stderr, "0xA3 is not implemented yet\n");
				break;

			case 0xB0:
				/* Copy imm8 to AL */
				eight_bit_src = cpu_read_byte_from_ram(cpu_state);
				cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->eax), !HIGH_BYTE);
				return true;

			case 0xB1:
				/* Copy imm8 to CL */
				eight_bit_src = cpu_read_byte_from_ram(cpu_state);
				cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->ecx), !HIGH_BYTE);
				return true;

			case 0xB2:
				/* Copy imm8 to DL */
				eight_bit_src = cpu_read_byte_from_ram(cpu_state);
				cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->edx), !HIGH_BYTE);
				return true;

			case 0xB3:
				/* Copy imm8 to BL */
				eight_bit_src = cpu_read_byte_from_ram(cpu_state);
				cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->ebx), !HIGH_BYTE);
				return true;

			case 0xB4:
				/* Copy imm8 to AH */
				eight_bit_src = cpu_read_byte_from_ram(cpu_state);
				cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->eax), HIGH_BYTE);
				return true;

			case 0xB5:
				/* Copy imm8 to CH */
				eight_bit_src = cpu_read_byte_from_ram(cpu_state);
				cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->ecx), HIGH_BYTE);
				return true;

			case 0xB6:
				/* Copy imm8 to DH */
				eight_bit_src = cpu_read_byte_from_ram(cpu_state);
				cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->edx), HIGH_BYTE);
				return true;

			case 0xB7:
				/* Copy imm8 to BH */
				eight_bit_src = cpu_read_byte_from_ram(cpu_state);
				cpu_write_byte_in_reg(eight_bit_src, &(cpu_state->ebx), HIGH_BYTE);
				return true;

			case 0xB8:
				/* Copy imm32 to EAX */
				four_byte_src = cpu_read_word_from_ram(cpu_state);
				cpu_write_word_in_reg(four_byte_src, &(cpu_state->eax));
				return true;

			case 0xB9:
				/* Copy imm32 to ECX */
				four_byte_src = cpu_read_word_from_ram(cpu_state);
				cpu_write_word_in_reg(four_byte_src, &(cpu_state->ecx));
				return true;

			case 0xBA:
				/* Copy imm32 to EDX */
				four_byte_src = cpu_read_word_from_ram(cpu_state);
				cpu_write_word_in_reg(four_byte_src, &(cpu_state->edx));
				return true;

			case 0xBB:
				/* Copy imm32 to EBX */
				four_byte_src = cpu_read_word_from_ram(cpu_state);
				cpu_write_word_in_reg(four_byte_src, &(cpu_state->ebx));
				return true;

			case 0xC6:
				/* Copy imm8 to r/m8. */
				if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT, IMMEDIATE)){
					uint8_t src = s_op.op1_const;
					if(s_op.op2_reg != 0){
						/* Write in a register */
						cpu_write_byte_in_reg(src, s_op.op2_reg, s_op.is_op2_high);
					} else {
						/* Write in memory */
						cpu_write_byte_in_ram(cpu_state, src, s_op.op2_mem);
					}
					return true;
				}
				break;
			case 0xC7:
				/* Copy imm32 to r/m32. */
				if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT, IMMEDIATE)){
					uint32_t src = s_op.op1_const;
					if(s_op.op2_reg != 0){
						/* Write in a register */
						cpu_write_word_in_reg(src, s_op.op2_reg);
					} else {
						/* Write in memory */
						cpu_write_word_in_ram(cpu_state, src, s_op.op2_mem);
					}
					return true;
				}
				break;

			default:
				break;
	}
	return false;
}

/** @brief nop - cpu does not support reading via bus
 */
static bool
cpu_readb(void *_cpu_state, uint32_t addr, uint8_t *valp) {
	/* We dont read from CPU */

	return false;
}

/** @brief nop - cpu does not support writing via bus
 */
static bool
cpu_writeb(void *_cpu_state, uint32_t addr, uint8_t val) {
	/* We dont write into CPU */

	return false;
}
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
