#include <libc.h>

void pthread_wrapper(void *(*func)(void *), void *param) {
  (*func)(param);
  exit();
}
