/*
 * sys.c - Syscalls implementation
 */
#include <block.h>
#include <devices.h>
#include <errno.h>
#include <io.h>
#include <list.h>
#include <mm.h>
#include <mm_address.h>
#include <random.h>
#include <sched.h>
#include <semaphore.h>
#include <types.h>
#include <utils.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions) {
  if (fd != 1)
    return -EBADF;
  if (permissions != ESCRIPTURA)
    return -EACCES;
  return 0;
}

int sys_ni_syscall() { return -ENOSYS; }

int sys_getpid() { return current()->PID; }

int sys_fork() {
  union task_union *new = NULL;
  int clone_ret;
  if ((clone_ret = clone_current_task(&new)) != 0) {
    return -clone_ret;
  }

  allocate_DIR(&new->task);

  page_table_entry *parent_PT = get_PT(current());
  int user_page_nr = 0;

  int temp_page = -1;
  for (int i = NUM_PAG_KERNEL; i < TOTAL_PAGES; ++i) {
    if (parent_PT[i].bits.present)
      ++user_page_nr;
    else
      temp_page = i;
  }

  if (temp_page < 0)
    return -ENOMEM;

  int phys_frames[user_page_nr];
  if (alloc_frames(user_page_nr, phys_frames) == -1) {
    return -ENOMEM;
  }

  page_table_entry *new_PT = get_PT(&new->task);

  copy_data(parent_PT, new_PT, sizeof(page_table_entry) * NUM_PAG_KERNEL);

  int *next_phys_frame = phys_frames;
  for (int i = NUM_PAG_KERNEL; i < TOTAL_PAGES; ++i) {
    if (parent_PT[i].bits.present) {
      int current_frame = *(next_phys_frame++);
      set_ss_pag(new_PT, i, current_frame);
      new_PT[i].bits.avail = parent_PT[i].bits.avail;

      set_ss_pag(parent_PT, temp_page, current_frame);
      set_cr3(current()->dir_pages_baseAddr);
      copy_data(
          (void *)(i * PAGE_SIZE), (void *)(temp_page * PAGE_SIZE), PAGE_SIZE);
    }
  }

  del_ss_pag(parent_PT, temp_page);
  set_cr3(current()->dir_pages_baseAddr);

  new->task.PID = allocate_new_pid();
  new->task.TID = allocate_new_tid();
  new->task.sem_group = 0;

  new->stack[KERNEL_STACK_SIZE - 19] = (unsigned long)ret_from_fork;
  new->stack[KERNEL_STACK_SIZE - 20] =
      (unsigned long)&new->stack[KERNEL_STACK_SIZE - 18];
  new->task.esp = (unsigned long)&new->stack[KERNEL_STACK_SIZE - 20];

  update_process_state_rr(&new->task, &ready_queue);

  return new->task.PID;
}

void sys_exit() {
  struct task_struct *process = current();

  if (list_empty(&process->thread_anchor))
    clear_user_space(process);
  list_del(&process->thread_anchor);

  process->PID = -1;
  process->TID = -1;
  list_add(&(process->queue_anchor), &free_queue);

  sched_next_rr();
}

int sys_waitkey(char *buffer, int timeout) {
  if (!access_ok(VERIFY_WRITE, (void *)buffer, 1)) {
    return -14; // EFAULT
  }

  if (timeout <= 0)
    return -22; // EINVAL

  int error = sys_read_key(buffer, timeout);

  if (error < 0)
    return error;
  else
    return 0;
}

