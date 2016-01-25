/*
 * $Id: boot-cdrom.S 10001 2014-10-15 14:00:05Z raimue $
 *
 * Copyright (C) 2006-2009 FAUmachine Team <info@faumachine.org>.
 * This program is free software. You can redistribute it and/or modify it
 * under the terms of the GNU General Public License, either version 2 of
 * the License, or (at your option) any later version. See COPYING.
 */
	.code16
	.text
_start:
	/* Far jump makes sure we canonicalize the address */
	cli
	ljmpw $0, $0x7c00 + 64

	.org 8
bi_pvd:
	.long 0xdeadbeef
bi_file:
	.long 0xdeadbeef
bi_length:
	.long 0xdeadbeef
bi_csum:
	.long 0xdeadbeef
bi_reserved:
	.long 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

	.org 64
_start1:
	movw $0, %ax

	/* Initialize stack. */
	movw %ax, %ss
	movw $0x7c00, %sp

	/* Initialize segments. */
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	cld

	/*
	 * Show first sign of life.
	 */
	movb $0x0e, %ah
	movb $'A', %al
	int $0x10

	/*
	 * Go protected mode.
	 */
	lgdt gdt_descr

	movl %cr0, %eax
	orl $1, %eax
	movl %eax, %cr0

	ljmpl $0x0008, $.L1

	.code32
.L1:	movw $0x0010, %ax
	movw %ax, %ss
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	/*
	 * Setup stack.
	 */
	movl $32*1024*1024, %esp

	/*
	 * Show next sign of life.
	 */
	movb $'B', 0xb8000
	movb $0x07, 0xb8001

	/* 
	 * Copy OS to 0x0000 (to have identical memory maps
	 * as the exercise VM)
	 */
	/* limit: (check size of boot-cdrom)
	 * must not exceed 0x7c00, that's where were currently
	 * running! (likewise in VM there's only ram up to 0x7fff. */
	movl $0x5000 / 8, %ecx
.Lcopy_kernal:
	movq 0x8c00(,%ecx,8), %mm0
	movq %mm0, (,%ecx,8)
	decl %ecx
	jge .Lcopy_kernal

	/* jump right to main, which should be at 0x0000 */
	ljmpl $0x08, $0x0000
.L2:	jmp .L2

	
gdt:
	/* NULL descriptor */
	.long 0x00000000, 0x00000000

	/* Code segment */
	/* Base 0x0, Limit 0xfffff, 32-bit */
	.long 0x0000ffff, 0x004f9b00

	/* Data segment */
	/* Base 0x0, Limit 0xfffff, page-granularity */
	.long 0x0000ffff, 0x00cf9300
gdt_end:
	
gdt_descr:
	.word (gdt_end - gdt) - 1
	.long gdt
	.word 0x0000
