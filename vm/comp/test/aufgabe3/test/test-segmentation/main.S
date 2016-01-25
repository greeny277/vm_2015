/* $Id: main.S 10603 2014-12-08 10:17:05Z raimue $
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
	/* initialize segmentation */
	lgdt gdt_descriptor
	movl $0x10, %eax
	/* load data descriptors */
	mov %ax, %ss
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	/* initialize esp */
	movl $0x7FFF, %esp
	/* load code descriptor */
	ljmpl $0x08, $main_segmentation_setup

main_segmentation_setup:
	call libs_clrscr
	call setup_idt
	call setup_pic0_single_autoeoi
	call setup_controller
	sti

main_loop_entry:
	/* main_loop_entry serves as generic entry point, e.g. after a
	 * process was aborted due to gp.
	 *
	 * Hence any register may have an arbitrary value.
	 * The only guarantee is that the stack segment and
	 * code segment are correctly (ss -> double fault, can't 
	 * recover. cs -> iret)
	 */
	movl $0x0010, %eax
	cli
	/* sanitize stack (there must be no stack in use at this point) */
	mov %ax, %ss
	movl $0x7FFF, %esp

	/* reinitialize ds, es, fs, gs with system defaults. */
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	sti

	movl $str_menu, %edx
	call libs_puts

main_loop:
	hlt
	/* interrupted in main_loop, check what key was read */
	/* FIXME interface is very ugly! */
	cli
	movb kbd_key_read, %al
	movb $0, kbd_key_read
	sti

	/* no byte read? */
	cmpb $0, %al
	je main_loop

	/* start process 1? */
	cmpb $'1', %al
	je launch_process_1

	/* start process 2? */
	cmpb $'2', %al
	je launch_process_2

	/* start process 3? */
	cmpb $'3', %al
	je launch_process_3

	/* use data as code segment (GP) */
	cmpb $'4', %al
	je use_data_as_code_segment

	/* test int 80 */
	cmpb $'t', %al
	je test_int80

	/* TODO */
	jmp main_loop

test_int80:
	movb $'x', test_byte_buf
	movl $4, %eax
	movl $1, %ebx
	movl $test_byte_buf, %ecx
	movl $1, %edx
	int $0x80

	movb $'\n', test_byte_buf
	movl $4, %eax
	movl $1, %ebx
	movl $test_byte_buf, %ecx
	movl $1, %edx
	int $0x80
	jmp main_loop_entry


launch_process_1:
	/* setup data segments */
	movl $0x0018, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	/* setup code segment and run task */
	ljmp $0x0020, $0x0000

	/* not reached (ideally!) */
	cli
	hlt

launch_process_2:
	/* setup data segments */
	movl $0x0028, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	/* setup code segment and run task */
	ljmp $0x0030, $0x0000

	/* not reached (ideally!) */
	cli
	hlt

launch_process_3:
	/* setup data segments */
	movl $0x0038, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	/* setup code segment and run task */
	ljmp $0x0040, $0x0000

	/* not reached (ideally!) */
	cli
	hlt

use_data_as_code_segment:
	ljmp $0x0028, $0x0000
	/* unreached */
	cli
	hlt

/* program pic0 to 8086 mode, autoeoi, single */
setup_pic0_single_autoeoi:
	/* icw1: edge triggered, single, ic4 present */
	movb $0x13, %al
	outb %al, $0x0020

	/* icw2: base address 32 */
	movb $0x20, %al
	outb %al, $0x0021

	/* no icw3 (single) */
	/* FIXME (uncomment to make it work with pic in FAUmachine) 
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

/** return to the main loop. Stack must contain plain interrupt
 *  data and no additional entries. Data segments should have been 
 *  restored already.
 *
 *  This should get jumped to from interrupt handlers.
 */
iret_to_main_loop_entry:
	/* exit task by mangling stack */
	movl $main_loop_entry, (%esp)
	/* avoid this opcode with 16-Bit prefix... */
	/* movw $0x08, 4(%esp) */

	/* use this instead: before cs, there should be eflags, so it's safe 
	 * read a long from there. However make sure to not break eflags! */
	movl 4(%esp), %eax
	andl $0xFFFF0000, %eax
	addl $0x08, %eax
	movl %eax, 4(%esp)
	iret

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
	/* this won't get reached (double fault!) */
	movl $str_stack_segment_fault, %edx
	call libs_puts

	/* "abort" task (just stall the machine) */
	jmp libs_exit

	iret

