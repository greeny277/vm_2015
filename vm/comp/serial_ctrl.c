/* $Id: serial_ctrl.c 10341 2014-11-12 15:38:08Z raimue $
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3 
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */

#include "serial_ctrl.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include "../utils/glue-io.h"
#include <string.h>
#include <errno.h>
#include <sched.h>

#define SERIAL_MEM_BASE 0xD800
#define SERIAL_MEM_MASK 0x000F

/* if defined, set the terminal to raw input. Normally, characters are
 * only recieved after a newline. In raw mode, characters will be recieved
 * as soon as these are entered. Additionally raw mode disables echoing
 * the input.
 */
#define CHARACTER_BASED_READING 1

/** backup termios struct only useful to restore termios on 
 *  SIGABRT, or on quit.
 */
#ifdef CHARACTER_BASED_READING
static struct termios _backup_ti;
#endif

struct cpssp {
	/** ports */
	struct sig_boolean *port_int;

	/** state */
	/** recieve register */
	uint8_t reg_recieve;

	/** byte recieved? */
	bool recieve_full;
};

/* flush stdin to register */
static void
serial_ctrl_flush(struct cpssp *cpssp)
{
	ssize_t ret;
	char c;

	assert(! cpssp->recieve_full);
	
	ret = read(0, &c, 1);
	if (ret <= 0) {
		/* nothing to flush (or error). */
		return;
	}

	cpssp->recieve_full = true;
	cpssp->reg_recieve = (uint8_t)c;

	sig_boolean_set(cpssp->port_int, cpssp, true);
}

static bool
serial_ctrl_readb(void *_cpssp, uint32_t addr, uint8_t *valp)
{
	struct cpssp *cpssp = (struct cpssp*)_cpssp;

	if ((addr & ~SERIAL_MEM_MASK) != SERIAL_MEM_BASE) {
		return false;
	}
	
	/* currently hardwired to ground */
	switch (addr & SERIAL_MEM_MASK) {
	case 0x0: /* return register contents */
		*valp = cpssp->reg_recieve;

		/* unflag interrupt, reenable recieve logic */
		sig_boolean_set(cpssp->port_int, cpssp, false);
		cpssp->recieve_full = false;

		/* flush again, maybe there's more data to read. */
		serial_ctrl_flush(cpssp);
		break;

	default:
		/* hardwired to ground */
		*valp = 0;
		break;
	}

	return true;
}

static bool
serial_ctrl_writeb(void *_cpssp, uint32_t addr, uint8_t val)
{
	int ret;

	if ((addr & ~SERIAL_MEM_MASK) != SERIAL_MEM_BASE) {
		return false;
	}


	switch (addr & SERIAL_MEM_MASK) {
	case 0x0: /* write to stdout */
		while (true) {
			ret = write(1, &val, 1);
			if (ret != 1) {
				if (errno == EAGAIN) {
					sched_yield();
					continue;
				}
				assert(false);
			}
			break;
		}
			
		
		break;

	default: /* not defined */
		break;
	}

	return true;
}

#ifdef CHARACTER_BASED_READING
static void
serial_ctrl_sig_terminate(int signum)
{
	/* restore terminal */
	tcsetattr(0, TCSANOW, &_backup_ti);

	/* abort is caught by us, need to restore the default before
	 * calling abort, otherwise we'd land here again.
	 */
	signal(SIGABRT, SIG_DFL);
	abort();
}

static void
serial_ctrl_make_raw(void)
{
	struct termios ti;
	int ret;
	static bool been_here = false;

	/* only perform this *once* for stdin */
	if (been_here) {
		return;
	}
	been_here = true;

	ret = tcgetattr(0, &_backup_ti);

	/* set to raw ... */
	ret = tcgetattr(0, &ti);
	assert(ret != -1);
	cfmakeraw(&ti);
	/* but allow ctrl-c and others to produce the corresponding signal
	 */
	ti.c_iflag |= ICRNL;
	ti.c_lflag |= ISIG | ECHONL;
	ti.c_oflag |= OPOST;

	ret = tcsetattr(0, TCSANOW, &ti);
	assert(ret != -1);

	/* make sure to restore terminal on one of these signals */
	signal(SIGQUIT, serial_ctrl_sig_terminate);
	signal(SIGINT, serial_ctrl_sig_terminate);
	signal(SIGABRT, serial_ctrl_sig_terminate);
}
#endif

static void
serial_ctrl_stdin_event(void *_cpssp)
{
	struct cpssp *cpssp = (struct cpssp *)_cpssp;

	if (cpssp->recieve_full) {
		return;
	}
	
	serial_ctrl_flush(cpssp);
}

void *
serial_ctrl_create(
	struct sig_host_bus *port_host,
	struct sig_boolean *port_int
)
{
	struct cpssp *cpssp;
	static const struct sig_host_bus_funcs hf = {
		.readb = serial_ctrl_readb,
		.writeb = serial_ctrl_writeb
	};
	static const struct glue_io_funcs io_f = {
		.event = serial_ctrl_stdin_event
	};

	cpssp = malloc(sizeof(struct cpssp));
	assert(cpssp != NULL);

	cpssp->port_int = port_int;
	cpssp->reg_recieve = 0;
	cpssp->recieve_full = false;

#ifdef CHARACTER_BASED_READING
	/* setup stdin for raw reading */
	serial_ctrl_make_raw();
#endif

	/* connect to slave ports */
	sig_host_bus_connect(port_host, cpssp, &hf);
	/* listen to stdin events */
	/* FIXME: Uncomment line below when implementing serial_ctrl */
	//glue_io_connect(cpssp, 0, &io_f);

	return cpssp;
}

void 
serial_ctrl_destroy(void *_cpssp)
{
	free(_cpssp);
#ifdef CHARACTER_BASED_READING
	/* restore terminal */
	tcsetattr(0, TCSANOW, &_backup_ti);
#endif
}
