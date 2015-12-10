/* $Id: linux_libs.S 10001 2014-10-15 14:00:05Z raimue $ *
 *
 * vim: filetype=ia64:
 *
 * implementation of small system library, variant for linux
 * This can be used to link assembly test programs that can be executed
 * on a linux host system 
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
	/* clobbered: eax, ebx, ecx, edx */

	/* put character in byte buffer */
	movb %al, byte_buf
	/* syscall write has number 4 */
	movl $4, %eax
	/* ebx: use 1 (stdout) to print to */
	movl $1, %ebx
	/* ecx: address of byte buffer */
	movl $byte_buf, %ecx
	/* print exactly one 1 byte */
	movl $1, %edx
	/* do syscall */
	int $0x80
	/* done, ignore return value for now */
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

.globl libs_exit
libs_exit:
	/* terminate execution, may also get jumped to */
	movl $1, %eax
	xorl %ebx, %ebx
	int $0x80

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
	
	pushl %edx
	call libs_printc
	popl %edx
	incl %edi
	jmp libs_puts_loop

libs_puts_done:
	ret


.data
.local byte_buf
byte_buf:
	.byte 0
