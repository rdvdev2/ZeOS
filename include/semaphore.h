#ifndef SEM_H
#define SEM_H

#include <list.h>
#include <sched.h>
#include <block.h>

#define NR_SEMS NR_TASKS
typedef int sem_t;

struct list_head free_semaphore_group_queue;

struct sem {
  int counter;
  int owner_TID;
  struct list_head blocked_anchor;
};

struct sem_group {
  struct list_head semaphore_group_anchor;
  int in_use_sems;
  struct sem semaphores[NR_SEMS];
};

void init_sems();

void initialize_semaphore(struct sem *s);

struct sem_group* assign_semaphore_group(struct task_struct *p);

struct sem* get_semaphore(sem_t *s);

int unblock_blocked_semaphore_threads(struct sem* s);

int unassign_semaphore_group(struct task_struct *p);

#endif
