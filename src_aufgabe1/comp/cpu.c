#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "cpu.h"


/* Method declarations */
static void computeOp2Address(cpssp_cpu *, uint8_t, uint32_t *, op_addr *);
static uint8_t cpu_get_byte_inc(cpssp_cpu *);

/* @brief Write back address of CPU own register
 * based on bit pattern.
 *
 * @param cpssp_cpu  CPU instance
 *
 * @param reg  Bit pattern for evaluation of register
 *
 * @param reg_addr  Pointer that holds the register
 *     address when bit pattern was valid
 *
 * @return True on success.
 *         False if pattern is greater than 7
 */
static uint8_t
cpu_evalRegister(cpssp_cpu *cpssp_cpu, uint8_t reg, uint32_t **reg_addr, bool is_8bit){
	/* FIXME Is it really necessary to return register name? */
	switch(reg){
		case EAX:
			*reg_addr = &(cpssp_cpu->eax);
			if(is_8bit){
					return AL;
			} else {
					return EAX;
			}
		case EBX:
			*reg_addr = &(cpssp_cpu->ebx);
			if(is_8bit){
					return BL;
			} else {
					return EBX;
			}
		case ECX:
			*reg_addr = &(cpssp_cpu->ecx);
			if(is_8bit){
					return CL;
			} else {
					return ECX;
			}
		case EDX:
			*reg_addr = &(cpssp_cpu->edx);
			if(is_8bit){
					return DL;
			} else {
					return EDX;
			}
		case ESP:
			if(is_8bit){
					*reg_addr = &(cpssp_cpu->eax);
					return AH;
			} else {
					*reg_addr = &(cpssp_cpu->esp);
					return ESP;
			}
		case EBP:
			if(is_8bit){
					*reg_addr = &(cpssp_cpu->ecx);
					return CH;
			} else {
					*reg_addr = &(cpssp_cpu->ebp);
					return EBP;
			}
		case ESI:
			if(is_8bit){
					*reg_addr = &(cpssp_cpu->edx);
					return DH;
			} else {
					*reg_addr = &(cpssp_cpu->esi);
					return ESI;
			}
		case EDI:
			if(is_8bit){
					*reg_addr = &(cpssp_cpu->ebx);
					return BH;
			} else {
					*reg_addr = &(cpssp_cpu->edi);
					return EDI;
			}
		default:
			return -1;
	}
}

/*
 * @brief  Interpret byte by it different parts
 * Bits 7..6  Scale or Addressing Mode
 * Bits 5..3  Register of first operand
 * Bits 2..0  Register of second operand
 *
 * @param cpssp_cpu    CPU instance
 * @param mod      Pointer to structure where results will be saved in
 * @param byte     byte that will be analysed
 * @param is_8bit  Is it an 8 bit instruction
 *
 * @return         True on success.
 *                 False in case of a huge programming mistake.
 */
static bool
cpu_eval_byte(cpssp_cpu *cpssp_cpu, modsib *mod, uint8_t byte, uint8_t is_8bit)
{
	uint32_t *op1;
	uint32_t *op2;

	mod->op1_name = cpu_evalRegister(cpssp_cpu, byte & (0x7), &op1, is_8bit);
	if( -1 == mod->op1_name) {
		return false;
	}

	mod->op2_name = cpu_evalRegister(cpssp_cpu, (byte >> 3) & 0x7, &op2, is_8bit);
	if( -1 == mod->op2_name) {
		return false;
	}

	mod->addr_or_scale_mode = (byte >> 6) & (0x7);
	mod->op1 = op1;
	mod->op2 = op2;

	return true;
}

