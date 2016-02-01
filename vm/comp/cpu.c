#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* DEBUG LIB */
#include <stdio.h>

#include "cpu.h"
#include "io_decoder.h"
#include "debug.h"

#ifdef DEBUG
	#include "memory.h"
	#include "bios_rom.h"
#endif

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#define EIGHT_BIT true
#define IMMEDIATE true

#define SUBTRACTION true

#define HIGH_BYTE true

#define IS_HIGH(x) (x ## _type == REGISTER_HIGH)

static cpu_register cpu_modrm_eval_register(cpu_state *cpu_state, cpu_register reg, uint32_t **reg_addr, bool is_8bit);
static bool         cpu_modrm_eval(cpu_state *cpu_state, modsib *mod, uint8_t byte, uint8_t is_8bit);

static bool cpu_decode_RM(cpu_state *cpu_state, op_addr *addr, bool is_8bit);
static void cpu_set_opaddr_regmem(cpu_state *cpu_state, uint8_t mode, uint32_t *addr, op_addr *op);

static uint8_t  cpu_read_byte_from_reg(uint32_t *reg_addr, bool is_high);
static uint32_t cpu_read_doubleword_from_reg(uint32_t *reg_addr);
static uint8_t  cpu_read_byte_from_mem(cpu_state *cpu_state, uint32_t mem_addr, cpu_segment segment);
static uint16_t cpu_read_word_from_mem(cpu_state *cpu_state, uint32_t mem_addr, cpu_segment segment);
static uint32_t cpu_read_doubleword_from_mem(cpu_state *cpu_state, uint32_t mem_addr, cpu_segment segment);

static uint8_t  cpu_consume_byte_from_mem(cpu_state *cpu_state);
static uint32_t cpu_consume_doubleword_from_mem(cpu_state *cpu_state);

static void cpu_write_byte_in_reg(uint32_t *reg_addr, uint8_t byte, bool is_high);
static void cpu_write_doubleword_in_reg(uint32_t *reg_addr, uint32_t word);
static void cpu_write_byte_in_mem(cpu_state *cpu_state, uint8_t byte, uint32_t mem_addr, cpu_segment segment);
static void cpu_write_doubleword_in_mem(cpu_state *cpu_state, uint32_t word, uint32_t mem_addr, cpu_segment segment);

static void cpu_stack_push_byte(cpu_state *cpu_state, uint8_t byte);
static void cpu_stack_push_word(cpu_state *cpu_state, uint16_t word);
static void cpu_stack_push_doubleword(cpu_state *cpu_state, uint32_t doubleword);
static uint8_t cpu_stack_pop_byte(cpu_state *cpu_state);
static uint16_t cpu_stack_pop_word(cpu_state *cpu_state);
static uint32_t cpu_stack_pop_doubleword(cpu_state *cpu_state);

static bool cpu_readb(void *_cpu_state, uint32_t addr, uint8_t *valp);
static bool cpu_writeb(void *_cpu_state, uint32_t addr, uint8_t val);

static void cpu_raise_flag(cpu_state *cpu_state, flag flag);
static void cpu_clear_flag(cpu_state *cpu_state, flag flag);
static void cpu_set_flag(cpu_state *cpu_state, flag flag, bool raised);

static void cpu_set_carry_add(cpu_state *cpu_state, uint32_t first_summand, uint32_t result);
static void cpu_set_carry_sub(cpu_state *cpu_state, uint32_t minuend, uint32_t subtrahend);
static void cpu_set_parity_flag(cpu_state *cpu_state, uint32_t result);

static void cpu_set_overflow_add(cpu_state *cpu_state, uint32_t summand_fst, uint32_t summand_snd, uint32_t result, bool is_8bit);
static void cpu_set_overflow_sub(cpu_state *cpu_state, uint32_t minuend, uint32_t subtrahend, uint32_t result, bool is_8bit);

static void cpu_set_aux_flag_add(cpu_state *cpu_state, uint32_t op1, uint32_t op2, uint32_t result, bool is_8bit);
static void cpu_set_aux_flag_sub(cpu_state *cpu_state, uint32_t minuend, uint32_t subtrahend, bool is_8bit);

static void cpu_set_sign_flag(cpu_state *cpu_state, uint32_t result, bool is_8bit);
static void cpu_set_zero_flag(cpu_state *cpu_state, uint32_t result);
static void cpu_set_eflag_arith(cpu_state *cpu_state, uint32_t op1, uint32_t op2, uint32_t result, bool is_8bit, bool is_subtraction);
static void cpu_set_eip(cpu_state *cpu_state, uint32_t new_addr);

