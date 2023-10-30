#include <asm.h>

ENTRY(stack_switch)
	push %ebp
	movl %esp, %ebp

	call current
	movl %ebp, 8(%eax)
	movl 8(%ebp), %ebx #Si algo misteriosamente no va, ponle un 16
	movl 8(%ebx), %esp

	pop %ebp
	ret
	
