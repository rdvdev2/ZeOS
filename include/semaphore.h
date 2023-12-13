#ifndef SEM_H
#define SEM_H

#include <list.h>
#include <sched.h>

typedef int sem_t

struct sem {
  int in_use;
  int count;
  struct list_head blocked_anchor;
}

struct sem_group {
  struct list_head sempahore_group_anchor
  struct sem[NR_TASKS]
}

#endif
