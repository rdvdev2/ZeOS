#include <asm.h>
#include <segment.h>

ENTRY (system_call_handler)
	SAVE_ALL
	cmpl $0, %eax
	jl err
	cmpl $MAX_SYSCALL, %eax
	jg err
	call *sys_call_table(, %eax, 0x04)
	jmp fin
err:
	movl $-38, %eax // ENOSYS
fin:
	movl %eax, 0x18(%esp)
	RESTORE_ALL
	iret

ENTRY(sys_call_table)
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_write
.globl MAX_SYSCALL
MAX_SYSCALL = (. - sys_call_table)
