#ifndef DEVICES_H__
#define DEVICES_H__

#include <list.h>
#include <circular_buffer.h>

extern unsigned int zeos_ticks;
extern struct list_head keyboard_blocked;
extern circular_buff keyboard_buffer;

typedef struct {
  int remaining_ticks;
} KeyboardBlocked;

int sys_write_console(char *buffer, int size);
int sys_read_key(char *buffer, int timeout);
#endif /* DEVICES_H__*/
