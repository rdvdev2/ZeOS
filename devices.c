#include <circular_buffer.h>
#include <block.h>
#include <io.h>
#include <list.h>
#include <sched.h>
#include <utils.h>

unsigned int zeos_ticks;

// Queue for blocked processes in I/O
struct list_head keyboard_blocked;

circular_buff keyboard_buffer;

int sys_write_console(char *buffer, int size) {
  int i;

  for (i = 0; i < size; i++)
    printc(buffer[i]);

  return size;
}

int sys_read_key(char *buffer, int timeout) {
  if (remove_item(&keyboard_buffer, buffer) == 0) {
    return 1;
  }
  
  current()->blocked.reason = BR_KEYBOARD;
  current()->blocked.blocked.keyboard.remaining_ticks = timeout;
  block();

  if (remove_item(&keyboard_buffer, buffer) == 0) {
    return 1;
  }
  
  return -62; // ETIME
}
