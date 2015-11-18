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

#define SUBTRACTION true

#define HIGH_BYTE true

void *cpu_create(struct sig_host_bus *port_host);
void  cpu_destroy(void *_cpu_state);

static cpu_register cpu_modrm_eval_register(cpu_state *cpu_state, cpu_register reg, uint32_t **reg_addr, bool is_8bit);
static bool         cpu_modrm_eval(cpu_state *cpu_state, modsib *mod, uint8_t byte, uint8_t is_8bit);

static bool cpu_decode_RM(cpu_state *cpu_state, op_addr *addr, bool is_8bit);
static void cpu_set_opaddr_regmem(cpu_state *cpu_state, uint8_t mode, uint32_t *addr, op_addr *op);

/*FIXME Rename read methods on a register to peek */
static uint8_t  cpu_read_byte_from_reg(uint32_t *reg_addr, bool is_high);
static uint32_t  cpu_read_word_from_reg(uint32_t *reg_addr);
static uint8_t  cpu_read_byte_from_mem(cpu_state *cpu_state);
static uint32_t cpu_read_word_from_mem(cpu_state *cpu_state);

static uint8_t  cpu_peek_byte_from_mem(cpu_state *cpu_state, uint32_t mem_addr);
static uint32_t cpu_peek_word_from_mem(cpu_state *cpu_state, uint32_t mem_addr);

static void cpu_write_byte_in_reg(uint8_t byte, uint32_t *reg_addr, bool is_high);
static void cpu_write_word_in_reg(uint32_t data, uint32_t *reg_addr);
static void cpu_write_byte_in_mem(cpu_state *cpu_state, uint8_t byte, uint32_t mem_addr);
static void cpu_write_word_in_mem(cpu_state *cpu_state, uint32_t word, uint32_t mem_addr);

static void cpu_stack_push_byte(cpu_state *cpu_state, uint8_t byte);
static void cpu_stack_push_doubleword(cpu_state *cpu_state, uint32_t doubleword);
static uint8_t cpu_stack_pop_byte(cpu_state *cpu_state);
static uint32_t cpu_stack_pop_doubleword(cpu_state *cpu_state);

static bool cpu_readb(void *_cpu_state, uint32_t addr, uint8_t *valp);
static bool cpu_writeb(void *_cpu_state, uint32_t addr, uint8_t val);

static void cpu_set_carry_add(cpu_state *cpu_state, uint32_t first_summand, uint32_t result);
static void cpu_set_carry_sub(cpu_state *cpu_state, uint32_t minuend, uint32_t subtrahend);

static void cpu_set_overflow_add(cpu_state *cpu_state, uint32_t summand_fst, uint32_t summand_snd, uint32_t result, bool is_8bit);
static void cpu_set_overflow_sub(cpu_state *cpu_state, uint32_t minuend, uint32_t subtrahend, uint32_t result, bool is_8bit);

static void cpu_set_sign_flag(cpu_state *cpu_state, uint32_t result, bool is_8bit);
static void cpu_set_zero_flag(cpu_state *cpu_state, uint32_t result);
static void cpu_set_eflag_arith(cpu_state *cpu_state, uint32_t op1, uint32_t op2, uint32_t result, bool is_8bit, bool is_subtraction);
static void cpu_set_eip(cpu_state *cpu_state, uint32_t new_addr);

static bool cpu_get_carry_flag(cpu_state *cpu_state);
static bool cpu_get_overflow_flag(cpu_state *cpu_state);
static bool cpu_get_sign_flag(cpu_state *cpu_state);
static bool cpu_get_zero_flag(cpu_state *cpu_state);

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

/** @brief Set instruction pointer
 *
 *  @param cpu_state is the cpu instance
 *  @param new_addr is the new memory address where the
 *                  next instruction will be loaded from
 */
static void cpu_set_eip(cpu_state *cpu_state, uint32_t new_addr){
	cpu_state->eip = new_addr;
}

/** @brief "destructor" of the cpu
 *
 *  @param _cpu_state  the cpu instance
 */
void
cpu_destroy(void *_cpu_state) {
	free(_cpu_state);
}