general_protection_fault:
	/* restore data segment */
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	movl $str_gp, %edx
	call libs_puts
	movl $str_task_abort, %edx
	call libs_puts

	/* error code */
	/* FIXME display error code, needs "and, shrb" hence disabled 
	 *       for now. 
	popl %eax
	pushl %eax
	shrb $4, %al
	call libs_printb
	popl %eax
	andb $0xf, %al
	call libs_printb
	*/

	/* terminate process */
	jmp iret_to_main_loop_entry

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

irq1_kill_proc:
	/* clear out last read byte */
	movb $0, kbd_key_read
	/* remove 4 segments + eax from stack */
	addl $20, %esp
	/* back out to main loop */
	jmp iret_to_main_loop_entry

irq1_handler:
	/* restore os data segments */
	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs

	pushl %eax
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	call release_comp_irq
	/* check for 'k' to terminate process */
	cmpb $'k', kbd_key_read
	je irq1_kill_proc

	popl %eax
	pop %gs
	pop %fs
	pop %es
	pop %ds
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

/** exit the current process and return to
 *  main_loop_entry. Must be called from an interrupt/trap
 *  handler which has eip as top stack entry.
 */
os_call_exit:
	movl $0x10, %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	movl $str_process_exit, %edx
	call libs_puts

	/* exit task by mangling stack */
	jmp iret_to_main_loop_entry

/** mimic a very small portion of linux' int $0x80 */
os_call_handler:
	/* special handling: exit */
	cmpl $1, %eax
	je os_call_exit

	/* save segments */
	push %gs
	push %fs
	push %es
	push %ds
	
	/* use os data segments... stack segment mustn't change! */
	pushl %eax
	movl $0x10, %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	popl %eax

	cmpl $4, %eax
	je os_call_write
	cmpl $3, %eax
	je os_call_read

	/* unhandled system call */
	movl $str_unhandled_syscall, %edx
	call libs_puts
	/* return -1 */
	movl $-1, %eax

os_call_out:
	/* restore segments */
	pop %ds
	pop %es
	pop %fs
	pop %gs
	iret

os_call_write:
	/* only fd 1 (stdout) supported */
	cmpl $1, %ebx
	jne os_call_out

	/* FIXME only support printing one character so far */
	cmpl $1, %edx
	jne os_call_out

	/* need to fetch that character from process' segment. */
	pop %ds
	/* overwrite eax (that's our return register anyways) */
	movl (%ecx), %eax
	push %ds

	/* switch back to os segment (this is ugly, as I'm trying to avoid
	 * opcodes with segment prefixes) */
	pushl %eax
	movl $0x10, %eax
	movw %ax, %ds
	popl %eax
	
	/* w00t, so eax contains the character now, call libs_printc */
	pushl %ebx
	pushl %ecx
	call libs_printc
	popl %ecx
	popl %ebx

	/* return 1, since 1 character was written */
	movl $1, %eax
	jmp os_call_out

