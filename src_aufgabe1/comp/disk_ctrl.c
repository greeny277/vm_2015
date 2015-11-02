#include "disk_ctrl.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


#define DISK_MEM_BASE   0xD000
#define DISK_MEM_BASE_2 0x0200
#define DISK_MEM_BASE_3 0x0300

#define DISK_MEM_MASK_UP 0xF000
#define DISK_MEM_MASK_DOWN 0x0F00

#define DISK_BNR		0x0000
#define DISK_BNR_1		0x0001
#define DISK_BNR_2		0x0002
#define DISK_BNR_3		0x0003
#define DISK_ERR_REG	0x0007
#define DISK_READ_WRITE 0x000b

#define DISK_SIZE		5*1024*1024
#define BUF_SIZE		512


static uint8_t err_reg;
static uint32_t bnr;
static bool read_write;

static char buffer[BUF_SIZE];

struct cpssp {
	/** ports */
	struct sig_host_bus *port_host;

	/** state */
};

static bool
disk_ctrl_readb(void *_cpssp, uint32_t addr, uint8_t *valp){
	return 0;
}

static bool
disk_ctrl_writeb(void *_cpssp, uint32_t addr, uint8_t val){
	return 0;
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

	FILE *f = fopen(fn, "w+");
	assert(f != NULL);

	int ret = fseek(f, DISK_SIZE, SEEK_SET);
	assert(ret == 0);

	sig_host_bus_connect(port_host, cpssp, &hf);

	return cpssp;
}

void
disk_ctrl_destroy(void *_cpssp)
{
}
