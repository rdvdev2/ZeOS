/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>

/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %edx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %ebx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %ebx; \
      pushl %ecx; \
      pushl %edx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

#define RESTORE_ALL \
      popl %edx; \
      popl %ecx; \
      popl %ebx; \
      popl %esi; \
      popl %edi; \
      popl %ebp; \
      popl %eax; \
      popl %ds; \
      popl %es; \
      popl %fs; \
      popl %gs;

#define EOI \
      movb $0x20, %al; \
      outb %al, $0x20;

// EXCEPTIONS

ENTRY(page_fault_handler_student)
	SAVE_ALL

	pushl 0x30(%esp)
	call page_fault_routine_student
	addl $4, %esp

	RESTORE_ALL
	addl $4, %esp
	iret

// INTERRUPTS

ENTRY(keyboard_handler)
	SAVE_ALL
	EOI
	call stats_user_to_system
	call keyboard_routine
	call stats_system_to_user
	RESTORE_ALL
	iret

ENTRY(clock_handler)
	SAVE_ALL
	EOI
	call stats_user_to_system
	call clock_routine
	call stats_system_to_user
	RESTORE_ALL
	iret


// SYSCALLS

ENTRY (system_call_handler)
        SAVE_ALL
	call stats_user_to_system
	movl 0x18(%esp), %eax
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
	call stats_system_to_user
        RESTORE_ALL
        iret

ENTRY(system_call_handler_sysenter)
	pushl $__USER_DS
	pushl %ebp
	pushfl
	pushl $__USER_CS
	push 4(%ebp)
	SAVE_ALL
	call stats_user_to_system
	movl 0x18(%esp), %eax
	cmpl $0, %eax
	jl sysenter_err
	cmpl $MAX_SYSCALL, %eax
	jg sysenter_err
	call *sys_call_table(, %eax, 0x04)
	jmp sysenter_fin
sysenter_err:
	movl $-38, %eax // ENOSYS
sysenter_fin:
	movl %eax, 0x18(%esp)
	call stats_system_to_user
	RESTORE_ALL
	movl (%esp), %edx
	movl 12(%esp), %ecx
	sti
	sysexit