/** @brief Set flags for addition or subtraction in eflag register
 *
 *  @param cpu_state is instance of the cpu
 *  @param op1 is the first operand of the arithmetic operation
 *  @param op2 is the second operand of the arithmetic operation
 *  @param result is the result of the arithmetic operation
 *  @param is_8bit indicates if 8bit modus is on or not
 *  @param is_subtraction indicates if a subtraction was performed
 */
static void
cpu_set_eflag_arith(cpu_state *cpu_state, uint32_t op1, uint32_t op2, uint32_t result, bool is_8bit, bool is_subtraction){
	if(is_subtraction){
		cpu_set_carry_sub(cpu_state, op1, op2);
		cpu_set_overflow_sub(cpu_state, op1, op2, result, is_8bit);
	} else {
		cpu_set_carry_add(cpu_state, op1, result);
		cpu_set_overflow_add(cpu_state, op1, op2, result, is_8bit);
	}
	cpu_set_sign_flag(cpu_state, result, is_8bit);
	cpu_set_zero_flag(cpu_state, result);
}
/** @brief Set sign bit in EFLAG register
 *
 */
static void cpu_set_sign_flag(cpu_state *cpu_state, uint32_t result, bool is_8bit){
	cpu_state->eflags &= ~(1 << SIGN_FLAG);
	if(is_8bit){
		cpu_state->eflags |= !!(result & 0x80) << SIGN_FLAG; //!! makes non-zero ints 1
	} else {
		cpu_state->eflags |= !!(result & 0x80000000) << SIGN_FLAG; //!! makes non-zero ints 1
	}
	return;
}

/** @brief Set carry bit in eflag for addition
 *
 *  @param summand_fst the first operand of the additon
 *  @param result  result of addition
 */
static void cpu_set_carry_add(cpu_state *cpu_state, uint32_t summand_fst, uint32_t result){
	cpu_state->eflags &= ~(1 << CARRY_FLAG);
	cpu_state->eflags |= (result < summand_fst) << CARRY_FLAG;
}

/** @brief Set carry bit in eflag for subtraction
 *
 *  @param minuend the first operand of the subtraction
 *  @param subtrahend the second operand of the subtraction
 */
static void cpu_set_carry_sub(cpu_state *cpu_state, uint32_t minuend, uint32_t subtrahend){
	cpu_state->eflags &= ~(1 << CARRY_FLAG);
	cpu_state->eflags |= (minuend < subtrahend) << CARRY_FLAG;
}

/** @brief Set overflow bit in eflag for addition
 *
 *  @param summand_fst the first operand of the additon
 *  @param summand_snd the second operand of the additon
 *  @param result the result of the addtion
 *  @param is_8bit indicates 8bit operation. Whether sign bit
 *                 is at positon 7 or 31.
 */
static void cpu_set_overflow_add(cpu_state *cpu_state, uint32_t summand_fst, uint32_t summand_snd, uint32_t result, bool is_8bit){
	if(is_8bit){
		if((!((summand_fst >> 7) ^ (summand_snd >> 7))) && ((summand_fst >> 7) ^ (result >> 7))){
			cpu_state->eflags |= (1 << OVERFLOW_FLAG);
		} else {
			cpu_state->eflags &= ~(1 << OVERFLOW_FLAG);
		}
	} else {
		if((!((summand_fst >> 31) ^ (summand_snd >> 31))) && ((summand_fst >> 31) ^ (result >> 31))){
			cpu_state->eflags |= (1 << OVERFLOW_FLAG);
		} else {
			cpu_state->eflags &= ~(1 << OVERFLOW_FLAG);
		}
	}
}

/** @brief Set overflow bit in eflag for subtraction
 *
 *  @param minuend the first operand of the subtraction
 *  @param subtrahend the second operand of the subtraction
 *  @param result the result of the subtraction
 *  @param is_8bit indicates 8bit operation. Whether sign bit
 *                 is at positon 7 or 31.
 */
