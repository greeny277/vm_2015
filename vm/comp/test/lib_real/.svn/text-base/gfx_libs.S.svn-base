/* $Id: gfx_libs.S 10001 2014-10-15 14:00:05Z raimue $ *
 *
 * vim: filetype=ia64:
 *
 * implementation of small system library, variant for direct graphic card
 * This can be used to link assembly test programs that can be executed
 * on a real system, or in a VM.
 *
 * Copyright (C) 2009 FAUmachine Team <info@faumachine.org>.
 * This program is free software. You can redistribute it and/or modify it
 * under the terms of the GNU General Public License, either version 2 of
 * the License, or (at your option) any later version. See COPYING.
 */

.text

/* set cursor position by cursor_x/cursor_y. */
.local set_cursor
set_cursor:
	pushl %edx
	movb $0x0f, %al
	movl $0x03d4, %edx
	outb %al, %dx
	movl cursor_y, %eax
	movb $80, %dl
	mulb %dl
	addl cursor_x, %eax
	movl $0x03d5, %edx
	outb %al, %dx
	decl %edx
	movb $0x0e, %al
	outb %al, %dx
	incl %edx
	movb %ah, %al
	outb %al, %dx
	popl %edx
	ret

/* scroll down one line
 * clobbered: ecx, ebx, mm0
 */
.local libs_scroll
libs_scroll:
	xorl %ecx, %ecx
libs_scroll_loop:
	movq 0xb80a0(,%ecx, 8), %mm0
	movq %mm0, 0xb8000(,%ecx, 8)
	incl %ecx
	cmpl $480, %ecx
	jb libs_scroll_loop

	/* clear last line */
	movl $39, %ecx
libs_scroll_clear_loop:
	movl $0x07200720, 0xb8f00(, %ecx, 4)
	decl %ecx
	jns libs_scroll_clear_loop
	ret


.globl libs_printc
libs_printc:
	/* print a character on screen */
	/* al: character to print */
	/* clobbered: eax, ebx, ecx */

	/* newline? */
	cmpb $0x0a, %al
	je _libs_printc_nl

	/* calculate offset:
	   0xb8000 + (y * 80 * 2) + (x * 2) */
	/* y * 160 = (y << 5) * 5 
	   = (y << 5) + (y << 5) + (y << 5) + (y << 5) + (y << 5)
	 */
	movl cursor_y, %ecx
	shll $5, %ecx
	movl %ecx, %ebx
	addl %ebx, %ecx
	addl %ebx, %ecx
	addl %ebx, %ecx
	addl %ebx, %ecx

	movl cursor_x, %ebx
	movb %al, 0xb8000(%ecx, %ebx, 2)
	movb $0x07, 0xb8001(%ecx, %ebx, 2)

	incl cursor_x
	cmpl $80, cursor_x
	jb _libs_printc_out

_libs_printc_nl:
	incl cursor_y
	movl $0, cursor_x

	/* need to scroll down? */
	cmpl $25, cursor_y
	jb _libs_printc_out
	decl cursor_y
	call libs_scroll

_libs_printc_out:
	call set_cursor
	ret

.globl libs_printb
libs_printb:
	/* print a byte (hex) on stdout */
	/* al: byte to print */
	/* clobbered: eax, ebx, ecx, edx */

	/* add '0' */
	addb $'0', %al
	cmpb $'9' + 1, %al
	jb libs_printb_write
	addb $'A' - '0' - 10, %al
libs_printb_write:
	call libs_printc
	ret



.globl libs_exit
libs_exit:
	cli
	movl $str_halting, %edx
	call libs_puts
	hlt

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

/** clear the screen.
 *  clobbered: None */
.globl libs_clrscr
libs_clrscr:
	pushl %ecx
	movl $80 * 25 * 4 - 4, %ecx

clrscr_loop:
	movl $0x07200720, 0xb8000(, %ecx, 4)
	decl %ecx
	jge clrscr_loop

	popl %ecx
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

.data
.local cursor_x
cursor_x:
	.long 0
.local cursor_y
cursor_y:
	.long 0

.section .rodata
.local str_halting
str_halting:
.asciz "****** Halting machine. ******"
