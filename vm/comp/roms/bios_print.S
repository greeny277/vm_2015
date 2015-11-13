/* $Id: bios_print.S 10001 2014-10-15 14:00:05Z raimue $ */

/* bios that prints "hallo welt\n" on serial console and then performs a 
 * hlt instruction.
 * This variant uses register-indirect addressing.
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3 
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */

_start:
	movl $txt, %ecx

print_loop:
	/* fetch byte from "txt" */
	movb (%ecx), %al

	/* last byte reached? */
	cmp $0, %al
	je end_loop

	/* output byte to serial ctrler */
	movb %al, 0xd800
	inc %ecx
	jmp print_loop

end_loop:
	/* end of test. */
	hlt

txt:
.ascii "hallo welt\n\0"
