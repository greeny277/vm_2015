#include "disk_ctrl.h"
#include <assert.h>


static bool
disk_ctrl_readb(void *_disk_state, uint32_t addr, uint8_t *valp){
	disk_state* disk_state = (struct disk_state*) _disk_state;
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
		case DISK_BNR_1:
		case DISK_BNR_2:
		case DISK_BNR_3:
			*valp = ((uint8_t*) &disk_state->bnr)[offset];
			return true;
		case DISK_ERR_REG:
			*valp = disk_state->err_reg;
			return true;
		case DISK_READ_WRITE:
			*valp = disk_state->read_write;
			return true;
		default:
			/* Check if adress is in range [0xD200, 0xD400] */
			if(offset < DISK_MEM_BUF){
				return false;
			}

			offset = offset - DISK_MEM_BUF;
			*valp = disk_state->buffer[offset];
			return true;
	}
}

static bool
read_from_disk(void *_disk_state){
		disk_state* disk_state = (struct disk_state*) _disk_state;
		int disk_addr = BUF_SIZE * disk_state->bnr;
		
		if(disk_addr + BUF_SIZE > DISK_SIZE){
				return false;
		}
		/* Set filepointer to disk_addr */
		if(0 != fseek(disk_state->f, disk_addr, SEEK_SET)){
			return false;
		}

		/* Read BUF_SIZE bytes in buffer */
        size_t byte_read = fread(disk_state->buffer, 1, BUF_SIZE, disk_state->f);

		/* Check for error */
		if(ferror(disk_state->f)){
			fprintf(stderr, "An error occured in fread(3)\n");
			return false;
		} else {
			return true;
		}
}

static bool
write_to_disk(void *_disk_state){
		disk_state* disk_state = (struct disk_state*) _disk_state;
		int disk_addr = BUF_SIZE * disk_state->bnr;
		
		if(disk_addr + BUF_SIZE > DISK_SIZE){
				return false;
		}
		
		/* Set filepointer to dis_addr */
		if(0 != fseek(disk_state->f, disk_addr, SEEK_SET)){
			return false;
		}

		/* Read BUF_SIZE bytes in buffer */
		return (BUF_SIZE == fwrite(disk_state->buffer, sizeof(char), BUF_SIZE, disk_state->f));
}

static bool
disk_ctrl_writeb(void *_disk_state, uint32_t addr, uint8_t val){
	disk_state* disk_state = (struct disk_state*) _disk_state;
	uint32_t offset = addr - DISK_MEM_BASE;

	if(offset >= DISK_MEM_BORDER){
		return false;
	}
	
	/* Adress is in range [0xD000; 0xD400] */

	switch (offset) {
		case DISK_BNR:
		case DISK_BNR_1:
		case DISK_BNR_2:
		case DISK_BNR_3:
			((uint8_t*) &disk_state->bnr)[offset] = val;
			return true;
		case DISK_ERR_REG:
			disk_state->err_reg = val;
			return true;
		case DISK_READ_WRITE:
			disk_state->read_write = val;
			if(val == 0) {
				if(!read_from_disk(_disk_state)){
					disk_state->err_reg = 1;
					return false;
				}
				return true;

			} else if(val == 1) {
				if(!write_to_disk(_disk_state)){
					disk_state->err_reg = 1;
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
			disk_state->buffer[offset] = val;
			return true;
	}
}

	void *
disk_ctrl_create(struct sig_host_bus *port_host, const char *fn)
{
	disk_state *disk_state;
	static const struct sig_host_bus_funcs hf = {
		.readb = disk_ctrl_readb,
		.writeb = disk_ctrl_writeb
	};

	disk_state = malloc(sizeof(struct disk_state));
	assert(disk_state != NULL);
	disk_state->port_host = port_host;

	disk_state->f = fopen(fn, "r+");
	assert(disk_state->f != NULL);

	int ret = fseek(disk_state->f, DISK_SIZE, SEEK_SET);
	assert(ret == 0);

	sig_host_bus_connect(port_host, disk_state, &hf);

	return disk_state;
}

	void
disk_ctrl_destroy(void *_disk_state)
{
	disk_state* disk_state = (struct disk_state*) _disk_state;
	fclose(disk_state->f);
	free(_disk_state);
}
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
