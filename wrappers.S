#include <asm.h>

#ifndef fast_syscalls
#define CALL_OS(SC_NUM) \
	movl $##SC_NUM, %eax; \
	int $0x80;
#else
#define CALL_OS(SC_NUM) \
	movl $##SC_NUM, %eax; \
	pushl %ecx; \
	pushl %edx; \
	pushl $ret_##SC_NUM; \
	pushl %ebp; \
	movl %esp, %ebp; \
	sysenter; \
ret_##SC_NUM: \
	popl %ebp; \
	addl $4, %esp; \
	popl %edx; \
	popl %ecx;
#endif

#define HANDLE_ERRORS(SC_NUM) \
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
	HANDLE_ERRORS(4)

	popl %ebx

	popl %ebp
	ret

ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp

	CALL_OS(10)

	popl %ebp
	ret

ENTRY(getpid)
	pushl %ebp
	movl %esp, %ebp

	CALL_OS(20)

	popl %ebp
	ret

ENTRY(fork)
	pushl %ebp
	movl %esp, %ebp
	
	CALL_OS(2)
	HANDLE_ERRORS(2)

	pop %ebp
	ret

ENTRY(exit)
	pushl %ebp
	movl %esp, %ebp
	
	CALL_OS(1)

ENTRY(get_stats)
	pushl %ebp
	movl %esp, %ebp
	
	movl 8(%ebp), %edx
	movl 12(%ebp), %ecx
	CALL_OS(35)
	HANDLE_ERRORS(35)

	pop %ebp
	ret

ENTRY(gotoXY)
	pushl %ebp
	movl %esp, %ebp
	
	movl 8(%ebp), %edx
	movl 12(%ebp), %ecx
	CALL_OS(5)
	HANDLE_ERRORS(5)

	pop %ebp
	ret

ENTRY(changeColor)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx
	movl 12(%ebp), %ecx
	CALL_OS(6)
	HANDLE_ERRORS(6)

	pop %ebp
	ret

ENTRY(clrscr)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx
	CALL_OS(7)
	HANDLE_ERRORS(7)

	pop %ebp
	ret

ENTRY(threadCreateWithStack)
	pushl %ebp
	movl %esp, %ebp

	pushl %ebx

	movl 8(%ebp), %edx
	movl 12(%ebp), %ecx
	movl 16(%ebp), %ebx
	CALL_OS(56)
	HANDLE_ERRORS(56)

	popl %ebx

	pop %ebp
	ret
