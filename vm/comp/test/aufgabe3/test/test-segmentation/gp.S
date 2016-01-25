/* $Id: gp.S 10001 2014-10-15 14:00:05Z raimue $ */

/* foce a GP by accessing way beyond the limit. */

.text
.globl _start
_start:
	movl $0xafbeadde, %eax
	movl (%eax), %eax
	/* not reached */
	cli
	hlt