int sys_write(int fd, char *buffer, int size) {

  int check_fd_result = check_fd(fd, ESCRIPTURA);
  if (check_fd_result != 0) {
    return check_fd_result;
  }

  if (buffer == (char *)0) {
    return -EFAULT;
  }

  if (size == 0)
    return 0;

  if (size < 0)
    return -EINVAL;

  int (*write_function)(char *, int);
  switch (fd) {
  case 1:
    write_function = sys_write_console;
    break;
  default:
    return -EIO;
  }

  int remaining = size;
  while (remaining > 0) {
    const int MAX_BUFF_SIZE = KERNEL_STACK_SIZE / 4;

    int buff_size;
    if (remaining > MAX_BUFF_SIZE)
      buff_size = MAX_BUFF_SIZE;
    else
      buff_size = remaining;

    char sys_buffer[buff_size];
    if (copy_from_user(&buffer[size - remaining], sys_buffer, buff_size) != 0) {
      return -EFAULT;
    }
    int written = write_function(sys_buffer, buff_size);

    if (written <= 0)
      return written;

    remaining -= written;
    if (written < buff_size)
      return size - remaining;
  }

  return size;
}

int sys_gettime() { return zeos_ticks; }

int sys_get_stats(int pid, struct stats *st) {
  struct task_struct *task = get_task_with_pid(pid);

  if (task == NULL)
    return -ESRCH;

  if (copy_to_user(&task->st, st, sizeof(struct stats)) != 0) {
    return -EFAULT;
  } else {
    return 0;
  }
}

int sys_gotoXY(int new_x, int new_y) {
  if (new_x < 0 || new_x > NUM_COLUMNS)
    return -EINVAL;
  if (new_y < 0 || new_y > NUM_ROWS)
    return -EINVAL;

  x = new_x;
  y = new_y;

  return 0;
}

int sys_changeColor(int fg, int bg) {
  // Background colors bigger than 8 activate blinking instead of a bright color
  if (fg < 0 || fg > 0xF)
    return -EINVAL;
  if (bg < 0 || bg > 0xF)
    return -EINVAL;

  foreground = fg;
  background = bg;
  return 0;
}

int sys_clrscr(char *b) {
  char print_char = ' ';

  if (b != ((void *)0))
    print_char = *b;
  for (Byte i = 0; i < NUM_COLUMNS; ++i) {
    for (Byte j = 0; j < NUM_ROWS; ++j) {
      printc_xy(i, j, print_char);
    }
  }
  return 0;
}

int sys_create_thread_stack(
    void (*function)(void *arg),
    int N,
    void *parameter) {
  union task_union *new = NULL;
  int clone_ret;
  if ((clone_ret = clone_current_task(&new)) != 0) {
    return clone_ret;
  }

  int phys_frames[N + 1];
  if (alloc_frames(N + 1, phys_frames) == -1) {
    return -ENOMEM;
  }

  page_table_entry *PT = get_PT(current());
  int block_sizes[2] = {N, 1};
  int block_starts[2] = {};
  if (allocate_user_pages(block_sizes, block_starts, 2, PT, phys_frames) != 0) {
    return -ENOMEM;
  }
  int first_stack_page = block_starts[0];

  set_cr3(get_DIR(current()));

  list_add(&new->task.thread_anchor, &current()->thread_anchor);

  new->task.TID = allocate_new_tid();

  unsigned long *stack_bottom =
      (unsigned long *)((N + first_stack_page) * PAGE_SIZE - 4);

  new->stack[KERNEL_STACK_SIZE - 3] = (unsigned long)(stack_bottom - 1); // ESP
  new->stack[KERNEL_STACK_SIZE - 6] = (unsigned long)function;           // EIP

  unsigned long ret_to_pagefault = NULL;
  copy_to_user(&parameter, stack_bottom, sizeof(unsigned long)); // PARAM
  copy_to_user(
      &ret_to_pagefault,
      stack_bottom - 1,
      sizeof(unsigned long)); // @RET

  update_process_state_rr(&new->task, &ready_queue);
  new->task.esp = (unsigned long)&new->stack[KERNEL_STACK_SIZE - 19];
  return 0;
}

