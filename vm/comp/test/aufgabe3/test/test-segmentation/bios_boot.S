/* $Id: bios_boot.S 10001 2014-10-15 14:00:05Z raimue $ */

/* bios that copies the first 64 blocks from the hard disk to 
 * RAM and then ljmps to address 0.
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3 
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */


_start:
	cli

	lgdt bios_gdtd
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	movl $0x18, %eax
	mov %ax, %ss
	movl $0x7FFF, %esp
	
	ljmpl $0x08, $_start1

_start1:
	xorl %eax, %eax
	xorl %edx, %edx
load_loop:
	call load_block
	call copy_block
	incl %eax
	add $512, %edx
	cmpl $56, %eax
	jb load_loop

	ljmp $0x08, $0


/** load block specified with %al to byte buffer of disk_ctrl */
load_block:
	/* al: lowest byte of block to load */
	movb $0, 0xd000
	movb $0, 0xd001
	movb $0, 0xd002
	movb %al, 0xd003

	/* read command */
	movb $0, 0xd00b
	ret

/** copy block from disk_ctrl byte buffer to memory with base edx.
 *  clobbered: ecx.
 */
copy_block:
	/* edx: destination address */
	xorl %ecx, %ecx
copy_loop:
	movb 0xd200(%ecx), %bl
	movb %bl, (%edx, %ecx, 1)
	incl %ecx
	cmpl $512, %ecx
	jne copy_loop
	ret

.section .rodata
bios_gdt:
	/* NULL entry */
	.long 0, 0
	/* code segment, base 0, limit 0xEFFF */
	.word 0x000F /* limit 15-0 */
	.word 0x0000 /* base 15-0 */
	.byte 0 /* base 23-16 */
	.byte 0b10011011
	.byte 0b11000000
	.byte 0 /* base 31-24 */

	/* data segment, base 0, limit 0xEFFF */
	.word 0x000F /* limit 15-0 */
	.word 0x0000 /* base 15-0 */
	.byte 0 /* base 23-16 */
	.byte 0b10010011
	.byte 0b11000000
	.byte 0 /* base 31-24 */

	/* stack segment, base 0, limit 0x7FFF */
	.word 0x0008 /* limit 15-0 */
	.word 0x0000 /* base 15-0 */
	.byte 0 /* base 23-16 */
	.byte 0b10010011
	.byte 0b11000000
	.byte 0 /* base 31-24 */
bios_gdt_end:

bios_gdtd:
	.word (bios_gdt_end - bios_gdt) - 1
	.long bios_gdt
	.word 0x0000
