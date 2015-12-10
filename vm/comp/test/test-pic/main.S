/* $Id: main.S 10352 2014-11-13 10:45:02Z raimue $
 *
 * vim: filetype=ia64:
 *
 * Copyright (C) 2009 FAUmachine Team <info@faumachine.org>.
 * This program is free software. You can redistribute it and/or modify it
 * under the terms of the GNU General Public License, either version 2 of
 * the License, or (at your option) any later version. See COPYING.
 */

.text
.globl main
main:
	cli
	call setup_idt
	call setup_pic0_single_autoeoi
	call setup_controller
	sti

main_loop:
	hlt
	jmp main_loop

/* program pic0 to 8086 mode, autoeoi, single */
setup_pic0_single_autoeoi:
	/* icw1: edge triggered, single, ic4 present */
	movb $0x13, %al
	outb %al, $0x0020

	/* icw2: base address 32 */
	movb $0x20, %al
	outb %al, $0x0021

	/* no icw3 (single) */
	/* FIXME (pic in FAUmachine) 
	xorb %al, %al
	outb %al, $0x0021 */

	/* icw4: autoeoi, 8086 mode */
	movb $0x03, %al
	outb %al, $0x0021

	/* program interrupt mask, disable all ints apart from 1 */
	movb $0xfd, %al
	outb %al, $0x0021
	ret

setup_idt:
	lidt idt_descr

	movl $str_idt_setup, %edx
	call libs_puts

	ret

/* ******************* interrupt/exception handlers  ******************
 * Attention: due to offsets begin split in IDT, the IDT (in the .data
 * section) must be close enough to the exception handlers in the .text
 * segment, since these are only using offset 15..0 for relocation.
 * However the linker seems to do the right thing generating
 * R_386_16 relocation entries, so it might not link if the sections
 * would be too far away from each other.
 * ******************************************************************** */

/* exception handler for all unhandled exceptions */
unhandled_exception:
	movl $str_unhandled, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit

	iret

double_fault:
	movl $str_double_fault, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit

	iret

segment_not_present_fault:
	movl $str_seg_not_present, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit

	iret

stack_segment_fault:
	movl $str_stack_segment_fault, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit

	iret

general_protection_fault:
	movl $str_gp, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit

	iret


page_fault:
	movl $str_page_fault, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit

	iret


/* exception handler for division by 0 */
handle_div0:
	movl $str_div0, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit
	iret

irq0_handler:
	movl $str_irq0, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit
	iret

irq1_handler:
	/* enable for debug output....
	movl $str_irq1, %edx
	call libs_puts
	*/

	call release_comp_irq
	iret

irq2_handler:
	movl $str_irq2, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit
	iret

irq3_handler:
	movl $str_irq3, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit
	iret

irq4_handler:
	movl $str_irq4, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit
	iret

irq5_handler:
	movl $str_irq5, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit
	iret

irq6_handler:
	movl $str_irq6, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit
	iret

irq7_handler:
	movl $str_irq7, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit
	iret

.section .rodata
idt:
	/* generic format traps/interrupts:
	 * (we're little endian!)
         *
	 * l1: 
	 * 	15-0: offset 15-0.
	 * 	31-16: segment selector
	 * l0: 
	 * 	7-5: 000, 4-0: reserved (use 0 here)
	 *      15-8:
	 *      	15: valid?, 
	 * 		14+13: dpl, 
	 *		12-9: 0D11 (D=1 -> 32 bit, 0 -> 16 bit)
	 * 		8: leave IF enabled? (1)
	 * 
	 * 	31-16: offset 31-16
	 */

	/* 0: divide error (Fault) */
	.word handle_div0
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 1: Debug (Fault/Trap) */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 2: NMI (Interrupt) */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xee /* 1110 1110 */
	.word 0

	/* 3: Breakpoint (Trap) */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 4: Overflow (Trap) */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 5 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 6 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 7 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 8 double fault */
	.word double_fault
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 9 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 10 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 11 */
	.word segment_not_present_fault
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 12 */
	.word stack_segment_fault
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 13 */
	.word general_protection_fault
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 14 */
	.word page_fault
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 15 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 16 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 17 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 18 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 19 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 20 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 21 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 22 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 23 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 24 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 25 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 26 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 27 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 28 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 29 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 30 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* 31 */
	.word unhandled_exception
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xef /* 1110 1111 */
	.word 0

	/* ********** user defined ********* */

	/* 32: pic0, irq 0 */
	.word irq0_handler
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xee /* 1110 1110 */
	.word 0

	/* 33: pic0, irq 1 */
	.word irq1_handler
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xee /* 1110 1110 */
	.word 0

	/* 34: pic0, irq 2 */
	.word irq2_handler
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xee /* 1110 1110 */
	.word 0

	/* 35: pic0, irq 3 */
	.word irq3_handler
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xee /* 1110 1110 */
	.word 0

	/* 36: pic0, irq 4 */
	.word irq4_handler
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xee /* 1110 1110 */
	.word 0

	/* 37: pic0, irq 5 */
	.word irq5_handler
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xee /* 1110 1110 */
	.word 0

	/* 38: pic0, irq 6 */
	.word irq6_handler
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xee /* 1110 1110 */
	.word 0

	/* 39: pic0, irq 7 */
	.word irq7_handler
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0xee /* 1110 1110 */
	.word 0

	/* sentinel */
	.long 0, 0
idt_end:

idt_descr:
	.word (idt_end - idt) - 1
	.long idt
	.word 0x0000

str_div0:
	.ascii "Division by Zero\n\0"

str_idt_setup:
	.ascii "IDT setup.\n\0"

str_unhandled:
	.ascii "Unhandled Exception/Interrupt\n\0"

str_double_fault:
	.ascii "Double Fault\n\0"

str_seg_not_present:
	.ascii "Segment not Present\n\0"

str_page_fault:
	.ascii "Page Fault\n\0"

str_stack_segment_fault:
	.ascii "Stack Segment Fault\n\0"

str_gp:
	.ascii "General Protection Fault\n\0"

str_irq0:
	.ascii "Interrupt 0\n\0"

str_irq1:
	.ascii "Interrupt 1\n\0"

str_irq2:
	.ascii "Interrupt 2\n\0"

str_irq3:
	.ascii "Interrupt 3\n\0"

str_irq4:
	.ascii "Interrupt 4\n\0"

str_irq5:
	.ascii "Interrupt 5\n\0"

str_irq6:
	.ascii "Interrupt 6\n\0"

str_irq7:
	.ascii "Interrupt 7\n\0"
