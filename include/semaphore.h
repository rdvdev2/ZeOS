#ifndef SEM_H
#define SEM_H

#include <list.h>
#include <sched.h>

typedef int sem_t;

struct sem {
  int in_use;
  int counter;
  struct list_head blocked_anchor;
};

struct sem_group {
  struct list_head semaphore_group_anchor;
  struct sem semaphores[10];
};

void initialize_semaphore(struct sem *s);

struct sem_group* assign_semaphore_group(struct task_struct *p);

struct sem* get_semaphore(sem_t *s);

#endif
