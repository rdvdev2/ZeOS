#include <block.h>
#include <devices.h>
#include <list.h>
#include <sched.h>

int block() {
  struct list_head *blocked_queue;

  // Add the task to the adequate blocked queue. NULL signifies failure.
  switch (current()->blocked.reason) {
  case BR_KEYBOARD:
    blocked_queue = &keyboard_blocked;
    break;
  default:
    blocked_queue = NULL;
    break;
  }

  if (blocked_queue == NULL)
    return -1;

  update_process_state_rr(current(), blocked_queue);
  sched_next_rr();

  return 0;
}

int unblock(struct list_head *task_anchor) {
  if (list_empty(task_anchor))
    return -1;

  struct task_struct *task =
      list_entry(task_anchor, struct task_struct, queue_anchor);

  if (task->state != ST_BLOCKED || task->blocked.reason == BR_NONE)
    return -1;

  task->blocked.reason = BR_NONE;
  update_process_state_rr(task, &ready_queue);

  return 0;
}