static bool cpu_get_carry_flag(cpu_state *cpu_state);
static bool cpu_get_overflow_flag(cpu_state *cpu_state);
static bool cpu_get_sign_flag(cpu_state *cpu_state);
static bool cpu_get_zero_flag(cpu_state *cpu_state);
static bool cpu_get_interrupt_flag(cpu_state *cpu_state);


static void cpu_handle_interrupt(cpu_state *cpu_state);
static void cpu_handle_gpf(cpu_state *cpu_state);
static void cpu_handle_interrupt_vector(cpu_state *cpu_state, int vector_number);

static void cpu_load_segment_register(cpu_state *cpu_state, cpu_segment segment, uint16_t public);
static void cpu_check_segment_range(cpu_state *cpu_state, cpu_segment segment, uint32_t address);

#ifdef DEBUG_PRINT_INST
static void cpu_print_inst(char *inst);
#endif

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
	cpu_state->saved_state = malloc(sizeof(struct cpu_state));
	assert(cpu_state->saved_state != NULL);
	((struct cpu_state*)(cpu_state->saved_state))->saved_state=NULL;

	cpu_state->port_host = port_host;
	/* Set base pointer to start address of ROM.
	 * The address is hardcoded, like in real hardware.
	 */
	cpu_state->eip = 0xE000;
	cpu_state->esp = 1024*32;

	int i;
	for(i=0; i<6; i++){
		(&(cpu_state->es)+i)->type = DATA_WRITEABLE;
		(&(cpu_state->es)+i)->base_addr = 0;
		(&(cpu_state->es)+i)->limit = 0xFFFF;
	}
	cpu_state->cs.type |= CODE_READABLE;
	cpu_state->cs.base_addr = 0;
	cpu_state->cs.limit = 0xFFFF;

	cpu_state->interrupt_raised = false;

	sig_host_bus_connect(port_host, cpu_state, &hf);
	return cpu_state;
}

void cpu_interrupt(cpu_state *instance){
	instance->interrupt_raised = true;
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
	free(((cpu_state*)_cpu_state)->saved_state);
	free(_cpu_state);
}


/** @brief sets a given flag to high (1)
 *
 *  @param cpu_state the instance of the cpu
 *  @param flag      the flag to raise
 */
static void cpu_raise_flag(cpu_state *cpu_state, flag flag){
	cpu_state->eflags |= (1 << flag);
}

/** @brief sets a given flag to low (0)
 *
 *  @param cpu_state the instance of the cpu
 *  @param flag      the flag to clear
 */
static void cpu_clear_flag(cpu_state *cpu_state, flag flag){
	cpu_state->eflags &= ~(1 << flag);
}

/** @brief sets a given flag to a given value (high or low)
 *
 *  @param cpu_state the instance of the cpu
 *  @param flag      the flag to set
 *  @param raised    whether the flag should be high (true) or low (false)
 */
static void cpu_set_flag(cpu_state *cpu_state, flag flag, bool raised){
	cpu_state->eflags ^= (-raised ^ cpu_state->eflags) & (1 << flag);
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
		cpu_set_aux_flag_sub(cpu_state, op1, op2, is_8bit);
	} else {
		cpu_set_carry_add(cpu_state, op1, result);
		cpu_set_overflow_add(cpu_state, op1, op2, result, is_8bit);
		cpu_set_aux_flag_add(cpu_state, op1, op2, result, is_8bit);
	}
	cpu_set_sign_flag(cpu_state, result, is_8bit);
	cpu_set_zero_flag(cpu_state, result);
}

static void cpu_set_aux_flag_sub(cpu_state *cpu_state, uint32_t minuend, uint32_t subtrahend, bool is_8bit){
	cpu_set_flag(cpu_state, AUX_CARRY_FLAG, ((minuend & 0x07) < (subtrahend & 0x07)));
}

static void cpu_set_aux_flag_add(cpu_state *cpu_state, uint32_t op1, uint32_t op2, uint32_t result, bool is_8bit){
	if(is_8bit){
		bool raised = ((op1 >> 3) & 0x01) ^ ((op2 >> 3) & 0x01) && !((result >> 3) & 0x01);
		cpu_set_flag(cpu_state, AUX_CARRY_FLAG, raised);
	} else {
		bool raised = ((op1 >> 7) & 0x01) ^ ((op2 >> 7) & 0x01) && !((result >> 7) & 0x01);
		cpu_set_flag(cpu_state, AUX_CARRY_FLAG, raised);
	}
	return;
}

