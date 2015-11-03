#include "bios_rom.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>


#define ROM_MEM_BASE 0xE000
#define ROM_SIZE 4*1024

struct cpssp {
	/** ports */
	struct sig_host_bus *port_host;

	/** state */
	char rom[ROM_SIZE];
};

static bool
rom_readb(void *_cpssp, uint32_t addr, uint8_t *valp){

	struct cpssp* cpssp = (struct cpssp*) _cpssp;
	uint32_t offset = addr - ROM_MEM_BASE;

	if(offset >= ROM_SIZE){
		return false;
	}

	*valp = cpssp->rom[offset];

	return true;
}

static bool
rom_writeb(void *_cpssp, uint32_t addr, uint8_t val){
	/* We dont write into ROM */

	/* Ignoring */
	return false;
}

void *
bios_rom_create(struct sig_host_bus *port_host, const char *rf)
{
	struct cpssp *cpssp;
	static const struct sig_host_bus_funcs hf = {
		.readb = rom_readb,
		.writeb = rom_writeb
	};

	cpssp = malloc(sizeof(struct cpssp));
	assert(cpssp != NULL);

	cpssp->port_host = port_host;

	/* Get size of BIOS file */
	struct stat s;
	assert(0 == stat(rf, &s));
	off_t bios_size = s.st_size;

	/* Check if BIOS file fits into ROM */
	assert(bios_size <= ROM_SIZE);

	/* Open bios file */
	FILE *f = fopen(rf, "r");
	assert(f != NULL);

	/* Zero out rom buffer */
	memset(cpssp->rom, 0, ROM_SIZE);

	/* Read BIOS file in rom buffer */
	assert(ROM_SIZE == fread(cpssp->rom, ROM_SIZE, sizeof(char), f));

	/* Close filepointer */
	fclose(f);

	sig_host_bus_connect(port_host, cpssp, &hf);

	return cpssp;
}

void
bios_rom_destroy(void *_cpssp)
{
	free(_cpssp);
}
