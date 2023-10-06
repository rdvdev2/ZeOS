#include <asm.h>

ENTRY(writeMSR)
	pushl %ebp
	movl %esp, %ebp
	
	movl 8(%esp), %ecx
	movl 12(%esp), %eax
	movl 16(%esp), %edx
	wrmsr

	popl %ebp
	ret
