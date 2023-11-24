#include <block.h>
#include <io.h>
#include <list.h>
#include <sched.h>
#include <utils.h>

unsigned int zeos_ticks;

// Queue for blocked processes in I/O
struct list_head keyboard_blocked;

char keyboard_buffer = '\0';

int sys_write_console(char *buffer, int size) {
  int i;

  for (i = 0; i < size; i++)
    printc(buffer[i]);

  return size;
}

int sys_read_key(char *buffer, int timeout) {
  current()->blocked.reason = BR_KEYBOARD;
  current()->blocked.blocked.keyboard.remaining_ticks = timeout;
  block();

  if (keyboard_buffer == '\0')
    return -62; // ETIME

  *buffer = keyboard_buffer;
  keyboard_buffer = '\0';

  return 1;
}