os_call_read:
	/* FIXME reads are not that easy! need real process handling 
	 * for that.
	 */
	jmp os_call_out


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

	/* 40: unused */
	.long 0, 0
	/* 41: unused */
	.long 0, 0
	/* 42: unused */
	.long 0, 0
	/* 43: unused */
	.long 0, 0
	/* 44: unused */
	.long 0, 0
	/* 45: unused */
	.long 0, 0
	/* 46: unused */
	.long 0, 0
	/* 47: unused */
	.long 0, 0
	/* 48: unused */
	.long 0, 0
	/* 49: unused */
	.long 0, 0
	/* 50: unused */
	.long 0, 0
	/* 51: unused */
	.long 0, 0
	/* 52: unused */
	.long 0, 0
	/* 53: unused */
	.long 0, 0
	/* 54: unused */
	.long 0, 0
	/* 55: unused */
	.long 0, 0
	/* 56: unused */
	.long 0, 0
	/* 57: unused */
	.long 0, 0
	/* 58: unused */
	.long 0, 0
	/* 59: unused */
	.long 0, 0
	/* 60: unused */
	.long 0, 0
	/* 61: unused */
	.long 0, 0
	/* 62: unused */
	.long 0, 0
	/* 63: unused */
	.long 0, 0
	/* 64: unused */
	.long 0, 0
	/* 65: unused */
	.long 0, 0
	/* 66: unused */
	.long 0, 0
	/* 67: unused */
	.long 0, 0
	/* 68: unused */
	.long 0, 0
	/* 69: unused */
	.long 0, 0
	/* 70: unused */
	.long 0, 0
	/* 71: unused */
	.long 0, 0
	/* 72: unused */
	.long 0, 0
	/* 73: unused */
	.long 0, 0
	/* 74: unused */
	.long 0, 0
	/* 75: unused */
	.long 0, 0
	/* 76: unused */
	.long 0, 0
	/* 77: unused */
	.long 0, 0
	/* 78: unused */
	.long 0, 0
	/* 79: unused */
	.long 0, 0
	/* 80: unused */
	.long 0, 0
	/* 81: unused */
	.long 0, 0
	/* 82: unused */
	.long 0, 0
	/* 83: unused */
	.long 0, 0
	/* 84: unused */
	.long 0, 0
	/* 85: unused */
	.long 0, 0
	/* 86: unused */
	.long 0, 0
	/* 87: unused */
	.long 0, 0
	/* 88: unused */
	.long 0, 0
	/* 89: unused */
	.long 0, 0
	/* 90: unused */
	.long 0, 0
	/* 91: unused */
	.long 0, 0
	/* 92: unused */
	.long 0, 0
	/* 93: unused */
	.long 0, 0
	/* 94: unused */
	.long 0, 0
	/* 95: unused */
	.long 0, 0
	/* 96: unused */
	.long 0, 0
	/* 97: unused */
	.long 0, 0
	/* 98: unused */
	.long 0, 0
	/* 99: unused */
	.long 0, 0
	/* 100: unused */
	.long 0, 0
	/* 101: unused */
	.long 0, 0
	/* 102: unused */
	.long 0, 0
	/* 103: unused */
	.long 0, 0
	/* 104: unused */
	.long 0, 0
	/* 105: unused */
	.long 0, 0
	/* 106: unused */
	.long 0, 0
	/* 107: unused */
	.long 0, 0
	/* 108: unused */
	.long 0, 0
	/* 109: unused */
	.long 0, 0
	/* 110: unused */
	.long 0, 0
	/* 111: unused */
	.long 0, 0
	/* 112: unused */
	.long 0, 0
	/* 113: unused */
	.long 0, 0
	/* 114: unused */
	.long 0, 0
	/* 115: unused */
	.long 0, 0
	/* 116: unused */
	.long 0, 0
	/* 117: unused */
	.long 0, 0
	/* 118: unused */
	.long 0, 0
	/* 119: unused */
	.long 0, 0
	/* 120: unused */
	.long 0, 0
	/* 121: unused */
	.long 0, 0
	/* 122: unused */
	.long 0, 0
	/* 123: unused */
	.long 0, 0
	/* 124: unused */
	.long 0, 0
	/* 125: unused */
	.long 0, 0
	/* 126: unused */
	.long 0, 0
	/* 127: unused */
	.long 0, 0
	/* 128: os call */
	.word os_call_handler
	.word 0x0008 /* segment 8 */
	.byte 0 /* constant 0/reserved */
	.byte 0b11101111
	.word 0
	/* 128: unused */
	.long 0, 0
idt_end:

idt_descr:
	.word (idt_end - idt) - 1
	.long idt
	.word 0x0000

str_div0:
	.asciz "Division by Zero\n"

str_idt_setup:
	.asciz "IDT setup.\n"

str_unhandled:
	.asciz "Unhandled Exception/Interrupt\n"

str_double_fault:
	.asciz "Double Fault\n"

str_seg_not_present:
	.asciz "Segment not Present\n"

str_page_fault:
	.asciz "Page Fault\n"

str_stack_segment_fault:
	.asciz "Stack Segment Fault\n"

str_gp:
	.asciz "General Protection Fault\n"

str_irq0:
	.asciz "Interrupt 0\n"

str_irq1:
	.asciz "Interrupt 1\n"

str_irq2:
	.asciz "Interrupt 2\n"

str_irq3:
	.asciz "Interrupt 3\n"

str_irq4:
	.asciz "Interrupt 4\n"

str_irq5:
	.asciz "Interrupt 5\n"

str_irq6:
	.asciz "Interrupt 6\n"

str_irq7:
	.asciz "Interrupt 7\n"

str_task_abort:
	.asciz "Aborting current task\n"

str_unhandled_syscall:
	.asciz "Unhandled system Call\n"
str_process_exit:
	.asciz "Syscall 1 called, exiting process.\n"

