#include <inner_task_switch.h>

void inner_task_switch(union task_union *new) {
	tss.esp0 = (DWord) new;
	set_cr3((*new).task.dir_pages_baseAddr);


	stack_switch();
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