char *sys_memRegGet(int num_pages) {
  if (num_pages <= 0)
    return (char *)-EINVAL;

  int frames[num_pages];
  if (alloc_frames(num_pages, frames) < 0)
    return (char *)-ENOMEM;

  page_table_entry *PT = get_PT(current());
  int first_page;
  if (allocate_user_pages(&num_pages, &first_page, 1, PT, frames) < 0) {
    free_frames(num_pages, frames);
    return (char *)-ENOMEM;
  }

  for (int page = first_page; page < first_page + num_pages; ++page) {
    if (page == first_page)
      PT[page].bits.avail = PT_AVAIL_USER_ALLOCATED_HEAD;
    else
      PT[page].bits.avail = PT_AVAIL_USER_ALLOCATED;
  }

  set_cr3(get_DIR(current()));
  return (char *)(PAGE_SIZE * first_page);
}

int sys_memRegDel(char *m) {
  if ((unsigned long)m % PAGE_SIZE != 0)
    return -EINVAL;

  page_table_entry *PT = get_PT(current());
  int first_page = (unsigned long)m / PAGE_SIZE;

  if (!((PT[first_page].bits.avail & PT_AVAIL_USER_ALLOCATED_HEAD) ==
        PT_AVAIL_USER_ALLOCATED_HEAD))
    return -EINVAL;

  int num_pages = 1;
  while (first_page + num_pages < TOTAL_PAGES &&
         (PT[first_page + num_pages].bits.avail &
          PT_AVAIL_USER_ALLOCATED_HEAD) == PT_AVAIL_USER_ALLOCATED)
    ++num_pages;

  int frames[num_pages];
  deallocate_user_pages(&num_pages, &first_page, 1, PT, frames);
  free_frames(num_pages, frames);

  set_cr3(get_DIR(current()));
  return 0;
}

sem_t* sys_semCreate(int initial_value) {
  struct task_struct *process = current();
  
  //Get the process' semaphore group or assign one if it doesn't have any
  struct sem_group *semaphore_group;
  if(process->sem_group == 0) {
    semaphore_group = assign_semaphore_group(process);
    if(semaphore_group == 0) return (sem_t *) -EAGAIN;
  }
  else {
    semaphore_group = process->sem_group;
  }

  semaphore_group->in_use_sems++;
  
  //Search for available semaphore and when found initialize it 
  for(int i=0; i < NR_TASKS; ++i) {
    struct sem *current_semaphore = &semaphore_group->semaphores[i];
    
    if(current_semaphore->owner_TID != -1) continue;
    current_semaphore->owner_TID = process->TID;
    current_semaphore->counter = initial_value;
    
    return (sem_t *) i; 
  }
  return (sem_t *) -ENOMEM;
}

int sys_semWait(sem_t* s) {  
  struct sem* current_semaphore = get_semaphore(s);
  
  if(current_semaphore == 0) return -EINVAL;//Again error, i'll look it up later
  if(current_semaphore->owner_TID == -1) return -EINVAL;
  
  current()->blocked.reason = BR_SEMAPHORE;
  current()->blocked.blocked.semaphore.s = current_semaphore;
  if(--(current_semaphore->counter) < 0) block();
  return 0;
}

int sys_semSignal(sem_t* s) {
  struct sem* current_semaphore = get_semaphore(s);

  if(current_semaphore == 0) return -EINVAL;
  if(current_semaphore->owner_TID == -1) return -EINVAL;

  if(++(current_semaphore->counter) <= 0 && !list_empty(&current_semaphore->blocked_anchor)) {
    if (unblock(list_first(&current_semaphore->blocked_anchor)) < 0) return -EINVAL;
  }
  return 0;
}

int sys_semDestroy(sem_t* s) {
  struct sem* current_semaphore = get_semaphore(s);
  struct task_struct *process = current();
  if(current_semaphore == 0) return -EINVAL;
  if(current_semaphore->owner_TID == -1) return -EINVAL;
  if(current_semaphore->owner_TID != process->TID) return -EINVAL;
 
  unblock_blocked_semaphore_threads(current_semaphore);
 
  initialize_semaphore(current_semaphore);
  
  if(--current()->sem_group->in_use_sems == 0) {
    int unassign_sem_group_res = unassign_semaphore_group(process); 
    if(unassign_sem_group_res == 0) return -EINVAL;
  }
  return 0;
}
