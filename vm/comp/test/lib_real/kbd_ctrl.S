/* $Id: kbd_ctrl.S 10001 2014-10-15 14:00:05Z raimue $
 *
 * Setup the keyboard controller (connected to irq1) so that it produces
 * interrupts.
 *
 * Copyright (C) 2009 FAUmachine Team <info@faumachine.org>.
 * This program is free software. You can redistribute it and/or modify it
 * under the terms of the GNU General Public License, either version 2 of
 * the License, or (at your option) any later version. See COPYING.
 */

.text

/* keyboard controller is irq1
 * io: 0x0060, 0x0064
 */

wait_controller_input_empty:
	inb $0x0064, %al
	andb $0x02, %al
	jnz wait_controller_input_empty
	ret

wait_controller_output_full:
	inb $0x0064, %al
	andb $0x01, %al
	jz wait_controller_output_full
	ret

.globl setup_controller
setup_controller:
	movl $str_begin, %edx
	call libs_puts
	/* setup for read mode */
	movb $0x20, %al
	outb %al, $0x0064
	call wait_controller_output_full

	/* read old mode into al */
	inb $0x0060, %al

	/* setup for write mode (and save old mode in ah) */
	movb %al, %ah
	movb $0x60, %al
	outb %al, $0x0064
	call wait_controller_input_empty

	/* enable interrupts */
	orb $0x01, %ah
	movb %ah, %al
	outb %al, $0x0060
	call wait_controller_input_empty

	/* all done */
	movl $str_ready, %edx
	call libs_puts

	ret

.globl release_comp_irq
release_comp_irq:
	pusha

	/* fetch status... s.th. in output buffer? */
	inb $0x0064, %al
	andb $0x01, %al
	jz release_comp_irq_out

	xorl %eax, %eax
	inb $0x0060, %al
	/* release code? */
	testb $0x80, %al
	jnz release_comp_irq_out

	/* perform table lookup */
	movb kbd_lc(,%eax,1), %al
	movb %al, kbd_key_read
	call libs_printc

release_comp_irq_out:
	popa
	ret

.section .data
/* the last read key will be stored here.
 * the os is free to overwrite this in the case of need.
 */
.globl kbd_key_read
kbd_key_read:
	.byte 0

.section .rodata
str_ready:
	.ascii "Keyboard controller programmed.\n\0"
str_begin:
	.ascii "Programming Keyboard controller.\n\0"


/* normal lookup table */
kbd_lc:
/* 00 */	.byte 0, 0x1b, '1', '2'
/* 04 */	.byte '3', '4', '5', '6'
/* 08 */	.byte '7', '8', '9', '0'
/* 0c */	.byte '-', '=', '\b', '\t'
/* 10 */	.byte 'q', 'w', 'e', 'r'
/* 14 */	.byte 't', 'y', 'u', 'i'
/* 18 */	.byte 'o', 'p', '[', ']'
/* 1c */	.byte '\n', -1, 'a', 's'
/* 20 */	.byte 'd', 'f', 'g', 'h'
/* 24 */	.byte 'j', 'k', 'l', ';'
/* 28 */	.byte '\'', '`', -1, '\\'
/* 2c */	.byte 'z', 'x', 'c', 'v'
/* 30 */	.byte 'b', 'n', 'm', ','
/* 34 */	.byte '.', '/', -1, '*'
/* 38 */	.byte -1, ' ', 0, 0
/* 3c */	.byte 0, 0, 0, 0
/* 40 */	.byte 0, 0, 0, 0
/* 44 */	.byte 0, 0, 0, 0
/* 48 */	.byte 0, 0, '-', 0
/* 4c */	.byte 0, 0, '+', 0
/* 50 */	.byte 0, 0, 0, 0
