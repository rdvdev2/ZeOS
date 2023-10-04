#include <asm.h>

ENTRY(write)
	pushl %ebp
	movl %esp, %ebp

	pushl %ebx

	movl  8(%ebp), %edx
	movl 12(%ebp), %ecx
	movl 16(%ebp), %ebx
	movl $4, %eax
	int $0x80
	cmpl $0, %eax
	jge exit_write
	movl $0, %ebx
	subl %eax, %ebx
	movl %ebx, errno
	movl $-1, %eax	
exit_write:
	popl %ebx

	popl %ebp
	ret

ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp

	movl $10, %eax
	int $0x80
	cmpl $0, %eax
	jge exit_gettime
	movl $0, %ebx
	subl %eax, %ebx
	movl %ebx, errno
	movl $-1, %eax
exit_gettime:
	popl %ebp
	ret
