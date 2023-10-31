#include <inner_task_switch.h>
#include <msrs.h>
#include <sched.h>

void stack_switch(union task_union *new);

void inner_task_switch(union task_union *new) {
	tss.esp0 = (unsigned long) &new->stack[KERNEL_STACK_SIZE-1];
  writeMSR(SYSENTER_ESP_MSR, (unsigned long) &new->stack[KERNEL_STACK_SIZE-1]);

	set_cr3((*new).task.dir_pages_baseAddr);

	stack_switch(new);
}
