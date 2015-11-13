/* $Id: bios_boot.S 10001 2014-10-15 14:00:05Z raimue $ */

/* bios that load block 0 from disk simulator, copies it to
 * 0x1000 and performs an absolute long jump to it.
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3 
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */


_start:
	/* load block 0 from disk */
	xorl %eax, %eax
	/* set block number */
	movb %al, 0xd000
	movb %al, 0xd001
	movb %al, 0xd002
	movb %al, 0xd003
	/* issue disk read command */
	movb %al, 0xd00b

	/* copy boot sector to RAM */
	xorl %ecx, %ecx

copy_loop:
	movb 0xd200(%ecx), %al
	movb %al, 0x1000(%ecx)
	incl %ecx
	cmpl $512, %ecx
	jb copy_loop

	/* block copied, initialize esp */
	movl $0x8000, %esp
	/* jmp right to code */
	ljmp $0,$0x1000