/*
 * @brief This method reads and decodes the MOD_RM, SIB and
 * DISPLACEMENT bytes. Furthermore it computes the addresses of
 * operand 1 and 2. Either as pointer to a register or
 * a virtual address for the RAM component
 *
 * @param cpssp_cpu    CPU instance
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
cpu_decode_RM(cpssp_cpu *cpssp_cpu, op_addr *addr, bool is_8bit, bool has_imm)
{
	uint8_t mod_rm = 0;

	/* Eval MOD_RM Byte */
	mod_rm = cpu_get_byte_inc(cpssp_cpu);
	modsib s_modrm;
	memset(&mod_rm, 0, sizeof(modsib));

	if(false == cpu_eval_byte(cpssp_cpu, &s_modrm, mod_rm, is_8bit)){
		return false;
	}

	/* TODO: interpret reg bits in case of immidiate */

	/* Check if SIB byte follows */
	if(s_modrm.op2_name == ESP && s_modrm.addr_or_scale_mode != REGISTER){
		/* Define variables for SIB byte */
		uint32_t *base;
		uint32_t *index;

		uint8_t sib = 0;
		uint8_t scale = 0;


		/* Read next byte increment eip */
		sib = cpu_get_byte_inc(cpssp_cpu);
		modsib s_sib;
		memset(&s_sib,0,sizeof(modsib));

		if(false == cpu_eval_byte(cpssp_cpu, &s_sib, sib, is_8bit)){
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
		computeOp2Address(cpssp_cpu, s_modrm.addr_or_scale_mode, &base_op2, addr);

	} else if(s_modrm.op2_name == EBP && s_modrm.addr_or_scale_mode == NO_DISPLACEMENT){
		/* Special case: Reject op2 and read in a 32 Bit displacement instead. */
		uint32_t base_0 = 0;
		computeOp2Address(cpssp_cpu, DISPLACEMENT_32, &base_0, addr);
	} else {
		/* Compute address of op2 */
		computeOp2Address(cpssp_cpu, s_modrm.addr_or_scale_mode, s_modrm.op2, addr);
	}

	/* Set op1 to reg address or as a constant value */
	if(has_imm && is_8bit){
			addr->op1_const = cpu_get_byte_inc(cpssp_cpu);
	} else if(has_imm){
			addr->op1_const = cpu_read_32_bit_addr(cpssp_cpu);
	} else{
			addr->op1_reg = s_modrm.op1;
	}
	return true;
}



static uint32_t
cpu_read_32_bit_addr(cpssp_cpu *cpssp_cpu){
		uint8_t displ1, displ2, displ3, displ4;
		uint32_t displacement_complete;

		displ1 = cpu_get_byte_inc(cpssp_cpu);
		displ2 = cpu_get_byte_inc(cpssp_cpu);
		displ3 = cpu_get_byte_inc(cpssp_cpu);
		displ4 = cpu_get_byte_inc(cpssp_cpu);

		displacement_complete = displ1;
		displacement_complete |= (displ2 << 8);
		displacement_complete |= (displ3 << 16);
		displacement_complete |= (displ4 << 24);

		return displacement_complete;
}


static uint8_t
cpu_get_byte_inc(cpssp_cpu *cpssp_cpu)
{
	uint8_t next_byte = sig_host_bus_readb(cpssp_cpu->port_host, (void *)cpssp_cpu, cpssp_cpu->eip);
	cpssp_cpu->eip = cpssp_cpu->eip + 1;

	return next_byte;
}


/*
 * @brief          Compute address of operand 2 subjected to the addressing mode
 *
 * @param cpssp_cpu    CPU instance
 *
 * @param mode     Contains addressing mode
 *
 * @param addr     Pointer that contains the base address. Usually
 *                 a pointer to CPU own register.
 */
static void
computeOp2Address(cpssp_cpu *cpssp_cpu, uint8_t mode, uint32_t *addr, op_addr *op){
	uint8_t displ1;
	uint32_t displacement_complete;
	switch(mode){
		case NO_DISPLACEMENT:
			/* Indirection with no displacement */
			op->op2_mem = *addr;
			return;

		case DISPLACEMENT_8:
			/* Read one extra byte from bus */
			displ1 = cpu_get_byte_inc(cpssp_cpu);
			/* Indirection with 8 bit displacement */
			op->op2_mem = displ1 + (*addr);
			return;

		case DISPLACEMENT_32:
			/* Indirection with 32 bit displacement */

			/* Attention: Lowest byte will be read first */
			displacement_complete = cpu_read_32_bit_addr(cpssp_cpu);
			op->op2_mem = displacement_complete + (*addr);
			return;
		case REGISTER:
			op->op2_reg = addr;
			return;
	}
	return;
}

static uint8_t
cpu_read_byte_from_mem(cpssp_cpu *cpssp_cpu, uint32_t ram_addr)
{
	return sig_host_bus_readb(cpssp_cpu->port_host, cpssp_cpu, ram_addr);	
}

static uint32_t
cpu_read_data_from_mem(cpssp_cpu *cpssp_cpu, uint32_t ram_addr)
{

	uint32_t data;
		uint8_t byte1, byte2, byte3, byte4;
		byte1 = sig_host_bus_readb(cpssp_cpu->port_host, cpssp_cpu, ram_addr);
		ram_addr++;
		
		byte2 = sig_host_bus_readb(cpssp_cpu->port_host, cpssp_cpu, ram_addr);
		ram_addr++;
		
		byte3 = sig_host_bus_readb(cpssp_cpu->port_host, cpssp_cpu, ram_addr);
		ram_addr++;
		
		byte4 = sig_host_bus_readb(cpssp_cpu->port_host, cpssp_cpu, ram_addr);
		
		data = byte1;
		data |= (byte2 << 8);
		data |= (byte3 << 16);
		data |= (byte4 << 24);
		
		return data;
}