static void cpu_set_overflow_sub(cpu_state *cpu_state, uint32_t minuend, uint32_t subtrahend, uint32_t result, bool is_8bit){
	if(is_8bit){
		if(((minuend >> 7) ^ (subtrahend >> 7)) && ((minuend >> 7) ^ (result >> 7))){
			cpu_state->eflags |= (1 << OVERFLOW_FLAG);
		} else {
			cpu_state->eflags &= ~(1 << OVERFLOW_FLAG);
		}
	} else {
		if(((minuend >> 31) ^ (subtrahend >> 31)) && ((minuend >> 31) ^ (result >> 31))){
			cpu_state->eflags |= (1 << OVERFLOW_FLAG);
		} else {
			cpu_state->eflags &= ~(1 << OVERFLOW_FLAG);
		}
	}
}

/** @brief Set zero bit in eflag
 *
 *  @param cpu_state the cpu instance
 *  @param result the result of the operation
 */
static void cpu_set_zero_flag(cpu_state *cpu_state, uint32_t result){
	cpu_state->eflags &= ~(1 << ZERO_FLAG);
	cpu_state->eflags |= (!result) << ZERO_FLAG;
}

/** @brief returns sign flag in EFLAG register
 *
 *  @return true when sign flag is set
 *          false else
 */
static bool cpu_get_sign_flag(cpu_state *cpu_state){
	return cpu_state->eflags & (1 << SIGN_FLAG);
}

/** @brief returns zero flag in EFLAG register
 *
 *  @return true when zero flag is set
 *          false else
 */
static bool cpu_get_zero_flag(cpu_state *cpu_state){
	return cpu_state->eflags & (1 << ZERO_FLAG);
}

/** @brief returns overflow flag in EFLAG register
 *
 *  @return true when overflow flag is set
 *          false else
 */
static bool cpu_get_overflow_flag(cpu_state *cpu_state){
	return cpu_state->eflags & (1 << OVERFLOW_FLAG);
}

/** @brief returns carry flag in EFLAG register
 *
 *  @return true when carry flag is set
 *          false else
 */
