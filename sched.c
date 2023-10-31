/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include "list.h"
#include <msrs.h>
#include <io.h>
#include <mm.h>
#include <sched.h>

union task_union task[NR_TASKS] __attribute__((__section__(".data.task")));

struct list_head free_queue;
struct list_head ready_queue;

struct task_struct * idle_task;
struct task_struct * task1_task;

#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry *get_DIR(struct task_struct *t) {
  return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry *get_PT(struct task_struct *t) {
  return (page_table_entry
              *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))
                 << 12);
}

int allocate_DIR(struct task_struct *t) {
  int pos;

  pos = ((int)t - (int)task) / sizeof(union task_union);

  t->dir_pages_baseAddr = (page_table_entry *)&dir_pages[pos];

  return 1;
}

void cpu_idle(void) {
  __asm__ __volatile__("sti" : : : "memory");

  while (1) {
    ;
  }
}

void init_idle(void) {
  struct list_head * pcb_entry = list_first(&free_queue);
  list_del(pcb_entry);
  union task_union * pcb = list_entry(pcb_entry, union task_union, task.free_queue_anchor);

  pcb->task.PID = 0;

  allocate_DIR(&pcb->task);

  pcb->stack[KERNEL_STACK_SIZE-1] = (unsigned long) cpu_idle;
  pcb->stack[KERNEL_STACK_SIZE-2] = 0;
  pcb->task.esp = (unsigned long) &pcb->stack[KERNEL_STACK_SIZE-2];

  idle_task = &pcb->task;
}

void init_task1(void) {
  struct list_head * pcb_entry = list_first(&free_queue);
  list_del(pcb_entry);
  union task_union * pcb = list_entry(pcb_entry, union task_union, task.free_queue_anchor);

  pcb->task.PID = 1;

  allocate_DIR(&pcb->task);
  set_user_pages(&pcb->task);

  tss.esp0 = (unsigned long) &pcb->stack[KERNEL_STACK_SIZE-1];
  writeMSR(SYSENTER_ESP_MSR, (unsigned long) &pcb->stack[KERNEL_STACK_SIZE-1]);

  set_cr3(pcb->task.dir_pages_baseAddr);

  task1_task = &pcb->task;
}

void init_sched() {
  INIT_LIST_HEAD(&free_queue);
  INIT_LIST_HEAD(&ready_queue);
  
  for (int i = 0; i < NR_TASKS; ++i) {
    list_add(&task[i].task.free_queue_anchor, &free_queue);
  }
}

struct task_struct *current() {
  int ret_value;

  __asm__ __volatile__("movl %%esp, %0" : "=g"(ret_value));
  return (struct task_struct *)(ret_value & 0xfffff000);
}
