/*
 * sys.c - Syscalls implementation
 */
#include "types.h"
#include <devices.h>
#include <errno.h>
#include <io.h>
#include <list.h>
#include <mm.h>
#include <mm_address.h>
#include <random.h>
#include <sched.h>
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
  if (list_empty(&free_queue))
    return -EAGAIN;
  struct list_head *new_entry = list_first(&free_queue);
  list_del(new_entry);
  union task_union *new =
      list_entry(new_entry, union task_union, task.queue_anchor);

  copy_data(current(), new, sizeof(union task_union));

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

  for (int i = 0; i < user_page_nr; ++i) {
    phys_frames[i] = alloc_frame();
    if (phys_frames[i] == -1) {
      for (int j = 0; j < i; ++j)
        free_frame(phys_frames[j]);
      return -ENOMEM;
    }
  }

  page_table_entry *new_PT = get_PT(&new->task);

  copy_data(parent_PT, new_PT, sizeof(page_table_entry) * NUM_PAG_KERNEL);

  int *next_phys_frame = phys_frames;
  for (int i = NUM_PAG_KERNEL; i < TOTAL_PAGES; ++i) {
    if (parent_PT[i].bits.present) {
      int current_frame = *(next_phys_frame++);
      set_ss_pag(new_PT, i, current_frame);

      set_ss_pag(parent_PT, temp_page, current_frame);
      set_cr3(current()->dir_pages_baseAddr);
      copy_data((void *)(i * PAGE_SIZE), (void *)(temp_page * PAGE_SIZE),
                PAGE_SIZE);
    }
  }

  del_ss_pag(parent_PT, temp_page);
  set_cr3(current()->dir_pages_baseAddr);

  int pid;
  do {
    pid = rand();
  } while (pid != -1 && get_task_with_pid(pid) != NULL);
  new->task.PID = pid;

  new->stack[KERNEL_STACK_SIZE - 19] = (unsigned long)ret_from_fork;
  new->stack[KERNEL_STACK_SIZE - 20] =
      (unsigned long)&new->stack[KERNEL_STACK_SIZE - 18];
  new->task.esp = (unsigned long)&new->stack[KERNEL_STACK_SIZE - 20];

  update_process_state_rr(&new->task, &ready_queue);

  return new->task.PID;
}

void sys_exit() {
  struct task_struct *process = current();

  clear_user_space(process);
  process->PID = -1;
  list_add(&(process->queue_anchor), &free_queue);
  sched_next_rr();
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
  if(new_x < 0 || new_x > NUM_COLUMNS) return -EINVAL;
  if(new_y < 0 || new_y > NUM_ROWS) return -EINVAL;

  x = new_x;
  y = new_y;

  return 0; 
}

int sys_changeColor(int fg, int bg) {
  //Background colors bigger than 8 activate blinking instead of a bright color
  if(fg < 0 || fg > 0xF) return -EINVAL;
  if(bg < 0 || bg > 0xF) return -EINVAL; 
  
  foreground = fg;
  background = bg;
  return 0;
}

int sys_clrscr(char* b) {
  char print_char = ' ';
  
  if(b != ((void *) 0)) print_char = *b; 
  for(Byte i = 0; i < NUM_COLUMNS; ++i) {
    for (Byte j = 0; j < NUM_ROWS; ++j) {
	printc_xy(i,j,print_char);
    }
  }
  return 0;
}

int sys_create_thread_stack(void (*function)(void* arg), int N, void* parameter) {
  if(list_empty(&free_queue))
    return -11;
 
  struct list_head *new_entry = list_first(&free_queue);
  list_del(new_entry);
  
  union task_union *new = list_entry(new_entry, union task_union, task.queue_anchor);
  
  //We assing N pages for the user stack and one more for the Thread Local Storage 
  int stack_physical_frames[N];
  int TLS_physical_frame = 0; 

  for (int i = 0; i < N+1; ++i) {
    if(i < N) {
      stack_physical_frames[i] = alloc_frame();
      if(stack_physical_frames[i] == -1) {
        for (int j = 0; j < i; ++j)
          free_frame(stack_physical_frames[j]); 
        return -12; 
      }
    }
    else {
      TLS_physical_frame = alloc_frame();
      if(TLS_physical_frame == -1) {
        free_frame(TLS_physical_frame);
	for(int j = 0; j < N; ++j) 
	  free_frame(stack_physical_frames[j]);
        return -12;
      }
    }	
  } 

  page_table_entry *PT = get_PT(current());
  int consecutive_free_pages = 0;
  int is_TLS_allocated = 0;
  int is_stack_allocated = 0;
  int first_stack_page = 0;

  for(int i = NUM_PAG_KERNEL; i < TOTAL_PAGES; ++i) {
    if(PT[i].bits.present) {
      consecutive_free_pages = 0;
      continue; 
    }

    ++consecutive_free_pages;

    if(consecutive_free_pages == N && !is_stack_allocated) {
      first_stack_page = i - N + 1;
      for(int j = first_stack_page; j <= i; ++j) {
        set_ss_pag(PT,j,stack_physical_frames[j - first_stack_page]);
      }
      is_stack_allocated = 1;
      consecutive_free_pages = 0;
    }
    else if(is_stack_allocated || i == TOTAL_PAGES -1 || PT[i+1].bits.present) {
      set_ss_pag(PT,i,TLS_physical_frame);
      is_TLS_allocated = 1;
      consecutive_free_pages = 0;
    }
    
    if(is_stack_allocated && is_TLS_allocated) break; 
  }
  
  set_cr3(get_DIR(current()));
  copy_data(current(), new, sizeof(union task_union));
  
  list_add(&new->task.thread_anchor, &current()->thread_anchor);
 /* 
  int tid;
  do {
    tid = rand();
  } while(tid != -1 && get_task_with_tid(tid) != NULL);
  new->task.TID = tid;
*/ 
  new->task.TID = 1;
  unsigned long *stack_bottom = (unsigned long *) ((N+1  + first_stack_page) * PAGE_SIZE - 4);
  new->stack[KERNEL_STACK_SIZE - 3] = (unsigned long) stack_bottom - 2;
  new->stack[KERNEL_STACK_SIZE - 6] = (unsigned long) function;
//  new->stack[KERNEL_STACK_SIZE - 17] = (unsigned long) function;
//  new->stack[KERNEL_STACK_SIZE - 18] = 0;
//  *stack_bottom = (unsigned long) parameter;
  copy_to_user(&parameter, stack_bottom, sizeof(unsigned long));
  unsigned long number = 0;
  copy_to_user(&number,stack_bottom-1,sizeof(unsigned long)); 
  copy_to_user(&number,stack_bottom-2,sizeof(unsigned long)); 
  update_process_state_rr(&new->task, &ready_queue);
  new->task.esp = (unsigned long)&new->stack[KERNEL_STACK_SIZE - 18] - 4;
  return 0;
}
