#include <io.h>
#include <list.h>
#include <utils.h>

// Queue for blocked processes in I/O
struct list_head blocked;

int sys_write_console(char *buffer, int size) {
  int i;

  for (i = 0; i < size; i++)
    printc(buffer[i]);

  return size;
}
