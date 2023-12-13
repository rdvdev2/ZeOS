#include <semaphore.h>


struct sem_group semaphore_groups[NR_TASKS/2];
extern struct list_head free_semaphore_group_queue;

void init_sems() {
  INIT_LIST_HEAD(&free_semaphore_group_queue); 
 
  for(int i = 0; i < NR_TASKS/2; ++i) {
    for(int j = 0; j < NR_SEMS; ++j) {
      initialize_semaphore(&(semaphore_groups[i].semaphores[j]));
    }
    list_add(&semaphore_groups[i].semaphore_group_anchor, &free_semaphore_group_queue);
  }
}

void initialize_semaphore(struct sem *s) {
  s->in_use = 0;
  s->counter = 0;
  s->parent_TID = -1;
  INIT_LIST_HEAD(&s->blocked_anchor);
}

struct sem_group* assign_semaphore_group(struct task_struct *p) {
  if(list_empty(&free_sem_group_queue) == 0) return 0;
  struct list_head *sem_group_head = list_first(&free_sem_group_queue);
  struct sem_group *group = list_entry(sem_group_head, struct sem_group, semaphore_group_anchor);
  list_del(sem_group_head);
  
  struct list_head* e;
  list_for_each(e, &p->thread_anchor) {
    struct task_struct *thread = list_entry(e, struct task_struct, thread_anchor);
    thread->sem_group = group;
  }

  p->sem_group = group;
  return group; 
}

struct sem* get_semaphore(sem_t *s) {
  int sem_number = (int) s;
  
  if(current()->sem_group == 0) return 0;
  if(sem_number < 0 || sem_number > NR_SEMS) return 0;
  return &current()->sem_group->semaphores[sem_number];
}

int unblock_blocked_semaphore_threads(struct sem* s) {
  struct list_head *e;
  list_for_each(e, &s->blocked_anchor) {
    if(unblock(e) < 1) return -1;
    list_del(e);
  }
  return 0;
}
