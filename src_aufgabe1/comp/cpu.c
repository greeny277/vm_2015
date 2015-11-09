#include <stdlib.h>
#include <assert.h>

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
static bool
cpu_evalRegister(cpssp *cpssp, uint8_t reg, uint32_t **reg_addr){
	switch(reg){
		case EAX:
			*reg_addr = &(cpssp->eax);
			return true;
		case EBX:
			*reg_addr = &(cpssp->ebx);
			return true;
		case ECX:
			*reg_addr = &(cpssp->ecx);
			return true;
		case EDX:
			*reg_addr = &(cpssp->edx);
			return true;
		case ESP:
			*reg_addr = &(cpssp->esp);
			return true;
		case EBP:
			*reg_addr = &(cpssp->ebp);
			return true;
		case ESI:
			*reg_addr = &(cpssp->esi);
			return true;
		case EDI:
			*reg_addr = &(cpssp->edi);
			return true;
		default:
			return false;
	}
}

static void
cpu_modRM(cpssp *cpssp, modRM *mod, uint8_t modRM){
	uint32_t *src;
	uint32_t *dest;
	uint8_t mode;

	cpu_evalRegister(cpssp, modRM & (0x7), &src);
	cpu_evalRegister(cpssp, (modRM >> 3) & 0x7, &dest);

	mode = (modRM >> 6) & (0x7);

	mod->addr_mode = mode;
	mod->dest = dest;
	mod->src = src;
}

bool
cpu_step(void *_cpssp)
{
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
