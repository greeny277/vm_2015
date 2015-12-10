/* $Id: kbd_ctrl.S 10001 2014-10-15 14:00:05Z raimue $
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
	ret

.globl release_comp_irq
release_comp_irq:
	pushl %eax
	movb 0xD800, %al
	movb %al, kbd_key_read
	popl %eax
	ret

.data
.globl kbd_key_read
kbd_key_read:
	.byte 0
