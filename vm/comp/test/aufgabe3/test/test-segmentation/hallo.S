/* $Id: hallo.S 10001 2014-10-15 14:00:05Z raimue $ */

/* simple binary file format: print "hallo welt" */

.text
.globl _start
_start:
	movl $txt, %edi

print_loop:
	movb (%edi), %al

	/* last byte reached? */
	cmp $0, %al
	je end_loop

	/* print character */
	call libs_printc
	incl %edi
	jmp print_loop

end_loop:
	/* end of test. */
	jmp libs_exit

.section .rodata
.local txt
txt:
.ascii "hallo welt\n\0"
