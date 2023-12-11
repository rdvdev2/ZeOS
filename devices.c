#include <io.h>
#include <list.h>
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