static bool cpu_get_carry_flag(cpu_state *cpu_state){
	return cpu_state->eflags & (1 << CARRY_FLAG);
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
 * as pointer to a register or virtual address for the memory component
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
cpu_decode_RM(cpu_state *cpu_state, op_addr *addr, bool is_8bit) {
	uint8_t mod_rm = 0;

	/* Eval MOD_RM Byte */
	mod_rm = cpu_read_byte_from_mem(cpu_state);
	modsib s_modrm;
	memset(&s_modrm, 0, sizeof(modsib));
	// memset(addr, 0, sizeof(op_addr));

	if(false == cpu_modrm_eval(cpu_state, &s_modrm, mod_rm, is_8bit)){
		return false;
	}

	addr->reg_value = s_modrm.op1_name;

	/*
	 * set reg part, this is straightforward
	 */
	if(!is_8bit){//TODO: Alle is_8bit dinge auf unlikely stellen?
		addr->reg_type = REGISTER_WORD;
	}else{
		if(s_modrm.op1_name < AH){
			addr->reg_type = REGISTER_LOW;
		} else {
			addr->reg_type = REGISTER_HIGH;
		}
	}
	addr->reg = s_modrm.op1;

	/*
	 * set regmem part, this the tricky part
	 */
	if(s_modrm.addr_or_scale_mode == REGISTER){
		if(!is_8bit){
			addr->reg_type = REGISTER_WORD;
		}else{
			if(s_modrm.op2_name < AH){
				addr->regmem_type = REGISTER_LOW;
			} else {
				addr->regmem_type = REGISTER_HIGH;
			}
		}

		addr->regmem_reg = s_modrm.op2;
	}else{
		addr->regmem_type = MEMORY;

		if(s_modrm.op2_name == ESP){
			/* We have a SIB byte following */
			uint32_t *base;
			uint32_t *index;

			uint8_t sib = 0;
			uint8_t scale = 0;

			/* Read next byte increment eip */
			sib = cpu_read_byte_from_mem(cpu_state);
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
			cpu_set_opaddr_regmem(cpu_state, s_modrm.addr_or_scale_mode, &base_op2, addr);

		} else if(s_modrm.op2_name == EBP && s_modrm.addr_or_scale_mode == NO_DISPLACEMENT){
			/* Special case: Reject op2 and read in a 32 Bit displacement instead. */
			uint32_t base_0 = 0;
			cpu_set_opaddr_regmem(cpu_state, DISPLACEMENT_32, &base_0, addr);
		} else {
			/* Compute address of op2 */
			cpu_set_opaddr_regmem(cpu_state, s_modrm.addr_or_scale_mode, s_modrm.op2, addr);
		}
	}
	return true;
}

/** @brief          Compute address of operand 2 subjected to the addressing mode
 *
 * @param cpu_state CPU instance
 *
 * @param mode      Contains addressing mode
 *
 * @param base_addr Pointer that contains the base address. Usually
 *                  a pointer to CPU own register.
 *
 * @param op        Pointer to the op_addr to write to
 */
static void
cpu_set_opaddr_regmem(cpu_state *cpu_state, uint8_t mode, uint32_t *base_addr, op_addr *op) {
	uint8_t displ1;
	uint32_t displacement_complete;
	switch(mode){
		case NO_DISPLACEMENT:
			/* Indirection with no displacement */
			op->regmem_mem = *base_addr;
			return;

		case DISPLACEMENT_8:
			/* Read one extra byte from bus */
			displ1 = cpu_read_byte_from_mem(cpu_state);
			/* Indirection with 8 bit displacement */
			op->regmem_mem = displ1 + (*base_addr);
			return;

		case DISPLACEMENT_32:
			/* Indirection with 32 bit displacement */

			/* Attention: Lowest byte will be read first */
			displacement_complete = cpu_read_word_from_mem(cpu_state);
			op->regmem_mem = displacement_complete + (*base_addr);
			return;
		case REGISTER:
			op->regmem_reg = base_addr;
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
static uint32_t
cpu_read_word_from_reg(uint32_t *reg_addr) {
	return *reg_addr;
}

/** @brief read byte at the instruction pointer's address from memory and increment IP
 *
 * @param cpu_state CPU instance
 *
 * @return the byte read
 */
static uint8_t
cpu_read_byte_from_mem(cpu_state *cpu_state) {
	uint8_t next_byte = sig_host_bus_readb(cpu_state->port_host, (void *)cpu_state, cpu_state->eip);
	cpu_state->eip = cpu_state->eip + 1;

	return next_byte;
}

/** @brief read a word (4 byte) at the instruction pointer's address from memory
 *         and increment IP by 4
 *
 * @param cpu_state CPU instance
 *
 * @return the word read
 */
static uint32_t
cpu_read_word_from_mem(cpu_state *cpu_state) {
	uint8_t displ1, displ2, displ3, displ4;
	uint32_t displacement_complete;

	displ1 = cpu_read_byte_from_mem(cpu_state);
	displ2 = cpu_read_byte_from_mem(cpu_state);
	displ3 = cpu_read_byte_from_mem(cpu_state);
	displ4 = cpu_read_byte_from_mem(cpu_state);

	displacement_complete = displ1;
	displacement_complete |= (displ2 << 8);
	displacement_complete |= (displ3 << 16);
	displacement_complete |= (displ4 << 24);

	return displacement_complete;
}

/** @brief read byte at given address from memory. keeps IP untouched.
 *
 * @param cpu_state CPU instance
 * @param mem_addr  the address to read at
 *
 * @return the byte read
 */
static uint8_t
cpu_peek_byte_from_mem(cpu_state *cpu_state, uint32_t mem_addr) {
	return sig_host_bus_readb(cpu_state->port_host, cpu_state, mem_addr);
}

/** @brief read a word (4 byte) at given address from memory. keeps IP untouched.
 *
 * @param cpu_state CPU instance
 * @param mem_addr  the address to read at
 *
 * @return the word read
 */
static uint32_t
cpu_peek_word_from_mem(cpu_state *cpu_state, uint32_t mem_addr) {

	uint32_t data;
	uint8_t byte1, byte2, byte3, byte4;
	byte1 = sig_host_bus_readb(cpu_state->port_host, cpu_state, mem_addr);
	mem_addr++;

	byte2 = sig_host_bus_readb(cpu_state->port_host, cpu_state, mem_addr);
	mem_addr++;

	byte3 = sig_host_bus_readb(cpu_state->port_host, cpu_state, mem_addr);
	mem_addr++;

	byte4 = sig_host_bus_readb(cpu_state->port_host, cpu_state, mem_addr);

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

/** @brief write a byte to the memory
 *
 * @param cpu_state  the cpu instance
 * @param byte       the byte to write
 * @param mem_addr   the address in the mem to write to
 */
static void
cpu_write_byte_in_mem(cpu_state *cpu_state, uint8_t byte, uint32_t mem_addr) {
	sig_host_bus_writeb(cpu_state->port_host, cpu_state, mem_addr, byte);
}

/** @brief write a word (4 byte) to the memory
 *
 * @param cpu_state  the cpu instance
 * @param word       the word to write
 * @param mem_addr   the address in memory to write to
 */
static void
cpu_write_word_in_mem(cpu_state *cpu_state, uint32_t word, uint32_t mem_addr) {
	uint8_t byte = word & 0xff;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, mem_addr, byte);
	byte = (word >> 8) & 0xff;
	mem_addr++;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, mem_addr, byte);
	byte = (word >> 16) & 0xff;
	mem_addr++;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, mem_addr, byte);
	byte = (word >> 24) & 0xff;
	mem_addr++;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, mem_addr, byte);
}

