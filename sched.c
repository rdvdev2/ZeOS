/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <devices.h>
#include <io.h>
#include <list.h>
#include <mm.h>
#include <msrs.h>
#include <sched.h>
#include <stack.h>
#include <utils.h>

union task_union task[NR_TASKS] __attribute__((__section__(".data.task")));

struct list_head free_queue;
struct list_head ready_queue;

struct task_struct *idle_task;

#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;

int current_task_remaining_quantum;

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

void set_initial_stats(struct task_struct *process) {
  process->st.user_ticks = 0;
  process->st.system_ticks = 0;
  process->st.blocked_ticks = 0;
  process->st.ready_ticks = 0;
  process->st.elapsed_total_ticks = get_ticks();
  process->st.total_trans = 0;
  process->st.remaining_ticks = 0;
}

void init_idle(void) {
  struct list_head *pcb_entry = list_first(&free_queue);
  list_del(pcb_entry);
  union task_union *pcb =
      list_entry(pcb_entry, union task_union, task.free_queue_anchor);

  pcb->task.PID = 0;

  allocate_DIR(&pcb->task);

  pcb->stack[KERNEL_STACK_SIZE - 1] = (unsigned long)cpu_idle;
  pcb->stack[KERNEL_STACK_SIZE - 2] = 0;
  pcb->task.esp = (unsigned long)&pcb->stack[KERNEL_STACK_SIZE - 2];

  set_quantum(&pcb->task, 0);
  // We don't set any state, this task is executed only when there is no other
  // task available

  idle_task = &pcb->task;
  set_initial_stats(idle_task);
}

void init_task1(void) {
  struct list_head *pcb_entry = list_first(&free_queue);
  list_del(pcb_entry);
  union task_union *pcb =
      list_entry(pcb_entry, union task_union, task.free_queue_anchor);

  pcb->task.PID = 1;

  allocate_DIR(&pcb->task);
  set_user_pages(&pcb->task);

  tss.esp0 = (unsigned long)&pcb->stack[KERNEL_STACK_SIZE - 1];
  writeMSR(SYSENTER_ESP_MSR, (unsigned long)&pcb->stack[KERNEL_STACK_SIZE - 1]);

  set_cr3(pcb->task.dir_pages_baseAddr);

  set_quantum(&pcb->task, 10);
  pcb->task.state =
      ST_RUN; // The init task is invoked by the OS after initialization
  current_task_remaining_quantum = 10;

  set_initial_stats(&pcb->task);
}

void init_sched() {
  INIT_LIST_HEAD(&free_queue);
  INIT_LIST_HEAD(&ready_queue);
  INIT_LIST_HEAD(&blocked);

  for (int i = 0; i < NR_TASKS; ++i) {
    list_add(&task[i].task.free_queue_anchor, &free_queue);
  }
}

struct task_struct *current() {
  int ret_value;

  __asm__ __volatile__("movl %%esp, %0" : "=g"(ret_value));
  return (struct task_struct *)(ret_value & 0xfffff000);
}

void inner_task_switch(union task_union *new) {
  tss.esp0 = (unsigned long)&new->stack[KERNEL_STACK_SIZE - 1];
  writeMSR(SYSENTER_ESP_MSR, (unsigned long)&new->stack[KERNEL_STACK_SIZE - 1]);

  set_cr3((*new).task.dir_pages_baseAddr);

  stack_switch(new);
}

int ret_from_fork() { return 0; }

unsigned int last_update_sched_data_rr_tick = 0;

void update_sched_data_rr() {
  int elapsed_ticks = zeos_ticks - last_update_sched_data_rr_tick;

  current_task_remaining_quantum -= elapsed_ticks;
  current()->st.remaining_ticks = current_task_remaining_quantum;

  if (current_task_remaining_quantum < 0)
    current_task_remaining_quantum = 0;

  last_update_sched_data_rr_tick += elapsed_ticks;
}

int needs_sched_rr() {
  return current_task_remaining_quantum == 0 && !list_empty(&ready_queue);
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest) {
  if (current() == idle_task)
    return;

  switch (t->state) {
  case ST_RUN:
    // There is no queue
    break;
  case ST_READY:
    list_del(&t->ready_queue_anchor);
    break;
  case ST_BLOCKED:
    list_del(&t->blocked_queue_anchor);
    break;
  }

  if (dest == NULL) {
    t->state = ST_RUN;
    // There is no queue
  } else if (dest == &ready_queue) {
    t->state = ST_READY;
    list_add_tail(&t->ready_queue_anchor, dest);
  } else if (dest == &blocked) {
    t->state = ST_BLOCKED;
    list_add_tail(&t->blocked_queue_anchor, dest);
  }
}

void sched_next_rr() {
  union task_union *next = NULL;

  if (!list_empty(&ready_queue)) {
    struct list_head *next_head = list_first(&ready_queue);
    next = list_entry(next_head, union task_union, task.ready_queue_anchor);
  } else {
    next = (union task_union *)idle_task;
  }

  update_process_state_rr(&next->task, NULL);

  current_task_remaining_quantum = get_quantum(&next->task);
  current()->st.remaining_ticks = current_task_remaining_quantum;

  stats_system_to_ready();
  task_switch(next);
  stats_ready_to_system();
}

int get_quantum(struct task_struct *t) { return t->quantum; }

void set_quantum(struct task_struct *t, int new_quantum) {
  t->quantum = new_quantum;
}

void schedule() {
  update_sched_data_rr();
  if (needs_sched_rr()) {
    update_process_state_rr(current(), &ready_queue);
    sched_next_rr();
  }
}