/** @brief Set sign bit in EFLAG register
 *
 */
static void cpu_set_sign_flag(cpu_state *cpu_state, uint32_t result, bool is_8bit){
	if(is_8bit){
		cpu_set_flag(cpu_state, SIGN_FLAG, (result & 0x80));
	} else {
		cpu_set_flag(cpu_state, SIGN_FLAG, (result & 0x80000000));
	}
	return;
}

/** @brief Set carry bit in eflag for addition
 *
 *  @param summand_fst the first operand of the additon
 *  @param result  result of addition
 */
static void cpu_set_carry_add(cpu_state *cpu_state, uint32_t summand_fst, uint32_t result){
	cpu_set_flag(cpu_state, CARRY_FLAG, (result < summand_fst));
}

/** @brief Set carry bit in eflag for subtraction
 *
 *  @param minuend the first operand of the subtraction
 *  @param subtrahend the second operand of the subtraction
 */
static void cpu_set_carry_sub(cpu_state *cpu_state, uint32_t minuend, uint32_t subtrahend){
	cpu_set_flag(cpu_state, CARRY_FLAG, (minuend < subtrahend));
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
		bool raised = (!((summand_fst >> 7) ^ (summand_snd >> 7))) && ((summand_fst >> 7) ^ (result >> 7));
		cpu_set_flag(cpu_state, OVERFLOW_FLAG, raised);
	} else {
		bool raised = (!((summand_fst >> 31) ^ (summand_snd >> 31))) && ((summand_fst >> 31) ^ (result >> 31));
		cpu_set_flag(cpu_state, OVERFLOW_FLAG, raised);
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
		bool raised = (((minuend >> 7) ^ (subtrahend >> 7)) && ((minuend >> 7) ^ (result >> 7)));
		cpu_set_flag(cpu_state, OVERFLOW_FLAG, raised);
	} else {
		bool raised = (((minuend >> 31) ^ (subtrahend >> 31)) && ((minuend >> 31) ^ (result >> 31)));
		cpu_set_flag(cpu_state, OVERFLOW_FLAG, raised);
	}
}

/** @brief Set zero bit in eflag
 *
 *  @param cpu_state the cpu instance
 *  @param result the result of the operation
 */
static void cpu_set_zero_flag(cpu_state *cpu_state, uint32_t result){
	cpu_set_flag(cpu_state, ZERO_FLAG, result==0);
}

/** @brief Set parity bit in eflag depending on result
 *
 *  @param cpu_state the cpu instance
 *  @param result the result of the operation
 */
