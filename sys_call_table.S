#include <asm.h>
#include <segment.h>

ENTRY(sys_call_table)
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_write
.globl MAX_SYSCALL
MAX_SYSCALL = (. - sys_call_table)
