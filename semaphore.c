#include <semaphore.h>


struct sem_group semaphore_groups[NR_TASKS/2];

void initialize_semaphore(struct sem *s) {
  s->in_use = 0;
  s->counter = 0;
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
  return &current()->sem_group->semaphores[(int)s];
}
