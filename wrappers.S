#include <asm.h>

#define CALL_OS(SC_NUM) \
	movl $##SC_NUM, %eax; \
	int $0x80; \
\
	cmpl $0, %eax; \
	jge exit_##SC_NUM; \
	movl $0, %ebx; \
	subl %eax, %ebx; \
	movl %ebx, errno; \
	movl $-1, %eax; \
exit_##SC_NUM:

ENTRY(write)
	pushl %ebp
	movl %esp, %ebp

	pushl %ebx

	movl  8(%ebp), %edx
	movl 12(%ebp), %ecx
	movl 16(%ebp), %ebx
	CALL_OS(4)

	popl %ebx

	popl %ebp
	ret

ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp

	CALL_OS(10)

	popl %ebp
	ret
