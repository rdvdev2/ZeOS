/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <mm_address.h>
#include <stats.h>
#include <types.h>

#define NR_TASKS 10
#define KERNEL_STACK_SIZE 1024

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct task_struct {
  int PID; /* Process ID. This MUST be the first field of the struct. */
  page_table_entry *dir_pages_baseAddr;
  unsigned long esp;
  struct list_head queue_anchor;
  enum state_t state;
  struct stats st;
  int quantum;
  struct list_head thread_anchor;
  int TID; // Thread ID
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE]; /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */
extern struct task_struct *idle_task;

extern struct list_head free_queue;
extern struct list_head ready_queue;

#define KERNEL_ESP(t) (DWord) & (t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP KERNEL_ESP(&task[1])

void set_initial_stats(struct task_struct *process);

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

int clone_current_task(union task_union ** new);

struct task_struct *current();

void task_switch(union task_union *t);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry *get_PT(struct task_struct *t);

page_table_entry *get_DIR(struct task_struct *t);

/* Headers for the scheduling policy */
void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();

int get_quantum(struct task_struct *t);
void set_quantum(struct task_struct *t, int new_quantum);

void schedule();

int ret_from_fork();

struct task_struct *get_task_with_pid(int pid);
struct task_struct *get_task_with_tid(int tid);

int allocate_new_pid();
int allocate_new_tid();

#endif /* __SCHED_H__ */
