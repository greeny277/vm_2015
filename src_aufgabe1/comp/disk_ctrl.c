#include "disk_ctrl.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


#define DISK_MEM_BASE   0xD000
#define DISK_MEM_BORDER 0x400
#define DISK_MEM_BUF    0x200

#define DISK_BNR		0x0
#define DISK_BNR_1		0x1
#define DISK_BNR_2		0x2
#define DISK_BNR_3		0x3
#define DISK_ERR_REG	0x7
#define DISK_READ_WRITE 0xb

#define DISK_SIZE		5*1024*1024
#define BUF_SIZE		512

struct cpssp {
	/** ports */
	struct sig_host_bus *port_host;

	/** state */
	uint8_t err_reg;
	/* Big endian representation */
	uint32_t bnr;
	uint8_t read_write;
	FILE *f;

	char buffer[BUF_SIZE];
};

static bool
disk_ctrl_readb(void *_cpssp, uint32_t addr, uint8_t *valp){
	struct cpssp* cpssp = (struct cpssp*) _cpssp;
	/*
	 * This works because addr is an unsigned int.
	 * Case: 0 < addr < 0xD offset is bigger than 0x0400 because
	 * of underflow
	 * Case: 0xD4 >= addr: offset is higher than 0x0400
	 */
	uint32_t offset = addr - DISK_MEM_BASE;

	if(offset >= DISK_MEM_BORDER){
		return false;
	}

	/* Adress is in range [0xD000; 0xD400] */

	*valp = 0;

	switch (offset) {
		case DISK_BNR:
			*valp = cpssp->bnr & 0xFF;
			return true;
		case DISK_BNR_1:
			*valp = 8 >> (cpssp->bnr & 0xFF00);
			return true;
		case DISK_BNR_2:
			*valp = 16 >> (cpssp->bnr & 0xFF0000);
			return true;
		case DISK_BNR_3:
			*valp = 24 >> (cpssp->bnr & 0xFF000000);
			return true;
		case DISK_ERR_REG:
			*valp = cpssp->err_reg;
			return true;
		case DISK_READ_WRITE:
			*valp = cpssp->read_write;
			return true;
		default:
			/* Check if adress is in range [0xD200, 0xD400] */
			if(offset < DISK_MEM_BUF){
				return false;
			}

			offset = offset - DISK_MEM_BUF;
			*valp = cpssp->buffer[offset];
			return true;
	}
}

static bool
read_from_disk(void *_cpssp){
		struct cpssp* cpssp = (struct cpssp*) _cpssp;
		int disk_addr = BUF_SIZE * cpssp->bnr;
		
		if(disk_addr + BUF_SIZE > DISK_SIZE){
				return false;
		}
		/* Set filepointer to disk_addr */
		if(0 != fseek(cpssp->f, disk_addr, SEEK_SET)){
			return false;
		}

		/* Read BUF_SIZE bytes in buffer */
		return (BUF_SIZE == fread(cpssp->buffer, BUF_SIZE, sizeof(char), cpssp->f));

}

static bool
write_to_disk(void *_cpssp){
		struct cpssp* cpssp = (struct cpssp*) _cpssp;
		int disk_addr = BUF_SIZE * cpssp->bnr;
		
		if(disk_addr + BUF_SIZE > DISK_SIZE){
				return false;
		}
		
		/* Set filepointer to dis_addr */
		if(0 != fseek(cpssp->f, disk_addr, SEEK_SET)){
			return false;
		}

		/* Read BUF_SIZE bytes in buffer */
		return (BUF_SIZE == fwrite(cpssp->buffer, BUF_SIZE, sizeof(char), cpssp->f));
}

static bool
disk_ctrl_writeb(void *_cpssp, uint32_t addr, uint8_t val){
	struct cpssp* cpssp = (struct cpssp*) _cpssp;
	uint32_t offset = addr - DISK_MEM_BASE;

	if(offset >= DISK_MEM_BORDER){
		return false;
	}
	
	/* Adress is in range [0xD000; 0xD400] */

	switch (offset) {
		case DISK_BNR:
			cpssp->bnr &= ~0xFF;
			cpssp->bnr |= val;
			return true;
		case DISK_BNR_1:
			cpssp->bnr &= ~0xFF00;
			cpssp->bnr |= (val << 8);
			return true;
		case DISK_BNR_2:
			cpssp->bnr &= ~0xFF0000;
			cpssp->bnr |= (val << 16);
			return true;
		case DISK_BNR_3:
			cpssp->bnr &= ~0xFF000000;
			cpssp->bnr |= (val << 24);
			return true;
		case DISK_ERR_REG:
			cpssp->err_reg = val;
			return true;
		case DISK_READ_WRITE:
			cpssp->read_write = val;
			if(val == 0) {
				if(!read_from_disk(_cpssp)){
					cpssp->err_reg = 1;
					return false;
				}
				return true;

			} else if(val == 1) {
				if(!write_to_disk(_cpssp)){
					cpssp->err_reg = 1;
					return false;
				}
				return true;
			} else {
				return true;
			}
		default:
			/* Check if adress is in range [0xD200, 0xD400] */
			if(offset < DISK_MEM_BUF){
				return false;
			}

			offset = offset - DISK_MEM_BUF;
			cpssp->buffer[offset] = val;
			return true;
	}
}

	void *
disk_ctrl_create(struct sig_host_bus *port_host, const char *fn)
{
	struct cpssp *cpssp;
	static const struct sig_host_bus_funcs hf = {
		.readb = disk_ctrl_readb,
		.writeb = disk_ctrl_writeb
	};

	cpssp = malloc(sizeof(struct cpssp));
	assert(cpssp != NULL);
	cpssp->port_host = port_host;

	cpssp->f = fopen(fn, "w+");
	assert(cpssp->f != NULL);

	int ret = fseek(cpssp->f, DISK_SIZE, SEEK_SET);
	assert(ret == 0);

	sig_host_bus_connect(port_host, cpssp, &hf);

	return cpssp;
}

	void
disk_ctrl_destroy(void *_cpssp)
{
	struct cpssp* cpssp = (struct cpssp*) _cpssp;
	fclose(cpssp->f);
	free(_cpssp);
}
