/* $Id: glue-io.c 10341 2014-11-12 15:38:08Z raimue $
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */

#include "glue-io.h"
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <sys/select.h>

/** sigio unprocessed? */
static bool sigio_pending = false;

/** connected io users */
static struct {
	/** callee instance */
	void *s;
	/** callback functions */
	const struct glue_io_funcs *f;
	/** registered file descriptor. */
	int fd;
} glue_io_members[10];

/** number of connected io users */
static int glue_io_nmembers = 0;

static void
glue_io_sigio(int signum)
{
	sigio_pending = true;
}

static void
glue_io_fd_async(int fd)
{
	int flags;
	int ret;

	flags = fcntl(fd, F_GETFL, 0);
	assert(0 <= flags);

	flags |= O_NONBLOCK | O_ASYNC;

	ret = fcntl(fd, F_SETFL, flags);
	assert(0 <= ret);

	ret = fcntl(fd, F_SETOWN, getpid());
	assert(0 <= ret);
}

void
glue_io_connect(void *s, int fd, const struct glue_io_funcs *f)
{
	assert(glue_io_nmembers
		< sizeof(glue_io_members) / sizeof(glue_io_members[0]));
	assert(s != NULL);
	assert(f != NULL);
	assert(fd >= 0);

	glue_io_members[glue_io_nmembers].s = s;
	glue_io_members[glue_io_nmembers].f = f;
	glue_io_members[glue_io_nmembers].fd = fd;

	glue_io_nmembers++;

	glue_io_fd_async(fd);
}

void
glue_io_create(void)
{
	struct sigaction saio;
	int ret;

	saio.sa_handler = glue_io_step;
	saio.sa_flags = SA_RESTART;
	sigemptyset(&saio.sa_mask);
	ret = sigaction(SIGIO, &saio, NULL);
	assert(ret == 0);
}

void
glue_io_destroy(void)
{
	glue_io_nmembers = 0;
}

void
glue_io_step(void)
{
	int i;
	fd_set fdr;
	struct timeval tv;
	int ret;
	int hfd = -1;

	if (! sigio_pending) {
		return;
	}

	sigio_pending = false;

	FD_ZERO(&fdr);

	/* find out fd number of event */
	for (i = 0; i < glue_io_nmembers; i++) {
		FD_SET(glue_io_members[i].fd, &fdr);

		if (glue_io_members[i].fd > hfd) {
			hfd = glue_io_members[i].fd;
		}
	}

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	ret = select(hfd + 1, &fdr, NULL, NULL, &tv);
	if (ret < 0) {
		return;
	}

	for (i = 0; i < glue_io_nmembers; i++) {
		if (FD_ISSET(glue_io_members[i].fd, &fdr)) {
			glue_io_members[i].f->event(glue_io_members[i].s);
		}
	}
}
