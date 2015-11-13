/* $Id: setup.c 10023 2014-10-16 09:56:44Z raimue $
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */

#include <stdbool.h>
#include "sig_host_bus.h"
#include "disk_ctrl.h"
#include "serial_ctrl.h"
#include "bios_rom.h"
#include "cpu.h"
#include "memory.h"

/** config of the entire vm */
struct cpssp {
	/* busses */
	/** host bus */
	struct sig_host_bus *host_bus;

	/* components */
	/** memory */
	void *comp_memory;
	/** disk controller */
	void *comp_disk_ctrl;
	/** serial controller */
	void *comp_serial_ctrl;
	/** bios rom */
	void *comp_bios_rom;
	/** cpu */
	void *comp_cpu;
	/* ... */

	/** filename of bios rom */
	const char *setup_bios_rom;
};

static void
setup_create(struct cpssp *cpssp)
{
	/* create busses */
	cpssp->host_bus = sig_host_bus_create();

	/* create components */
	cpssp->comp_memory = memory_create(cpssp->host_bus);
	cpssp->comp_disk_ctrl = disk_ctrl_create(cpssp->host_bus, "disk.bin");
	cpssp->comp_serial_ctrl = serial_ctrl_create(cpssp->host_bus);
	cpssp->comp_bios_rom =
		bios_rom_create(cpssp->host_bus, cpssp->setup_bios_rom);
	cpssp->comp_cpu = cpu_create(cpssp->host_bus);
}

static void
setup_destroy(struct cpssp *cpssp)
{
	/* tear down components */
	memory_destroy(cpssp->comp_memory);
	disk_ctrl_destroy(cpssp->comp_disk_ctrl);
	serial_ctrl_destroy(cpssp->comp_serial_ctrl);
	bios_rom_destroy(cpssp->comp_bios_rom);
	cpu_destroy(cpssp->comp_cpu);

	/* tear down busses */
	sig_host_bus_destroy(cpssp->host_bus);
}

void
setup_run(struct cpssp *cpssp)
{
	bool ret = true;

	while (ret) {
		ret = cpu_step(cpssp->comp_cpu);
	}
}

int
main(int argc, char **argv)
{
	static struct cpssp cpssp;
	if (argc == 2) {
		cpssp.setup_bios_rom = argv[1];
	} else {
		cpssp.setup_bios_rom = "comp/roms/bios_print_simple.rom";
	}

	setup_create(&cpssp);
	setup_run(&cpssp);
	setup_destroy(&cpssp);

	return 0;
}
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */