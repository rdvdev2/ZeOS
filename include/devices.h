#ifndef DEVICES_H__
#define DEVICES_H__

#include <list.h>

extern unsigned int zeos_ticks;
extern struct list_head keyboard_blocked;
extern char keyboard_buffer;

int sys_write_console(char *buffer, int size);
#endif /* DEVICES_H__*/
