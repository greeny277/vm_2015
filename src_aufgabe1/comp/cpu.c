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
 * This struct works for SIB and ModRM Byte
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
	uint32_t *op1_addr;
	uint32_t *op2_addr;
} op_addr;

static void computeAddress(cpssp *, uint8_t, uint32_t *, uint32_t *);
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
 * @brief This method reads out each byte to determine
 * the complete address of operand 1 and 2.
 */
static bool
cpu_decodeOperands(cpssp *cpssp, op_addr *addr, bool is_8bit)
{
	uint8_t mod_rm = 0;
	uint8_t sib = 0;
	uint32_t *base = 0;
	uint32_t *index = 0;
	uint8_t scale = 0;

	/* Eval MOD_RM Byte */
	mod_rm = cpu_get_byte_inc(cpssp);
	modsib s_modrm;
	memset(&mod_rm, 0, sizeof(modsib));

	if(false == cpu_evalByte(cpssp, &s_modrm, mod_rm, is_8bit)){
		return false;
	}
	
	addr->op1_addr = s_modrm.op1;

	/* Check for SIB Byte */
	if(s_modrm.op2_name == ESP && s_modrm.addr_or_scale_mode != REGISTER){
		sib = cpu_get_byte_inc(cpssp);
		modsib s_sib;
		memset(&s_sib,0,sizeof(modsib));

		if(false == cpu_evalByte(cpssp, &s_sib, sib, is_8bit)){
				return false;
		}

		scale = s_sib.addr_or_scale_mode;
		scale = 1 << scale;
		base = s_sib.op2;
		index = s_sib.op1;
		// TODO Interpret SIB Byte
		// TODO read out immidiate
	} else {
		// TODO read out immidiate
		computeAddress(cpssp, s_modrm.addr_or_scale_mode, s_modrm.op2, addr->op2_addr);
	}

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
 * @brief Depending on mode return address.
 *		This method is always called for operand2.
 */
static void
computeAddress(cpssp *cpssp, uint8_t mode, uint32_t *addr, uint32_t *new_addr){
	uint8_t displ1, displ2, displ3, displ4;
	uint32_t displacement_complete;
	switch(mode){
		case NO_DISPLACEMENT:
			/* Indirection with no displacement */
			*new_addr = sig_host_bus_readb(cpssp->port_host, (void *)cpssp, ((*addr)));
			return;

		case DISPLACEMENT_8:
			/* Read one extra byte from bus */
			displ1 = cpu_get_byte_inc(cpssp);
			/* Indirection with 8 bit displacement */
			*new_addr = displ1 + (*addr);
			return;

		case DISPLACEMENT_32:
			/* Indirection with 32 bit displacement */

			/* Read lowest byte first */
			displ1 = cpu_get_byte_inc(cpssp);
			displ2 = cpu_get_byte_inc(cpssp);
			displ3 = cpu_get_byte_inc(cpssp);
			displ4 = cpu_get_byte_inc(cpssp);

			displacement_complete = displ1;
			displacement_complete |= (displ2 << 8);
			displacement_complete |= (displ3 << 16);
			displacement_complete |= (displ4 << 24);

			*new_addr = displacement_complete + (*addr);
			return;
		case REGISTER:
			*new_addr = *addr;
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

	/* read the first byte from instruction pointer */
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
cpudestroy(void *_cpssp)
{
	free(_cpssp);
}
