#include <inner_task_switch.h>
#include <msrs.h>
#include <sched.h>

void inner_task_switch(union task_union *new) {
	tss.esp0 = (unsigned long) &new->stack[KERNEL_STACK_SIZE-1];
  writeMSR(SYSENTER_ESP_MSR, (unsigned long) &new->stack[KERNEL_STACK_SIZE-1]);

	set_cr3((*new).task.dir_pages_baseAddr);


	stack_switch(new);
/*
	__asm__(
		"call current\n"
		"movl %ebp, 8(%eax)\n" %eax tiene la direccion del PCB, movemos ebp a PCB.esp
		"movl 8(%ebp), %ebx\n" 
		"movl 8(%ebx), %esp\n" 8(%ebp) es new, movemos *new + 8 (esp) a %esp
		"pop %ebp\n"
		"ret\n"		
	);
*/
}
