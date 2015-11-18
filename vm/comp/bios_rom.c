#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bios_rom.h"

/* Declaration of static methods */
static bool rom_readb(void *_rom_state, uint32_t addr, uint8_t *valp);
static bool rom_writeb(void *_rom_state, uint32_t addr, uint8_t val);

static bool
rom_readb(void *_rom_state, uint32_t addr, uint8_t *valp){

	rom_state *rom_state = (struct rom_state *) _rom_state;
	uint32_t offset = addr - ROM_MEM_BASE;

	if(offset >= ROM_SIZE){
		return false;
	}

	*valp = rom_state->rom[offset];

	return true;
}

static bool
rom_writeb(void *_rom_state, uint32_t addr, uint8_t val){
	/* We dont write into ROM */

	/* Ignoring */
	return false;
}

void *
bios_rom_create(struct sig_host_bus *port_host, const char *rf)
{
	rom_state *rom_state;
	static const struct sig_host_bus_funcs hf = {
		.readb = rom_readb,
		.writeb = rom_writeb
	};

	rom_state = malloc(sizeof(struct rom_state));
	assert(rom_state != NULL);

	rom_state->port_host = port_host;

	/* Get size of BIOS file */
	struct stat s;
	assert(0 == stat(rf, &s));
	off_t bios_size = s.st_size;

	/* Check if BIOS file fits into ROM component */
	assert(bios_size <= ROM_SIZE);

	/* Open bios file */
	FILE *f = fopen(rf, "r");
	assert(f != NULL);

	/* Zero out rom buffer */
	memset(rom_state->rom, 0, ROM_SIZE);

	/* Read BIOS file in rom buffer */
	assert(bios_size == fread(rom_state->rom, 1, bios_size, f));

	/* Close filepointer */
	fclose(f);

	sig_host_bus_connect(port_host, rom_state, &hf);

	return rom_state;
}

void
bios_rom_destroy(void *_rom_state)
{
	free(_rom_state);
}
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
