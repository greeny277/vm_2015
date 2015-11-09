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

typedef struct modRM {
	uint32_t *src;
	uint32_t *dest;
	uint8_t addr_mode;
	/* Indicates if a SIB byte follows (set to 1) or not */
	bool sib_byte;
	uint8_t src_reg;
	uint8_t dest_reg;

} modRM;

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
cpu_modRM(cpssp *cpssp, modRM *mod, uint8_t modRM, bool is_8bit){
	uint32_t *src;
	uint32_t *dest;
	uint8_t mode;


	mod->src_reg = cpu_evalRegister(cpssp, modRM & (0x7), &src, is_8bit);
	if( -1 == mod->src_reg) {
		return false;
	}

	mod->dest_reg = cpu_evalRegister(cpssp, (modRM >> 3) & 0x7, &dest, is_8bit);
	if( -1 == mod->dest_reg) {
		return false;
	}

	mode = (modRM >> 6) & (0x7);

	mod->addr_mode = mode;
	mod->dest = dest;
	mod->src = src;

	return true;
}

static uint8_t
cpu_get_byte_inc(cpssp *cpssp)
{
	uint8_t next_byte = sig_host_bus_readb(cpssp->port_host, (void *)cpssp, cpssp->eip);
	cpssp->eip = cpssp->eip + 1;

	return next_byte;
}

bool
cpu_step(void *_cpssp)
{

	/* cast */
	cpssp *cpssp = (struct cpssp *) _cpssp;

	/* read the first byte from instruction pointer */

	// FIXME: where to get the actual instruction, ROM or RAM??
	
	uint8_t op_code = cpu_get_byte_inc(cpssp);
	
	uint8_t mod_rm;
	uint8_t sib;

	modRM mod;
	memset(&mod,0,sizeof(modRM));

	switch(op_code) {
			case 0x88:
				/* MOV r8 to r/m8 */
				mod_rm = cpu_get_byte_inc(cpssp);
				if(!cpu_modRM(cpssp, &mod, mod_rm, true)){
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
