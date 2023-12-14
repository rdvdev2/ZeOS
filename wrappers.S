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
	movl $0, %ecx; \
	subl %eax, %ecx; \
	movl %ecx, errno; \
	movl $-1, %eax; \
exit_##SC_NUM:

ENTRY(waitKey)
	pushl %ebp
	movl %esp, %ebp

	movl  8(%ebp), %edx
	movl 12(%ebp), %ecx
	CALL_OS(3)
	HANDLE_ERRORS(3)

	popl %ebp
	ret

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

ENTRY(__set_thread_wrapper)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx
	CALL_OS(55)
	HANDLE_ERRORS(55)

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

ENTRY(memRegGet)
	pushl %ebp
     	movl %esp, %ebp

	movl 8(%ebp), %edx
 	CALL_OS(12)
	HANDLE_ERRORS(12)
	
	pop %ebp
	ret

ENTRY(memRegDel)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx
	CALL_OS(13)
	HANDLE_ERRORS(13)

	pop %ebp
	ret

ENTRY(semCreate)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx
	CALL_OS(64)
	HANDLE_ERRORS(64)

	pop %ebp
	ret

ENTRY(semWait)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx
	CALL_OS(65)
	HANDLE_ERRORS(65)

	pop %ebp
	ret


ENTRY(semSignal)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx
	CALL_OS(66)
	HANDLE_ERRORS(66)

	pop %ebp
	ret


ENTRY(semDestroy)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx
	CALL_OS(67)
	HANDLE_ERRORS(67)

	pop %ebp
	ret
