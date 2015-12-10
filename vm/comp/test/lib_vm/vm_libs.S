/* $Id: vm_libs.S 10001 2014-10-15 14:00:05Z raimue $ *
 *
 * vim: filetype=ia64:
 *
 * implementation of small system library, variant for the VM.
 * This should be linked to assembly test programs that
 * are designated to be run in the VM
 *
 * Copyright (C) 2009 FAUmachine Team <info@faumachine.org>.
 * This program is free software. You can redistribute it and/or modify it
 * under the terms of the GNU General Public License, either version 2 of
 * the License, or (at your option) any later version. See COPYING.
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
	addb $'0', %al
	cmpb $'9' + 1, %al
	jb libs_printb_write
	/* hex digit */
	addb $'a' - '0' - 10, %al
libs_printb_write:
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

/** print value in eax as hex number.
 *  Clobbered: see libs_printb, apart from
 *  eax.
 */
.globl libs_printl
libs_printl:
	pushl %eax

	shrl $32 - 8, %eax
	andl $0xFF, %eax
	call libs_printb_hex
	movl (%esp), %eax
	shrl $32 - 16, %eax
	andl $0xFF, %eax
	call libs_printb_hex
	movl (%esp), %eax
	shrl $32 - 24, %eax
	andl $0xFF, %eax
	call libs_printb_hex
	movl (%esp), %eax
	andl $0xFF, %eax
	call libs_printb_hex

	popl %eax
	ret


/** print value in al as hex number.
 *  Clobbered: see libs_printb, apart from
 *  eax.
 */
.globl libs_printb_hex
libs_printb_hex:
	pushl %eax

	shrb $4, %al
	call libs_printb
	movl (%esp), %eax
	andl $0xF, %eax
	call libs_printb

	popl %eax
	ret


.globl libs_exit
libs_exit:
	cli
	hlt
