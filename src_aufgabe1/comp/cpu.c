#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "cpu.h"

#define NO_DISPLACEMENT 0
#define DISPLACEMENT_8  1
#define DISPLACEMENT_32 2
#define REGISTER        3

#define EAX 0
#define ECX 1
#define EDX 2
#define EBX 3
#define ESP 4
#define EBP 5
#define ESI 6
#define EDI 7


#define AH 8
#define AL 9
#define BH 10
#define BL 11
#define CH 12
#define CL 13
#define DH 14
#define DL 15


typedef struct cpssp {
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
} cpssp;

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
	/* Contains pointer to register in case of REGISTER addressment */
	uint32_t *op2_reg;
	/* Contains address in case of memory addressment */
	uint32_t op2_mem;
	/* High or Low Byte in case of 8 bit */
	/* uint32_t is_high; */
} op_addr;

/* Method declarations */
static void computeAddress(cpssp *, uint8_t, uint32_t *, op_addr *);
static uint8_t cpu_get_byte_inc(cpssp *);

/* @brief Write back address of CPU own register
 * based on bit pattern.
 *
 * @param cpssp  CPU instance
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
cpu_evalRegister(cpssp *cpssp, uint8_t reg, uint32_t **reg_addr, bool is_8bit){
	/* FIXME Is it really necessary to return register name? */
	switch(reg){
		case EAX:
			*reg_addr = &(cpssp->eax);
			if(is_8bit){
					return AL;
			} else {
					return EAX;
			}
		case EBX:
			*reg_addr = &(cpssp->ebx);
			if(is_8bit){
					return BL;
			} else {
					return EBX;
			}
		case ECX:
			*reg_addr = &(cpssp->ecx);
			if(is_8bit){
					return CL;
			} else {
					return ECX;
			}
		case EDX:
			*reg_addr = &(cpssp->edx);
			if(is_8bit){
					return DL;
			} else {
					return EDX;
			}
		case ESP:
			if(is_8bit){
					*reg_addr = &(cpssp->eax);
					return AH;
			} else {
					*reg_addr = &(cpssp->esp);
					return ESP;
			}
		case EBP:
			if(is_8bit){
					*reg_addr = &(cpssp->ecx);
					return CH;
			} else {
					*reg_addr = &(cpssp->ebp);
					return EBP;
			}
		case ESI:
			if(is_8bit){
					*reg_addr = &(cpssp->edx);
					return DH;
			} else {
					*reg_addr = &(cpssp->esi);
					return ESI;
			}
		case EDI:
			if(is_8bit){
					*reg_addr = &(cpssp->ebx);
					return BH;
			} else {
					*reg_addr = &(cpssp->edi);
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
 * @param cpssp    CPU instance
 * @param mod      Pointer to structure where results will be saved in
 * @param byte     byte that will be analysed
 * @param is_8bit  Is it an 8 bit instruction
 *
 * @return         True on success.
 *                 False in case of a huge programming mistake.
 */
static bool
cpu_evalByte(cpssp *cpssp, modsib *mod, uint8_t byte, uint8_t is_8bit)
{
	uint32_t *op1;
	uint32_t *op2;

	mod->op1_name = cpu_evalRegister(cpssp, byte & (0x7), &op1, is_8bit);
	if( -1 == mod->op1_name) {
		return false;
	}

	mod->op2_name = cpu_evalRegister(cpssp, (byte >> 3) & 0x7, &op2, is_8bit);
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
 * @param cpssp    CPU instance
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
cpu_decodeOperands(cpssp *cpssp, op_addr *addr, bool is_8bit)
{
	uint8_t mod_rm = 0;

	/* Eval MOD_RM Byte */
	mod_rm = cpu_get_byte_inc(cpssp);
	modsib s_modrm;
	memset(&mod_rm, 0, sizeof(modsib));

	if(false == cpu_evalByte(cpssp, &s_modrm, mod_rm, is_8bit)){
		return false;
	}
	
	/* Set address of op1 */
	addr->op1_reg = s_modrm.op1;

	/* Check if SIB byte follows */
	if(s_modrm.op2_name == ESP && s_modrm.addr_or_scale_mode != REGISTER){
		/* Define variables for SIB byte */
		uint32_t *base;
		uint32_t *index;

		uint8_t sib = 0;
		uint8_t scale = 0;


		/* Read next byte increment eip */
		sib = cpu_get_byte_inc(cpssp);
		modsib s_sib;
		memset(&s_sib,0,sizeof(modsib));

		if(false == cpu_evalByte(cpssp, &s_sib, sib, is_8bit)){
				return false;
		}

		scale = s_sib.addr_or_scale_mode;
		scale = 1 << scale;
		base = s_sib.op2;
		
		// TODO base reg is EBP and index reg is ESP!!!
		if(s_sib.op1_name == ESP) {
			/* Index is ESP */
			computeAddress(cpssp, s_modrm.addr_or_scale_mode, s_modrm.op2, addr);
		
		} else if (s_sib.op2_name == EBP){
			/* Base is EBP */
			index = s_sib.op1;
			/* Base is not used */
			addr->op2_mem = *index * scale;
				
		} else {
				index = s_sib.op1;
				addr->op2_mem = *base + (*index * scale);
		}

	} else if(s_modrm.op2_name == EBP && s_modrm.addr_or_scale_mode == NO_DISPLACEMENT){
		/* Special case: Reject op2 and read in a 32 Bit displacement instead. */
		uint32_t base_0 = 0;
		computeAddress(cpssp, DISPLACEMENT_32, &base_0, addr);
	} else {
		/* Compute address of op2 */
		computeAddress(cpssp, s_modrm.addr_or_scale_mode, s_modrm.op2, addr);
	}
	// TODO read out immidiate/constant value

	return true;
}

static uint8_t
cpu_get_byte_inc(cpssp *cpssp)
{
	uint8_t next_byte = sig_host_bus_readb(cpssp->port_host, (void *)cpssp, cpssp->eip);
	cpssp->eip = cpssp->eip + 1;

	return next_byte;
}


/*
 * @brief          Compute address of operand 2 subjected to the addressing mode
 *
 * @param cpssp    CPU instance
 *
 * @param mode     Contains addressing mode
 *
 * @param addr     Pointer that contains the base address. Usually
 *                 a pointer to CPU own register.
 */
static void
computeAddress(cpssp *cpssp, uint8_t mode, uint32_t *addr, op_addr *op){
	uint8_t displ1, displ2, displ3, displ4;
	uint32_t displacement_complete;
	switch(mode){
		case NO_DISPLACEMENT:
			/* Indirection with no displacement */
			op->op2_mem = *addr;
			return;

		case DISPLACEMENT_8:
			/* Read one extra byte from bus */
			displ1 = cpu_get_byte_inc(cpssp);
			/* Indirection with 8 bit displacement */
			op->op2_mem = displ1 + (*addr);
			return;

		case DISPLACEMENT_32:
			/* Indirection with 32 bit displacement */

			/* Attention: Lowest byte will be read first */
			displ1 = cpu_get_byte_inc(cpssp);
			displ2 = cpu_get_byte_inc(cpssp);
			displ3 = cpu_get_byte_inc(cpssp);
			displ4 = cpu_get_byte_inc(cpssp);

			displacement_complete = displ1;
			displacement_complete |= (displ2 << 8);
			displacement_complete |= (displ3 << 16);
			displacement_complete |= (displ4 << 24);

			op->op2_mem = displacement_complete + (*addr);
			return;
		case REGISTER:
			op->op2_reg = addr;
			return;
	}
	return;
}

bool
cpu_step(void *_cpssp)
{
	/* cast */
	cpssp *cpssp = (struct cpssp *) _cpssp;

	uint8_t op_code;

	/* read the first byte from instruction pointer and increment ip
	 * afterards */
	op_code = cpu_get_byte_inc(cpssp);
	
	op_addr s_op;
	memset(&s_op, 0, sizeof(op_addr));

	switch(op_code) {
			case 0x88:
				/* MOV r8 to r/m8 */
				if(!cpu_decodeOperands(cpssp, &s_op, true)){
					return false;
				}
				
			default:
				/* FIXME: add something*/
				return true;	
	}
	return false;
}

static bool
cpu_readb(void *_cpssp, uint32_t addr, uint8_t *valp){
	/* We dont read from CPU */

	return false;
}

static bool
cpu_writeb(void *_cpssp, uint32_t addr, uint8_t val){
	/* We dont write into CPU */

	return false;
}


void *
cpu_create(struct sig_host_bus *port_host)
{
	struct cpssp *cpssp;
	static const struct sig_host_bus_funcs hf = {
		.readb = cpu_readb,
		.writeb = cpu_writeb
	};

	cpssp = malloc(sizeof(struct cpssp));
	assert(cpssp != NULL);

	cpssp->port_host = port_host;
	/* Set base pointer to start address of ROM.
	 * The address is hardcoded, like in real hardware.
	 */
	cpssp->eip = 0xE000;

	sig_host_bus_connect(port_host, cpssp, &hf);
	return cpssp;
}

void
cpu_destroy(void *_cpssp)
{
	free(_cpssp);
}