static void cpu_set_parity_flag(cpu_state *cpu_state, uint32_t result){
	//x86 only looks at the least significant bit
	bool raised =   ((result & 0x01) != 0) ^ ((result & 0x10) != 0) ^
					((result & 0x02) != 0) ^ ((result & 0x20) != 0) ^
					((result & 0x04) != 0) ^ ((result & 0x40) != 0) ^
					((result & 0x08) != 0) ^ ((result & 0x80) != 0);

	cpu_set_flag(cpu_state, PARITY_FLAG, raised);
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


/** @brief returns interrupt flag in EFLAG register
 *
 *  @return true when interrupt flag is set
 *          false else
 */
static bool cpu_get_interrupt_flag(cpu_state *cpu_state){
	return cpu_state->eflags & (1 << INTERRUPT_FLAG);
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

	if(!is_8bit){
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
	uint32_t *mod_reg;
	uint32_t *mod_rm;

	mod->addr_or_scale_mode = (byte >> 6) & (0x7);

	/* Decode register */
	mod->mod_reg_name = cpu_modrm_eval_register(cpu_state, (byte >> 3) & (0x7), &mod_reg, is_8bit);
	if(unlikely(-1 == mod->mod_reg_name)) {
		return false;
	}


	if(mod->addr_or_scale_mode == REGISTER){
		mod->mod_rm_name = cpu_modrm_eval_register(cpu_state, byte & 0x7, &mod_rm, is_8bit);
	} else{
		mod->mod_rm_name = cpu_modrm_eval_register(cpu_state, byte & 0x7, &mod_rm, !EIGHT_BIT);
	}

	/* Decode register/memory */
	if(unlikely(-1 == mod->mod_rm_name)) {
		return false;
	}

	mod->mod_reg = mod_reg;
	mod->mod_rm = mod_rm;

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
	mod_rm = cpu_consume_byte_from_mem(cpu_state);
	modsib s_modrm;
	memset(&s_modrm, 0, sizeof(modsib));
	// memset(addr, 0, sizeof(op_addr));

	if(unlikely(false == cpu_modrm_eval(cpu_state, &s_modrm, mod_rm, is_8bit))){
		return false;
	}

	addr->reg_value = s_modrm.mod_reg_name & 0x07;

	/*
	 * set reg part, this is straightforward
	 */
	if(!is_8bit){//TODO: Alle is_8bit dinge auf unlikely stellen?
		addr->reg_type = REGISTER_WORD;
	}else{
		if(s_modrm.mod_reg_name < AH){
			addr->reg_type = REGISTER_LOW;
		} else {
			addr->reg_type = REGISTER_HIGH;
		}
	}
	addr->reg = s_modrm.mod_reg;

	/*
	 * set regmem part, this the tricky part
	 */
	if(s_modrm.addr_or_scale_mode == REGISTER){
		if(!is_8bit){
			addr->reg_type = REGISTER_WORD;
		}else{
			if(s_modrm.mod_rm_name < AH){
				addr->regmem_type = REGISTER_LOW;
			} else {
				addr->regmem_type = REGISTER_HIGH;
			}
		}

		addr->regmem_reg = s_modrm.mod_rm;
	}else{
		addr->regmem_type = MEMORY;

		if(s_modrm.mod_rm_name == ESP){
			/* We have a SIB byte following */
			uint8_t base;
			uint8_t index;

			uint8_t sib = 0;
			uint8_t scale = 0;

			/* Read next byte increment eip */
			sib = cpu_consume_byte_from_mem(cpu_state);

			scale = 1 << (sib >> 6);
			base = sib & 0x07;
			index = (sib>>3) & 0x07;

			/* Compute base address for mod_rm */
			uint32_t base_mod_rm;
			if(index == ESP) {//see lecture slides 80x86 ass 14-15
				/* Index is not used */
				base_mod_rm = *(&(cpu_state->eax) + base);

			} else if (base == EBP){ //see lecture slides 80x86 ass 14-15
				/* Base is not used */
				base_mod_rm= (*(&(cpu_state->eax) + index)) * scale;

			} else {
				base_mod_rm = *(&(cpu_state->eax) + base) + ((*(&(cpu_state->eax) + index)) * scale);
			}
			/* Remember: addressing mode is unequal to REGISTER! */
			cpu_set_opaddr_regmem(cpu_state, s_modrm.addr_or_scale_mode, &base_mod_rm, addr);

		} else if(s_modrm.mod_rm_name == EBP && s_modrm.addr_or_scale_mode == NO_DISPLACEMENT){
			/* Special case: Reject mod_rm and read in a 32 Bit displacement instead. */
			uint32_t base_0 = 0;
			cpu_set_opaddr_regmem(cpu_state, DISPLACEMENT_32, &base_0, addr);
		} else {
			/* Compute address of mod_rm */
			cpu_set_opaddr_regmem(cpu_state, s_modrm.addr_or_scale_mode, s_modrm.mod_rm, addr);
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
	int8_t displ1;
	int32_t displacement_complete;
	switch(mode){
		case NO_DISPLACEMENT:
			/* Indirection with no displacement */
			op->regmem_mem = *base_addr;
			return;

		case DISPLACEMENT_8:
			/* Read one extra byte from bus */
			displ1 = cpu_consume_byte_from_mem(cpu_state);
			/* Indirection with 8 bit displacement */
			op->regmem_mem = displ1 + (*base_addr);
			return;

		case DISPLACEMENT_32:
			/* Indirection with 32 bit displacement */

			/* Attention: Lowest byte will be read first */
			displacement_complete = cpu_consume_doubleword_from_mem(cpu_state);
			op->regmem_mem = displacement_complete + (*base_addr);
			return;
		case REGISTER:
			op->regmem_reg = base_addr;
			return;
	}
	return;
}

static void cpu_load_segment_register(cpu_state *cpu_state, cpu_segment segment, uint16_t gdtr_offset){
	uint32_t entry_lower = cpu_read_doubleword_from_mem(cpu_state, cpu_state->gdtr_base + gdtr_offset, NOCHECK);
	uint32_t entry_upper = cpu_read_doubleword_from_mem(cpu_state, cpu_state->gdtr_base + gdtr_offset + 4, NOCHECK);

	uint32_t segment_base = (entry_lower >> 16) | (entry_upper & 0xFF) << 16 | (entry_upper & 0xFF000000);
	uint32_t segment_limit = (entry_lower & 0xFFFF) | (entry_upper & 0x0F0000);

	bool s_flag      = entry_upper & (0x01 << 12); //1=code/data 0 = system
	bool present     = entry_upper & (0x01 << 15);
	bool l_flag      = entry_upper & (0x01 << 21); //0=32 bit 1=64bit
	bool db_flag     = entry_upper & (0x01 << 22);
	bool granularity = entry_upper & (0x01 << 23);
	uint8_t dpl      = (entry_upper >> 13) & 0x03;
	uint8_t type     = (entry_upper >>  8) & 0x0F;


	segment_register *cur_register = &(cpu_state->es)+segment;

	if(segment == CODE && !(type & DATACODE)){     // trying to load a data segment into cs
		cpu_handle_gpf(cpu_state);
		//this should never be reached!
		assert(false);
	}

	if(!present ||                                       //segment not present
	    !s_flag ||                                       //system stuff
	    dpl != 0 ||                                      //privilege level != 0
	    l_flag != 0 ||                                   //64/32bit
	    db_flag != 1 ||                                  //16/32bit
	    (type & DATACODE && type & CODE_CONFORMING) ||   //code and conforming
	    (!(type & DATACODE) && type & DATA_EXPANDDOWN))  //data and expanding down)
    {
		cpu_handle_gpf(cpu_state);
		//this should never be reached!
		assert(false);
	}

	cur_register->public_part = gdtr_offset;
	cur_register->base_addr = segment_base;
	cur_register->limit = granularity ? segment_limit*4*1024 : segment_limit;
	cur_register->type = type;
}

static void cpu_check_segment_range(cpu_state *cpu_state, cpu_segment segment, uint32_t address){
	if(segment == NOCHECK)
		return;

	uint32_t segment_limit = (&(cpu_state->es)+segment)->limit;

	if(unlikely(address > segment_limit)){
		cpu_handle_gpf(cpu_state);
		//this should never be reached!
		assert(false);
	}
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
cpu_read_doubleword_from_reg(uint32_t *reg_addr) {
	return *reg_addr;
}

/** @brief read byte at the instruction pointer's address from memory and increment IP
 *
 * @param cpu_state CPU instance
 *
 * @return the byte read
 */
static uint8_t
cpu_consume_byte_from_mem(cpu_state *cpu_state) {
	cpu_check_segment_range(cpu_state, CODE, cpu_state->eip);

	uint8_t next_byte = sig_host_bus_readb(cpu_state->port_host, (void *)cpu_state, cpu_state->cs.base_addr + cpu_state->eip);
	cpu_state->eip = cpu_state->eip + 1;

	return next_byte;
}

/** @brief read a word (2 byte) at the instruction pointer's address from memory
 *         and increment IP by 4
 *
 * @param cpu_state CPU instance
 *
 * @return the word read
 */
static uint16_t
cpu_consume_word_from_mem(cpu_state *cpu_state) {
	cpu_check_segment_range(cpu_state, CODE, cpu_state->eip);

	uint8_t displ1, displ2;
	uint16_t displacement_complete;

	displ1 = cpu_consume_byte_from_mem(cpu_state);
	displ2 = cpu_consume_byte_from_mem(cpu_state);

	displacement_complete = displ1;
	displacement_complete |= (displ2 << 8);

	return displacement_complete;
}

/** @brief read a double word (4 byte) at the instruction pointer's address from memory
 *         and increment IP by 4
 *
 * @param cpu_state CPU instance
 *
 * @return the word read
 */
static uint32_t
cpu_consume_doubleword_from_mem(cpu_state *cpu_state) {
	cpu_check_segment_range(cpu_state, CODE, cpu_state->eip);

	uint8_t displ1, displ2, displ3, displ4;
	uint32_t displacement_complete;

	displ1 = cpu_consume_byte_from_mem(cpu_state);
	displ2 = cpu_consume_byte_from_mem(cpu_state);
	displ3 = cpu_consume_byte_from_mem(cpu_state);
	displ4 = cpu_consume_byte_from_mem(cpu_state);

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
cpu_read_byte_from_mem(cpu_state *cpu_state, uint32_t mem_addr, cpu_segment segment) {
	cpu_check_segment_range(cpu_state, segment, mem_addr);

	uint32_t segment_base;
	if(segment != NOCHECK)
		segment_base = (&(cpu_state->es)+segment)->base_addr;
	else
		segment_base = 0;

	return sig_host_bus_readb(cpu_state->port_host, cpu_state, segment_base + mem_addr);
}

/** @brief read a word (2 byte) at given address from memory. keeps IP untouched.
 *
 * @param cpu_state CPU instance
 * @param mem_addr  the address to read at
 *
 * @return the word read
 */
static uint16_t
cpu_read_word_from_mem(cpu_state *cpu_state, uint32_t mem_addr, cpu_segment segment) {
	cpu_check_segment_range(cpu_state, segment, mem_addr);

	uint32_t segment_base;
	if(segment != NOCHECK)
		segment_base = (&(cpu_state->es)+segment)->base_addr;
	else
		segment_base = 0;

	uint16_t data = 0;
	uint8_t byte1, byte2;
	byte1 = sig_host_bus_readb(cpu_state->port_host, cpu_state, segment_base + mem_addr);
	mem_addr++;

	byte2 = sig_host_bus_readb(cpu_state->port_host, cpu_state, segment_base + mem_addr);
	mem_addr++;

	data = byte1;
	data |= (byte2 << 8);

	return data;
}

/** @brief read a double word (4 byte) at given address from memory. keeps IP untouched.
 *
 * @param cpu_state CPU instance
 * @param mem_addr  the address to read at
 *
 * @return the double word read
 */
static uint32_t
cpu_read_doubleword_from_mem(cpu_state *cpu_state, uint32_t mem_addr, cpu_segment segment) {
	cpu_check_segment_range(cpu_state, segment, mem_addr);

	uint32_t segment_base;
	if(segment != NOCHECK)
		segment_base = (&(cpu_state->es)+segment)->base_addr;
	else
		segment_base = 0;

	uint32_t data = 0;
	uint8_t byte1, byte2, byte3, byte4;
	byte1 = sig_host_bus_readb(cpu_state->port_host, cpu_state, segment_base + mem_addr);
	mem_addr++;

	byte2 = sig_host_bus_readb(cpu_state->port_host, cpu_state, segment_base + mem_addr);
	mem_addr++;

	byte3 = sig_host_bus_readb(cpu_state->port_host, cpu_state, segment_base + mem_addr);
	mem_addr++;

	byte4 = sig_host_bus_readb(cpu_state->port_host, cpu_state, segment_base + mem_addr);

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
 * @param reg_addr  the address of the register to read
 * @param byte      the byte to write
 * @param is_high   whether to write the high byte
 */
static void
cpu_write_byte_in_reg(uint32_t *reg_addr, uint8_t byte, bool is_high) {
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
 * @param word      the word to write
 * @param reg_addr  the address of the register to read
 */
static void
cpu_write_doubleword_in_reg(uint32_t *reg_addr, uint32_t word) {
	*reg_addr = word;
}

/** @brief write a byte to the memory
 *
 * @param cpu_state  the cpu instance
 * @param byte       the byte to write
 * @param mem_addr   the address in the mem to write to
 */
static void
cpu_write_byte_in_mem(cpu_state *cpu_state, uint8_t byte, uint32_t mem_addr, cpu_segment segment) {
	cpu_check_segment_range(cpu_state, segment, mem_addr);

	uint32_t segment_base;
	if(segment != NOCHECK)
		segment_base = (&(cpu_state->es)+segment)->base_addr;
	else
		segment_base = 0;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, segment_base + mem_addr, byte);
}

/** @brief write a word (4 byte) to the memory
 *
 * @param cpu_state  the cpu instance
 * @param word       the word to write
 * @param mem_addr   the address in memory to write to
 */
static void
cpu_write_doubleword_in_mem(cpu_state *cpu_state, uint32_t word, uint32_t mem_addr, cpu_segment segment) {
	cpu_check_segment_range(cpu_state, segment, mem_addr);
	uint8_t byte = word & 0xff;

	uint32_t segment_base;
	if(segment != NOCHECK)
		segment_base = (&(cpu_state->es)+segment)->base_addr;
	else
		segment_base = 0;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, segment_base + mem_addr, byte);
	byte = (word >> 8) & 0xff;
	mem_addr++;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, segment_base + mem_addr, byte);
	byte = (word >> 16) & 0xff;
	mem_addr++;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, segment_base + mem_addr, byte);
	byte = (word >> 24) & 0xff;
	mem_addr++;

	sig_host_bus_writeb(cpu_state->port_host, cpu_state, segment_base + mem_addr, byte);
}

/** @brief Save byte on stack and decrements stack pointer afterwards
 *
 *  @param cpu_state is the cpu instance
 *  @param byte that get pushed on stack
 */
static void cpu_stack_push_byte(cpu_state *cpu_state, uint8_t byte){
	/* Decrement first to assert that esp points
	 * on the last pushed byte
	 */
	cpu_state->esp--;
	cpu_write_byte_in_mem(cpu_state, byte, cpu_state->esp, STACK);
}

/** @brief Save doubleword on stack and decrements stack pointer
 *         afterwards. The most significant byte
 *         get pushed first (on the higher address).
 *
 *  @param cpu_state is the cpu instance
 *  @param doubleword that get pushed on stack
 */
static void cpu_stack_push_doubleword(cpu_state *cpu_state, uint32_t doubleword){
	uint8_t byte = 0;
	byte = (doubleword >> 24) & 0xff;
	cpu_stack_push_byte(cpu_state, byte);

	byte = (doubleword >> 16) & 0xff;
	cpu_stack_push_byte(cpu_state, byte);

	byte = (doubleword >> 8) & 0xff;
	cpu_stack_push_byte(cpu_state, byte);

	byte = doubleword & 0xff;
	cpu_stack_push_byte(cpu_state, byte);
}

/** @brief Save a word on stack and decrements stack pointer
 *         afterwards. The most significant byte
 *         get pushed first (on the higher address).
 *
 *  @param cpu_state is the cpu instance
 *  @param word that get pushed on stack
 */
static void cpu_stack_push_word(cpu_state *cpu_state, uint16_t word){
	uint8_t byte = 0;
	byte = (word >> 8) & 0xff;
	cpu_stack_push_byte(cpu_state, byte);

	byte = word & 0xff;
	cpu_stack_push_byte(cpu_state, byte);
}

/** @brief Read byte from stack and increments stack pointer afterwards
 *
 *  @param cpu_state is the cpu instance
 *
 *  @return byte on stack
 */
static uint8_t cpu_stack_pop_byte(cpu_state *cpu_state){
	uint8_t byte = cpu_read_byte_from_mem(cpu_state, cpu_state->esp, STACK);
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
	uint32_t dw = 0;
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

/** @brief Read word from stack and increments stack pointer afterwards.
 *         Least significant byte is read first.
 *
 *  @param cpu_state is the cpu instance
 *
 *  @return word on stack
 */
static uint16_t cpu_stack_pop_word(cpu_state *cpu_state){
	uint16_t dw = 0;
	uint8_t byte0, byte1;

	byte0 = cpu_stack_pop_byte(cpu_state);
	byte1 = cpu_stack_pop_byte(cpu_state);

	dw  = byte0;
	dw |= (byte1 << 8);

	return dw;
}

/** @brief Save state of cpu on stack. This procedure is called bv an
 *         interrupt handling routine.
 */
static void cpu_save_state(cpu_state *cpu_state){
	cpu_stack_push_doubleword(cpu_state, cpu_state->eflags);
	cpu_stack_push_word(cpu_state, cpu_state->cs.public_part);
	cpu_stack_push_doubleword(cpu_state, cpu_state->eip);
}

/** @brief Save state of cpu on stack. This procedure is called by IRET
 */
static void cpu_restore_state(cpu_state *cpu_state){
	cpu_state->eip = cpu_stack_pop_doubleword(cpu_state);
	cpu_load_segment_register(cpu_state, CODE, cpu_stack_pop_word(cpu_state));
	cpu_state->eflags = cpu_stack_pop_doubleword(cpu_state);
}

static void cpu_handle_interrupt(cpu_state * cpu_state){
	uint8_t index = sig_host_bus_read_io_dev(cpu_state->port_host,
		cpu_state, PIC_BEGIN);

	if(index * VECTOR_SIZE > cpu_state->idtr_limit){
		fprintf(stderr, "Array index out of bounds exception, ignoring interrupt");
		return;
	}

	cpu_handle_interrupt_vector(cpu_state, index);
}

/** @brief handles a general protection fault */
static void cpu_handle_gpf(cpu_state *cpu_state){
	memcpy(cpu_state, cpu_state->saved_state, sizeof(*cpu_state));

	#ifdef DEBUG
		fprintf(stderr, "General Protection fault at address %#08x\n", cpu_state->eip);
	#endif

	cpu_handle_interrupt_vector(cpu_state, 0x0D);

	longjmp(cpu_state->jump_target_new_instruction, 1);
}

static void cpu_handle_interrupt_vector(cpu_state *cpu_state, int vector_number){
	uint32_t idtr_vector_base = (cpu_state->idtr_base) + vector_number * VECTOR_SIZE;

	uint32_t segment = cpu_read_word_from_mem(cpu_state, idtr_vector_base+2, NOCHECK);
	uint32_t offset = cpu_read_word_from_mem(cpu_state, idtr_vector_base, NOCHECK) |
						cpu_read_word_from_mem(cpu_state, idtr_vector_base+6, NOCHECK) << 16;

	//save state, jump to calculated address
	cpu_save_state(cpu_state);

	cpu_state->interrupt_raised = false;
	cpu_clear_flag(cpu_state, INTERRUPT_FLAG);

	cpu_load_segment_register(cpu_state, CODE, segment);
	cpu_state->eip = offset;
}

#ifdef DEBUG_PRINT_INST
static void
cpu_print_inst(char *inst){
	fprintf(stderr, "%s", inst);
	fflush(stderr);
	return;
}
#endif

bool
cpu_step(void *_cpu_state) {

	/* cast */
	cpu_state *cpu_state = (struct cpu_state *) _cpu_state;
	#ifdef DEBUG
	ram_state *_ram_state = (ram_state *)(cpu_state->port_host->members[0].s);
	rom_state *_rom_state = (rom_state *)(cpu_state->port_host->members[2].s);
	#endif

	if(setjmp(cpu_state->jump_target_new_instruction)){
		/* if a general exception fault was handled, we jump here and return */
		return true;
	}

	/* save the current cpu_state in case a general protection fault has to be handled*/
	memcpy(cpu_state->saved_state, cpu_state, sizeof(*cpu_state));

	if(cpu_state->interrupt_raised && cpu_get_interrupt_flag(cpu_state)){
		cpu_handle_interrupt(cpu_state);
	}

	uint8_t op_code = 0;
	bool cond;

	/* read the first byte from instruction pointer and increment ip
	 * afterards */
	op_code = cpu_consume_byte_from_mem(cpu_state);

	op_addr s_op;
	memset(&s_op, 0, sizeof(op_addr));

	#ifdef DEBUG_PRINT_INST_ADDR
		#ifdef DEBUG_PRINT_INST
			fprintf(stderr, "Instruction (%#08x + %#08x): %#x   ", cpu_state->cs.base_addr,  cpu_state->eip, op_code);
		#else
			fprintf(stderr, "Instruction (%#08x + %#08x): %#x\n", cpu_state->cs.base_addr, cpu_state->eip, op_code);
			fflush(stderr);
		#endif
	#endif

	switch(op_code) {
		case 0xfa:{
			/*CLI*/
			#ifdef DEBUG_PRINT_INST
			cpu_print_inst("CLI\n");
			#endif
			cpu_clear_flag(cpu_state, INTERRUPT_FLAG);

			return true;
		}
		case 0xfb:{
			/*STI*/
			#ifdef DEBUG_PRINT_INST
			cpu_print_inst("STI\n");
			#endif
			cpu_raise_flag(cpu_state, INTERRUPT_FLAG);
			return true;
		}

		#include "instructionBlocks/cpu_andInst.c"

		#include "instructionBlocks/cpu_addInst.c"

		#include "instructionBlocks/cpu_compareInst.c"

		#include "instructionBlocks/cpu_jumpInst.c"

		#include "instructionBlocks/cpu_moveInst.c"

		#include "instructionBlocks/cpu_specialInst.c"

		#include "instructionBlocks/cpu_stackInst.c"

		#include "instructionBlocks/cpu_xorInst.c"

		#include "instructionBlocks/cpu_incInst.c"

		#include "instructionBlocks/cpu_intInst.c"

		#include "instructionBlocks/cpu_decInst.c"

		#include "instructionBlocks/cpu_hltInst.c"

		#include "instructionBlocks/cpu_ioInst.c"

		#include "instructionBlocks/cpu_pushInst.c"

		#include "instructionBlocks/cpu_popInst.c"

		default:
			break;
	}

	#ifdef DEBUG_PRINT_INST
		fprintf(stderr, "Unknown op-code: 0x%02x\n", op_code);
	#endif
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