str_menu:
	.ascii "Main Menu - launch a single process\n"
	.ascii "1: Start process (1) (hello world)\n"
	.ascii "2: Start process (2) (arith.S)\n"
	.ascii "3: Start process (4) (accessing data beyond limit, GP)\n"
	.ascii "4: use Data segment as code segment (GP)\n"
	.ascii "t: Test int $0x80 (print x and newline)\n"
	.asciz "k: Kill running process, return here\n"

str_debug:
	.asciz "debug 1\n"

gdt:
	/* entries: 
	 * limit 7-0
	 * limit 15-8
	 * base 7-0
	 * base 15-8
	 * base 23-16
	 *    p/1:dpl/2:s/1:type/4
	 * 	-p: present
	 *	-dpl: descriptor privilege level
	 *	-s: descriptor type (0=system, 1=code or data)
	 *	-type: 
	 *		s=1
	 *			cs:e:w:a
	 *			cs:c:r:a
	 *
	 *			-cs: 0=data segment
	 *			-e: expand down
	 *			-w: write allowed
	 *			-a: accessed? (set by CPU on load of entry)
	 *			
	 *			-cs: 1=code segment
	 *			-c: conforming/non-conforming
	 *			    (conforming=dpl check not done)
	 *			-r: read allowed
	 *			-a: accessed? (set by CPU on load of entry)
	 *		s=0: available for tss, call gates, etc.
	 *
	 *    g/1:db/1:0/1:avl/1:limit 19-16/4
	 *      -g: granularity (0: limit=bytes, 1:limit=4KiB Blocks
	 *      -db: 1=32 Bit
	 *      -avl: no meaning in cpu
	 * base 31-24
	 */

	/* NULL descriptor */
	.long 0x00000000, 0x00000000

	/* "OS" code segment: base 0x0, limit 0x7FFF, 32-bit,
	 * page granularity */
	.word 0x0008 /* limit 15-0 */
	.word 0x0000 /* base 15-0 */
	.byte 0 /* base 23-16 */
	.byte 0b10011011
	.byte 0b11000000
	.byte 0 /* base 31-24 */

	/* "OS" data segment: base 0x0, limit 0xDFFF, 32-bit,
	 * page granularity */
	.word 0x000e /* limit 15-0 */
	.word 0x0000 /* base 15-0 */
	.byte 0 /* base 23-16 */
	.byte 0b10010011
	.byte 0b11000000
	.byte 0 /* base 31-24 */

	/* Data segment, process 1 */
	/* base:   loader base (0x0000)
	 *       + relative offset in file (0x3000)
	 *       - skipped file beginning of loader (0x1000)
	 * limit: 1 page.
	 */
	.word 0x01 /* limit 15-0 */
	.word 0x2000 /* base 15-0 */
	.byte 0 /* base 23-16 */
	.byte 0b10010011
	.byte 0b11000000
	.byte 0 /* base 31-24 */

	/* corresponding code segment */
	.word 0x01 /* limit 15-0 */
	.word 0x2000 /* base 15-0 */
	.byte 0 /* base 23-16 */
	.byte 0b10011011
	.byte 0b11000000
	.byte 0 /* base 31-24 */

	/* data segment arith.S */
	.word 0x01 /* limit 15-0 */
	.word 0x3000 /* base 15-0 */
	.byte 0x00 /* base 23-16 */
	.byte 0b10010011
	.byte 0b11000000
	.byte 0 /* base 31-24 */

	/* code segment arith.S */
	.word 0x01 /* limit 15-0 */
	.word 0x3000 /* base 15-0 */
	.byte 0x00 /* base 23-16 */
	.byte 0b10011011
	.byte 0b11000000
	.byte 0 /* base 31-24 */

	/* data segment gp.S */
	.word 0x01 /* limit 15-0 */
	.word 0x4000 /* base 15-0 */
	.byte 0x00 /* base 23-16 */
	.byte 0b10010011
	.byte 0b11000000
	.byte 0 /* base 31-24 */

	/* code segment gp.S */
	.word 0x01 /* limit 15-0 */
	.word 0x4000 /* base 15-0 */
	.byte 0x00 /* base 23-16 */
	.byte 0b10011011
	.byte 0b11000000
	.byte 0 /* base 31-24 */
gdt_end:
	
gdt_descriptor:
	.word (gdt_end - gdt) - 1
	.long gdt
	.word 0x0000

.data
test_byte_buf:
	.byte 00
