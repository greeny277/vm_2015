/* $Id: glue-io.h 10341 2014-11-12 15:38:08Z raimue $ */

#ifndef __GLUE_IO_H_INCLUDED
#define __GLUE_IO_H_INCLUDED

/** callbacks for io functions */
struct glue_io_funcs {
	/** event happened on file descriptor fd
	 *  @param s callee instance.
	 */
	void (*event)(void *s);
};

/** connect a callback to a file descriptor, so that the callback will be
 *  called whenever an event (sigio) happens on the file descriptor.
 *  @param s instance of caller.
 *  @param fd file descriptor
 *  @param f callback functions.
 */
extern void
glue_io_connect(void *s, int fd, const struct glue_io_funcs *f);

/** initialize and create io glue */
extern void
glue_io_create(void);

/** destroy io glue */
extern void
glue_io_destroy(void);

/** perform one io "step", call this at each iteration step to
 *  check for pending io on file descriptors and call the corresponding
 *  callback functions.
 */
extern void
glue_io_step(void);

#endif /* __IO_H_INCLUDED */
