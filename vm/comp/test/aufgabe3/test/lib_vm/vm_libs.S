/* $Id: vm_libs.S 10001 2014-10-15 14:00:05Z raimue $ */

/* implementation of small system library, variant for the VM.
 * This should be linked to assembly test programs that
 * are designated to be run in the VM
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3 
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */

.text
.globl libs_printc
libs_printc:
	/* print a character on stdout */
	/* al: character to print */
	/* clobbered: (legacy for linux variant: eax, ebx, ecx, edx)
	              actually: None */

	movb %al, 0xd800
	ret

.globl libs_printb
libs_printb:
	/* print a byte on stdout */
	/* al: byte to print */
	/* clobbered: eax, ebx, ecx, edx */

	/* add '0' */
	addb $0x30, %al
	call libs_printc
	ret

/** print a string pointed to by edx
  * edx: address of 0-terminated string.
  * clobbered: many registers
  */
.globl libs_puts
libs_puts:
	xorl %edi, %edi
libs_puts_loop:
	movb (%edx, %edi, 1), %al
	cmpb $0, %al
	je libs_puts_done

	call libs_printc
	incl %edi
	jmp libs_puts_loop

libs_puts_done:
	ret

/** do (not) clear the screen. (not implemented for serial controller)
 */
.globl libs_clrscr
libs_clrscr:
	ret

.globl libs_exit
libs_exit:
	cli
	hlt

.globl pic_quirk
pic_quirk:
	/* hey, your PIC should be correct in the first place! :) */
	ret