/** @brief Save byte on stack and decrements stack pointer afterwards
 *
 *  @param cpu_state is the cpu instance
 *  @param byte that get pushed on stack
 */
static void cpu_stack_push_byte(cpu_state *cpu_state, uint8_t byte){
	cpu_write_byte_in_mem(cpu_state, byte, cpu_state->esp);
	cpu_state->esp--;
}

/** @brief Save doubleword on stack and decrements stack pointer
 *         afterwards. The most significant byte
 *         get pushed first (on the higher address).
 *
 *  @param cpu_state is the cpu instance
 *  @param doubleword that get pushed on stack
 */
static void cpu_stack_push_doubleword(cpu_state *cpu_state, uint32_t doubleword){
	uint8_t byte;
	byte = (doubleword >> 24) & 0xff;
	cpu_stack_push_byte(cpu_state, byte);

	byte = (doubleword >> 16) & 0xff;
	cpu_stack_push_byte(cpu_state, byte);

	byte = (doubleword >> 8) & 0xff;
	cpu_stack_push_byte(cpu_state, byte);

	byte = doubleword & 0xff;
	cpu_stack_push_byte(cpu_state, byte);
}

/** @brief Read byte from stack and increments stack pointer afterwards
 *
 *  @param cpu_state is the cpu instance
 *
 *  @return byte on stack
 */
static uint8_t cpu_stack_pop_byte(cpu_state *cpu_state){
	uint8_t byte = cpu_peek_byte_from_mem(cpu_state, cpu_state->esp);
	cpu_state->esp++;
	return byte;
}

/** @brief Read doubleword from stack and increments stack pointer afterwards.
 *         Least significant byte is read first.
 *
 *  @param cpu_state is the cpu instance
 *
 *  @return doubleword on stack
 */
