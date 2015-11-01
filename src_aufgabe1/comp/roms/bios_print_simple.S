/* $Id: bios_print_simple.S 10001 2014-10-15 14:00:05Z raimue $ */

/* bios that prints "hallo welt\n" on serial console and then performs a
 * hlt instruction.
 * This variant is very simple and only uses direct memory reads/writes.
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3 
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */

_start:
	/* h */
	movb txt, %al
	movb %al, 0xd800
	/* a */
	movb txt + 1, %al
	movb %al, 0xd800
	/* l */
	movb txt + 2, %al
	movb %al, 0xd800
	/* l */
	movb txt + 3, %al
	movb %al, 0xd800
	/* o */
	movb txt + 4, %al
	movb %al, 0xd800
	/* " " */
	movb txt + 5, %al
	movb %al, 0xd800
	/* w */
	movb txt + 6, %al
	movb %al, 0xd800
	/* e */
	movb txt + 7, %al
	movb %al, 0xd800
	/* l */
	movb txt + 8, %al
	movb %al, 0xd800
	/* t */
	movb txt + 9, %al
	movb %al, 0xd800
	/* "\n" */
	movb txt + 10, %al
	movb %al, 0xd800
	/* end of test. */
	hlt

txt:
.ascii "hallo welt\n"
