/* $Id: sig_boolean.h 9992 2014-10-15 11:42:43Z raimue $ */

#ifndef __SIG_BOOLEAN_H_INCLUDED
#define __SIG_BOOLEAN_H_INCLUDED

#include <stdbool.h>

struct sig_boolean_funcs {
	/** callback for set new value. Will only get called if signal
	 *  value changed.
	 *  @param s object pointer of callee
	 *  @param val new value of boolean signal
	 */
	void (*set)(void *s, bool val);
};

struct sig_boolean {
	/** number of registered members */
	unsigned int nmembers;
	struct {
		/** object of registered member */
		void *s;
		/** callback functions for registered member */
		const struct sig_boolean_funcs *f;
	} members[8];

	/** last bus value */
	bool last_value;
};

/** create sig_boolean
 *  @return newly allocated sig_boolean instance.
 */
extern struct sig_boolean *
sig_boolean_create(void);


/** destroy a sig_boolean instance.
 *  @param sig_boolean instance.
 */
extern void
sig_boolean_destroy(struct sig_boolean *bus);

/** connect to a sig_boolean bus.
 *  @param bus sig_boolean bus object.
 *  @param s object pointer
 *  @param f callback functions.
 */
extern void
sig_boolean_connect(
	struct sig_boolean *bus,
	void *s,
	const struct sig_boolean_funcs *f
);

/** set a new value on a boolean bus.
 *  @param bus bus instance.
 *  @param s sender object.
 *  @param val new value to set.
 */
extern void
sig_boolean_set(
	struct sig_boolean *bus,
	void *s,
	bool val
);

#endif /* __SIG_BOOLEAN_H_INCLUDED */
