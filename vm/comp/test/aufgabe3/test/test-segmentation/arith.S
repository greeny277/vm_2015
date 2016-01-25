/* $Id: arith.S 10001 2014-10-15 14:00:05Z raimue $ */

/* library to calculate with long number (pseudo-bcd like)
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3 
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */


.set NUM_DIGITS,500

.text
.globl _start
_start:
	movl $250, %ecx
	call arith_fak

	movl $_num_erg, %ebx
	call arith_print 
	jmp libs_exit


/* normalize a number 
   ebx: address of number 

   clobbered: eax, ecx
 */
.local arith_norm
arith_norm:
	xorl %ecx, %ecx
	xorl %eax, %eax

_arith_norm_loop:
	addb %al, (%ecx, %ebx)
	xorl %eax, %eax

_arith_norm_overflow_loop:
	cmpb $10, (%ecx, %ebx) 
	jb _arith_norm_loop_chk
	addb $1, %al
	addb $-10, (%ecx, %ebx)
	jmp _arith_norm_overflow_loop

_arith_norm_loop_chk:
	incl %ecx
	cmpl $NUM_DIGITS, %ecx
	jb _arith_norm_loop

	ret


/* add two numbers together.
   ebx: address of first number
   edx: address of second number
   result stored in first number

   clobbered: eax, ecx
*/
.globl arith_add
arith_add:
	xorl %ecx, %ecx

_add_loop:
	movb (%ecx, %ebx), %al
	addb (%ecx, %edx), %al
	movb %al, (%ecx, %ebx)

	incl %ecx
	cmpl $NUM_DIGITS, %ecx
	jb _add_loop

	call arith_norm
	ret

/* left shift one number
 * ebx address of source
 * edx address of dest
 * eax number of digits to left shift
 * clobbered: eax, edi, ecx
 */
.globl arith_shift_left
arith_shift_left:
	/* pad with zeros */
	xorl %ecx, %ecx
	xorl %edi, %edi
_shift_pre:
	cmpl %ecx, %eax
	je _shift_shift
	movb $0, (%ecx, %edx)
	incl %ecx
	jmp _shift_pre
_shift_shift:
	movb (%edi, %ebx), %al
	movb %al, (%ecx, %edx)
	incl %edi
	incl %ecx
	cmpl $NUM_DIGITS, %ecx
	jb _shift_shift
	ret

/* nullify a number. 
 * ebx address of number
 * clobbered: ecx
 */
.local nullify
nullify:
	xorl %ecx, %ecx
nullify_loop:
	movb $0, (%ecx, %ebx)
	incl %ecx
	cmpl $NUM_DIGITS, %ecx
	jb nullify_loop
	ret


/* factored out of arith_mul
 * multiply each digit of two numbers into dest.
 * %ebx first number (result as well)
 * %dl digit to multiply with
 * clobbered: %ecx
 */
.local arith_mul_num_only
arith_mul_num_only:
	xorl %ecx, %ecx
arith_mul_num_only_loop:
	movb (%ecx, %ebx), %al
	mulb %dl
	movb %al, (%ecx, %ebx)
	incl %ecx
	cmpl $NUM_DIGITS, %ecx
	jb arith_mul_num_only_loop
	ret
	

/* multiply two numbers, store the result in the first number
   ebx: address of first number
   edx: address of second number
   result stored in _num_erg

   clobbered: about everything
 */
.globl arith_mul
arith_mul:
	movl %ebx, _scratch0
	movl %edx, _scratch1
	movl $_num_erg, %ebx
	call nullify
	xorl %ecx, %ecx
	movl %ecx, _scratch2

_mul_loop:
	/* left shift 2nd number by %ecx digits */
	movl _scratch2, %eax
	movl _scratch1, %ebx

	movl $_num_tmp1, %edx
	call arith_shift_left

	/* multiply with nth digit from first number */
	movl _scratch0, %ebx
	movl _scratch2, %ecx
	movb (%ecx, %ebx), %dl
	movl $_num_tmp1, %ebx

	call arith_mul_num_only
	call arith_norm

	movl %ebx, %edx
	movl $_num_erg, %ebx
	call arith_add

	/* test if done */
	movl _scratch2, %ecx
	incl %ecx
	movl %ecx, _scratch2
	cmpl $NUM_DIGITS, %ecx
	jb _mul_loop

	ret

/* copy from ebx to edx
 * ebx source number
 * edx destination number
 * clobbered ecx, al
 */
.local copy_num
copy_num:
	xorl %ecx, %ecx
copy_loop:
	movb (%ecx, %ebx), %al
	movb %al, (%ecx, %edx)
	incl %ecx
	cmpl $NUM_DIGITS, %ecx
	jb copy_loop
	ret

/* calculate the factorial of ecx.
 * ecx factorical to calculate
 * result stored in _num_erg.
 *
 * clobbered: about everything.
 */
.globl arith_fak
arith_fak:
	movl %ecx, _scratch3
	xorl %eax, %eax
	movl %eax, _scratch4

	/* constant 1 */
	movl $_num_tmp2, %ebx
	call nullify
	addb $1, (%ebx)

	/* multiplicator counter start with 0 */
	movl $_num_tmp3, %ebx
	call nullify

	/* result, start with 1 */
	movl $_num_tmp4, %ebx
	call nullify
	addb $1, (%ebx)
arith_loop:
	/* increase counter */
	movl $_num_tmp3, %ebx
	movl $_num_tmp2, %edx
	call arith_add

	/* multiply counter with result */
	movl $_num_tmp3, %ebx
	movl $_num_tmp4, %edx
	call arith_mul
	/* copy back result */
	movl $_num_erg, %ebx
	movl $_num_tmp4, %edx
	call copy_num

	/* increase register counters */
	movl _scratch3, %ecx
	movl _scratch4, %eax
	incl %eax
	cmpl %ecx, %eax
	movl %ecx, _scratch3
	movl %eax, _scratch4
	jb arith_loop

	/* copy result to _num_erg */
	movl $_num_tmp4, %ebx
	movl $_num_erg, %edx
	call copy_loop
	ret


/* print a number.
   ebx: address of number to print
   
   clobbered: eax, ebx, ecx, edx
 */ 
.globl arith_print
arith_print:
	movl $NUM_DIGITS, %ecx

_print_loop:	
	movb -1(%ecx, %ebx), %al
	movl %ebx, _scratch3
	movl %ecx, _scratch4
	call libs_printb
	movl _scratch3, %ebx
	movl _scratch4, %ecx
	decl %ecx
	jne _print_loop
	movb $0x0a, %al
	call libs_printc
	ret

.data
.local _scratch0
_scratch0:
	.long 0
.local _scratch1
_scratch1:
	.long 0
.local _scratch2
_scratch2:
	.long 0
.local _scratch3
_scratch3:
	.long 0
.local _scratch4
_scratch4:
	.long 0

.local _num_tmp1
_num_tmp1:
	.skip NUM_DIGITS

.local _num_tmp2
_num_tmp2:
	.skip NUM_DIGITS
.local _num_tmp3
_num_tmp3:
	.skip NUM_DIGITS
.local _num_tmp4
_num_tmp4:
	.skip NUM_DIGITS
.local _num_erg
_num_erg:
	.skip NUM_DIGITS
