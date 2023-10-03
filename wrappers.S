#include <asm.h>

ENTRY(write)
	pushl %ebp
	movl %esp, %ebp

	movl  8(%ebp), %edx
	movl 12(%ebp), %ecx
	movl 16(%ebp), %ebx
	movl $4, %eax
	int $0x80
	cmpl $0, %eax
	jge exit
	movl $0, %ebx
	subl %eax, %ebx
	movl %ebx, errno
	movl $-1, %eax	
exit:
	popl %ebp
	ret
