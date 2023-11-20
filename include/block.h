#ifndef BLOCK_H
#define BLOCK_H

#include <list.h>

typedef enum {
  BR_NONE,
  BR_KEYBOARD,
} BlockReason;

typedef union {
  void *dummy;
} BlockedUnion;

typedef struct {
  BlockReason reason;
  BlockedUnion blocked;
} BlockedTaggedUnion;

int block();
int unblock(struct list_head *task_anchor);

#endif
