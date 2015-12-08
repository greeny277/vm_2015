/* $Id: sig_boolean.c 10341 2014-11-12 15:38:08Z raimue $
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */

#include "sig_boolean.h"
#include "stdlib.h"
#include "assert.h"

struct
sig_boolean *sig_boolean_create(void)
{
	struct sig_boolean *ret = malloc(sizeof(struct sig_boolean));
	assert(ret != NULL);

	ret->nmembers = 0;
	ret->last_value = false;
	return ret;
}


void
sig_boolean_destroy(struct sig_boolean *bus)
{
	free(bus);
}

void
sig_boolean_connect(
	struct sig_boolean *bus,
	void *s,
	const struct sig_boolean_funcs *f
)
{
	assert(bus != NULL);
	assert(s != NULL);
	assert(bus->nmembers
		< sizeof(bus->members) / sizeof(bus->members[0]));

	bus->members[bus->nmembers].s = s;
	bus->members[bus->nmembers].f = f;
	bus->nmembers++;
}

void
sig_boolean_set(struct sig_boolean *bus, void *s, bool val)
{
	unsigned int i;

	if (bus->last_value == val) {
		return;
	}

	bus->last_value = val;

	for (i = 0; i < bus->nmembers; i++) {
		if (bus->members[i].s == s) {
			continue;
		}

		if (bus->members[i].f == NULL) {
			continue;
		}

		bus->members[i].f->set(bus->members[i].s, val);
	}
}

void
sig_boolean_raise(struct sig_boolean *bus, void *s)
{
	sig_boolean_set(bus, s, true);
}

void
sig_boolean_clear(struct sig_boolean *bus, void *s)
{
	sig_boolean_set(bus, s, false);
}
