/* $Id: sig_host_bus.c 10023 2014-10-16 09:56:44Z raimue $
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */

#include "sig_host_bus.h"
#include <stdlib.h>
#include <assert.h>

struct sig_host_bus *
sig_host_bus_create(void)
{
	struct sig_host_bus *ret;

	ret = malloc(sizeof(struct sig_host_bus));
	assert(ret != NULL);

	ret->nmembers = 0;
	return ret;
}


void
sig_host_bus_destroy(struct sig_host_bus *bus)
{
	free(bus);
}

void
sig_host_bus_connect(
	struct sig_host_bus *bus,
	void *s,
	const struct sig_host_bus_funcs *f
)
{
	assert(bus != NULL);
	assert(s != NULL);
	assert(bus->nmembers
		< (sizeof(bus->members) / sizeof(bus->members[0])));

	bus->members[bus->nmembers].s = s;
	bus->members[bus->nmembers].f = f;
	bus->nmembers++;
}

void
sig_host_bus_writeb(
	const struct sig_host_bus *bus,
	void *s,
	uint32_t addr,
	uint8_t val
)
{
	unsigned int i;
	bool ret;

	for (i = 0; i < bus->nmembers; i++) {
		if (bus->members[i].s == s) {
			continue;
		}

		if (bus->members[i].f == NULL) {
			continue;
		}

		if (bus->members[i].f->writeb == NULL) {
			continue;
		}

		ret = bus->members[i].f->writeb(bus->members[i].s, addr, val);
		if (ret) {
			break;
		}
	}

	#ifdef DEBUG_PRINT_ERRORS
		if(!ret){
			fprintf(stderr, "No device on bus for address (%#d) in write\n", addr);
		}
	#endif

}

uint8_t
sig_host_bus_readb(
	const struct sig_host_bus *bus,
	void *s,
	uint32_t addr
)
{
	uint8_t ret = 0xFF;
	unsigned int i;
	bool r;

	for (i = 0; i < bus->nmembers; i++) {
		if (bus->members[i].s == s) {
			continue;
		}

		if (bus->members[i].f == NULL) {
			continue;
		}

		if (bus->members[i].f->readb == NULL) {
			continue;
		}

		r = bus->members[i].f->readb(bus->members[i].s, addr, &ret);
		if (r) {
			break;
		}
	}

	#ifdef DEBUG_PRINT_ERRORS
		if(!r){
			fprintf(stderr, "No device on bus for address (%#d) in read\n", addr);
		}
	#endif

	return ret;
}

uint8_t
sig_host_bus_inb(
	const struct sig_host_bus *bus,
	void *s,
	uint32_t addr
)
{
	uint8_t ret = 0xFF;
	unsigned int i;
	bool r;

	for (i = 0; i < bus->nmembers; i++) {
		if (bus->members[i].s == s) {
			continue;
		}

		if (bus->members[i].f == NULL) {
			continue;
		}

		if (bus->members[i].f->readb == NULL) {
			continue;
		}

		r = bus->members[i].f->readb(bus->members[i].s, addr, &ret);
		if (r) {
			break;
		}
	}

	return ret;
}

void
sig_host_bus_outb(
	const struct sig_host_bus *bus,
	void *s,
	uint32_t addr,
	uint8_t val
)
{
	unsigned int i;
	bool ret;

	for (i = 0; i < bus->nmembers; i++) {
		if (bus->members[i].s == s) {
			continue;
		}

		if (bus->members[i].f == NULL) {
			continue;
		}

		if (bus->members[i].f->outb == NULL) {
			continue;
		}

		ret = bus->members[i].f->outb(bus->members[i].s, addr, val);
		if (ret) {
			break;
		}
	}

}

uint8_t
sig_host_bus_read_io_dev(
	const struct sig_host_bus *bus,
	void *s,
	uint32_t addr
)
{
	uint8_t ret = 0xFF;
	unsigned int i;
	bool r;

	for (i = 0; i < bus->nmembers; i++) {
		if (bus->members[i].s == s) {
			continue;
		}

		if (bus->members[i].f == NULL) {
			continue;
		}

		if (bus->members[i].f->read_from_io_dev == NULL) {
			continue;
		}

		r = bus->members[i].f->read_from_io_dev(bus->members[i].s, addr, &ret);
		if (r) {
			break;
		}
	}

	return ret;
}

void
sig_host_bus_write_io_dev(
	const struct sig_host_bus *bus,
	void *s,
	uint32_t addr,
	uint8_t val
)
{
	unsigned int i;
	bool ret;

	for (i = 0; i < bus->nmembers; i++) {
		if (bus->members[i].s == s) {
			continue;
		}

		if (bus->members[i].f == NULL) {
			continue;
		}

		if (bus->members[i].f->write_to_io_dev == NULL) {
			continue;
		}

		ret = bus->members[i].f->write_to_io_dev(bus->members[i].s, addr, val);
		if (ret) {
			break;
		}
	}
}
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
