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


static uint8_t err_reg;
static uint32_t bnr;
static uint8_t read_write;
static FILE *f;

static char buffer[BUF_SIZE];

struct cpssp {
	/** ports */
	struct sig_host_bus *port_host;

	/** state */
};


static bool
disk_ctrl_readb(void *_cpssp, uint32_t addr, uint8_t *valp){
	/*
	 * This works because addr is an unsigned int.
	 * Case: 0 < addr < 0xD offset is bigger than 0x0400 because
	 * of underflow
	 * Case: 0xD4 >= addr: offset is higher than 0x0400
	 */
	uint32_t offset = addr - DISK_MEM_BASE;

	if(offset > DISK_MEM_BORDER){
		return false;
	}

	/* Adress is in range [0xD000; 0xD400] */

	*valp = 0;

	switch (offset) {
		case DISK_BNR:
			*valp = bnr & 0xFF;
			return true;
		case DISK_BNR_1:
			*valp = 8 >> (bnr & 0xFF00);
			return true;
		case DISK_BNR_2:
			*valp = 16 >> (bnr & 0xFF0000);
			return true;
		case DISK_BNR_3:
			*valp = 24 >> (bnr & 0xFF000000);
			return true;
		case DISK_ERR_REG:
			*valp = err_reg;
			return true;
		case DISK_READ_WRITE:
			*valp = read_write;
			return true;
		default:
			/* Check if adress is in range [0xD200, 0xD400] */
			if(offset < DISK_MEM_BUF){
				return false;
			}

			offset = offset - DISK_MEM_BUF;
			*valp = buffer[offset];
			return true;
	}
}

static bool
read_from_disk(){
		int disk_addr = BUF_SIZE * bnr;
		
		if(disk_addr + BUF_SIZE > DISK_SIZE){
				return false;
		}
		/* Set filepointer to dis_addr */
		if(0 != fseek(f,disk_addr,SEEK_SET)){
			return false;
		}

		/* Read BUF_SIZE bytes in buffer */
		return (BUF_SIZE == fread(buffer,BUF_SIZE, sizeof(char),f));

}

static bool
write_to_disk(){
		int disk_addr = BUF_SIZE * bnr;
		
		if(disk_addr + BUF_SIZE > DISK_SIZE){
				return false;
		}
		
		/* Set filepointer to dis_addr */
		if(0 != fseek(f,disk_addr,SEEK_SET)){
			return false;
		}

		/* Read BUF_SIZE bytes in buffer */
		return (BUF_SIZE == fwrite(buffer,BUF_SIZE, sizeof(char),f));
}

static bool
disk_ctrl_writeb(void *_cpssp, uint32_t addr, uint8_t val){
	uint32_t offset = addr - DISK_MEM_BASE;

	if(offset > DISK_MEM_BORDER){
		return false;
	}
	
	/* Adress is in range [0xD000; 0xD400] */

	/* TODO: check address in BNR? */

	switch (offset) {
		case DISK_BNR:
			bnr = bnr & 0x00;
			bnr = bnr | val;
			return true;
		case DISK_BNR_1:
			bnr = bnr & 0x00FF;
			bnr = (val << 8) | bnr;
			return true;
		case DISK_BNR_2:
			bnr = bnr & 0x00FF;
			bnr = (val << 16) | bnr;
			return true;
		case DISK_BNR_3:
			bnr = bnr & 0x00FF;
			bnr = (val << 24) | bnr;
			return true;
		case DISK_ERR_REG:
			err_reg = val;
			return true;
		case DISK_READ_WRITE:
			/* XXX: clear the err_reg?? */
			err_reg = 0;
			read_write = val;
			if(read_write == 0) {
				if(!read_from_disk()){
					err_reg = 1;
					return false;
				}
				return true;

			} else if(read_write == 1)	{
				if(!write_to_disk()){
					err_reg = 1;
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
			buffer[offset] = val;
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

	f = fopen(fn, "w+");
	assert(f != NULL);

	int ret = fseek(f, DISK_SIZE, SEEK_SET);
	assert(ret == 0);

	sig_host_bus_connect(port_host, cpssp, &hf);

	return cpssp;
}

	void
disk_ctrl_destroy(void *_cpssp)
{
	fclose(f);
}
