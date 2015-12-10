/* $Id: setup_serial_irq.S 10001 2014-10-15 14:00:05Z raimue $
 *
 * Setup the serial controller and handle interrupts.
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */

.text

.globl setup_controller
setup_controller:
	/* nothing to do */
	ret

.globl release_comp_irq
release_comp_irq:
	/* this will (on purpose) clobber %al and others... of course a real
	 * interrupt handler mustn't do this!
	 */
	mov 0xD800, %al
	call libs_printc
	ret
