#include <asm.h>

ENTRY(stack_switch)
	push %ebp
	movl %esp, %ebp

	call current
	movl %ebp, 8(%eax)
	movl 8(%ebp), %ebx
	movl 8(%ebx), %esp

	pop %ebp
	ret
	
