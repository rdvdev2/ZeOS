#ifndef BLOCK_H
#define BLOCK_H

#include "devices.h"
#include <list.h>

typedef enum {
  BR_KEYBOARD,
} BlockReason;

typedef union {
  KeyboardBlocked keyboard;
} BlockedUnion;

typedef struct {
  BlockReason reason;
  BlockedUnion blocked;
} BlockedTaggedUnion;

void block();
int unblock(struct list_head *task_anchor);

#endif