static uint32_t
cpu_read_byte_from_register(bool is_high, uint32_t *reg_addr)
{
	if(is_high) {
		return (*reg_addr >> 8) & 0xff;
	}
	
	return *reg_addr & 0xff;
} 
static void
cpu_write_byte_in_mem(cpssp_cpu *cpssp_cpu, uint8_t byte, uint32_t ram_addr)
{
	sig_host_bus_writeb(cpssp_cpu->port_host, cpssp_cpu, ram_addr, byte);	
}

static void
cpu_write_data_in_mem(cpssp_cpu *cpssp_cpu, uint32_t data, uint32_t ram_addr)
{
	uint8_t byte = data & 0xff; 

	sig_host_bus_writeb(cpssp_cpu->port_host, cpssp_cpu, ram_addr, byte);	
	byte = (data >> 8) & 0xff;
	ram_addr++;

	sig_host_bus_writeb(cpssp_cpu->port_host, cpssp_cpu, ram_addr, byte);	
	byte = (data >> 16) & 0xff;
	ram_addr++;
	
	sig_host_bus_writeb(cpssp_cpu->port_host, cpssp_cpu, ram_addr, byte);	
	byte = (data >> 24) & 0xff;
	ram_addr++;
	
	sig_host_bus_writeb(cpssp_cpu->port_host, cpssp_cpu, ram_addr, byte);	
}


static void
cpu_write_byte_in_reg(uint8_t byte, uint32_t *reg_addr, bool is_high)
{
	if(!is_high) {
		*reg_addr &= ~0xff;
		*reg_addr |= byte;
	} else {
		*reg_addr &= ~0xff00;
		*reg_addr |= (byte << 8);
		
	}
}

static void
cpu_write_data_in_reg(uint32_t data, uint32_t *reg_addr)
{	
	*reg_addr = data;
}

bool
cpu_step(void *_cpssp_cpu)
{
	/* cast */
	cpssp_cpu *cpssp_cpu = (struct cpssp_cpu *) _cpssp_cpu;

	uint8_t op_code;

	/* read the first byte from instruction pointer and increment ip
	 * afterards */
	op_code = cpu_get_byte_inc(cpssp_cpu);
	
	op_addr s_op;
	memset(&s_op, 0, sizeof(op_addr));

	switch(op_code) {
			case 0x88:
				/* MOV r8 to r/m8 */
				if(!cpu_decode_RM(cpssp_cpu, &s_op, true, false)){
					uint8_t src = cpu_read_byte_from_register(s_op.is_op1_high, s_op.op1_reg);
					if(s_op.op2_reg != 0){
						/* Write in a register */
						cpu_write_byte_in_reg(src, s_op.op2_reg, s_op.is_op2_high);
					} else {
						/* Write in memory */
						cpu_write_byte_in_mem(cpssp_cpu, src, s_op.op2_mem);
					}
					return true;
				}
				break;
			case 0x89:
				/* Mov r32 to r/m32 */
				if(!cpu_decode_RM(cpssp_cpu, &s_op, true, false)){
					uint32_t src = *(s_op.op2_reg);
					if(s_op.op2_reg != 0){
						/* Write in a register */
						cpu_write_data_in_reg(src, s_op.op1_reg);
					} else {
						/* Write in memory */
						cpu_write_data_in_mem(cpssp_cpu, src, s_op.op2_mem);
					}
					return true;
				}
				break;

			default:
				/* FIXME: add something*/
				return true;	
	}
	return false;
}

static bool
cpu_readb(void *_cpssp_cpu, uint32_t addr, uint8_t *valp){
	/* We dont read from CPU */

	return false;
}

static bool
cpu_writeb(void *_cpssp_cpu, uint32_t addr, uint8_t val){
	/* We dont write into CPU */

	return false;
}


void *
cpu_create(struct sig_host_bus *port_host)
{
	struct cpssp_cpu *cpssp_cpu;
	static const struct sig_host_bus_funcs hf = {
		.readb = cpu_readb,
		.writeb = cpu_writeb
	};

	cpssp_cpu = malloc(sizeof(struct cpssp_cpu));
	assert(cpssp_cpu != NULL);

	cpssp_cpu->port_host = port_host;
	/* Set base pointer to start address of ROM.
	 * The address is hardcoded, like in real hardware.
	 */
	cpssp_cpu->eip = 0xE000;

	sig_host_bus_connect(port_host, cpssp_cpu, &hf);
	return cpssp_cpu;
}

void
cpu_destroy(void *_cpssp_cpu)
{
	free(_cpssp_cpu);
}
