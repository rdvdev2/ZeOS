#ifndef BLOCK_H
#define BLOCK_H

#include <devices.h>
#include <list.h>
#include <semaphore.h>

typedef enum {
  BR_KEYBOARD,
  BR_SEMAPHORE,
} BlockReason;

typedef union {
  KeyboardBlocked keyboard;
  SemaphoreBlocked semaphore;
} BlockedUnion;

typedef struct {
  BlockReason reason;
  BlockedUnion blocked;
} BlockedTaggedUnion;

void block();
int unblock(struct list_head *task_anchor);
#endif