static uint32_t cpu_stack_pop_doubleword(cpu_state *cpu_state){
	uint32_t dw;
	uint8_t byte0, byte1, byte2, byte3;

	byte0 = cpu_stack_pop_byte(cpu_state);
	byte1 = cpu_stack_pop_byte(cpu_state);
	byte2 = cpu_stack_pop_byte(cpu_state);
	byte3 = cpu_stack_pop_byte(cpu_state);

	dw  = byte0;
	dw |= (byte1 << 8);
	dw |= (byte2 << 16);
	dw |= (byte3 << 24);

	return dw;
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
	op_code = cpu_read_byte_from_mem(cpu_state);

	op_addr s_op;
	memset(&s_op, 0, sizeof(op_addr));

	switch(op_code) {
		#include "cpu_moveInst.c"
		
		#include "cpu_cmpInst.c"

		#include "cpu_JumpInst.c"

		case 0xFE: {
			/* Increment r/m byte by 1 */
			if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT)){
				uint8_t src;
				if(s_op.regmem_type == MEMORY){
					src = cpu_peek_byte_from_mem(cpu_state, s_op.regmem_mem);
					src++;
					cpu_write_byte_in_mem(cpu_state, src, s_op.regmem_mem);
				} else {
					src = cpu_read_byte_from_reg(s_op.reg, s_op.regmem_type == REGISTER_HIGH);
					src++;
					cpu_write_byte_in_reg(src, s_op.regmem_reg, s_op.regmem_type == REGISTER_HIGH);
				}
			return true;
			}
			break;
		}

		case 0xFF: {
			/* Increment r/m word by 1 */
			if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
				uint32_t src;
				if(s_op.regmem_type == MEMORY){
					src = cpu_peek_word_from_mem(cpu_state, s_op.regmem_mem);
					src++;
					cpu_write_word_in_mem(cpu_state, src, s_op.regmem_mem);
				} else {
					src = cpu_read_word_from_reg(s_op.reg);
					src++;
					cpu_write_word_in_reg(src, s_op.regmem_reg);
				}
			return true;
			}
			break;
		}

		case 0x40: {
			/* Increment doubleword register eax by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->eax));
			src++;
			cpu_write_word_in_reg(src, &(cpu_state->eax));
			return true;
		}
		
		case 0x41: {
			/* Increment doubleword register ecx by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->ecx));
			src++;
			cpu_write_word_in_reg(src, &(cpu_state->ecx));
			return true;
		}

		case 0x42: {
			/* Increment doubleword register edx by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->edx));
			src++;
			cpu_write_word_in_reg(src, &(cpu_state->edx));
			return true;
		}

		case 0x43: {
			/* Increment doubleword register ebx by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->ebx));
			src++;
			cpu_write_word_in_reg(src, &(cpu_state->ebx));
			return true;
		}

		case 0x44: {
			/* Increment doubleword register esp by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->esp));
			src++;
			cpu_write_word_in_reg(src, &(cpu_state->esp));
			return true;
		}

		case 0x45: {
			/* Increment doubleword register ebp by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->ebp));
			src++;
			cpu_write_word_in_reg(src, &(cpu_state->ebp));
			return true;
		}

		case 0x46: {
			/* Increment doubleword register esi by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->esi));
			src++;
			cpu_write_word_in_reg(src, &(cpu_state->esi));
			return true;
		}

		case 0x47: {
			/* Increment doubleword register edi by 1 */
			uint32_t src = cpu_read_word_from_reg(&(cpu_state->edi));
			src++;
			cpu_write_word_in_reg(src, &(cpu_state->edi));
			return true;
		}

		case 0x0f: {
			/* The opcode is two bytes long */
			uint8_t op_code_2 = cpu_read_byte_from_mem(cpu_state);
			switch(op_code_2){
				#include "cpu_extInst.c"
			}
		}

		case 0x80: {
			/* Special case: Specific instruction decoded in Mod/RM byte */
			
			if(!cpu_decode_RM(cpu_state, &s_op, EIGHT_BIT)){
				switch(s_op.reg_value){
					#include "cpu_special0x80.c"
				}
			}
			break;
		}

		case 0x81: {
			/* Special case: Specific instruction decoded in Mod/RM byte */
			
			if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
				switch(s_op.reg_value){
					#include "cpu_special0x81.c"
				}
			}
			break;
		}

		case 0x83: {
			/* Special case: Specific instruction decoded in Mod/RM byte */
			
			if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
				switch(s_op.reg_value){
					#include "cpu_special0x83.c"
				}
			}
			break;
		}

		case 0xC6: {
			/* Special case: Specific instruction decoded in Mod/RM byte */
			
			if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
				switch(s_op.reg_value){
					#include "cpu_special0xC6.c"
				}
			}
			break;
		}

		case 0xC7: {
			/* Special case: Specific instruction decoded in Mod/RM byte */
			
			if(!cpu_decode_RM(cpu_state, &s_op, !EIGHT_BIT)){
				switch(s_op.reg_value){
					#include "cpu_special0xC7.c"
				}
			}
			break;
		}
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
