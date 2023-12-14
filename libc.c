/*
 * libc.c
 */

#include <errors.h>
#include <libc.h>
#include <libpthread.h>
#include <types.h>

int errno;

extern int main();

// This is a small wrapper that initializes the libc for the user, similar to crt0.o
int __attribute__((__section__(".text.main"))) __start() {
  _init_libc();

  return main();
}

void _init_libc() {
  if (__set_thread_wrapper(pthread_wrapper) != 0) {
    int _errno = errno;

    char msg[] = "[LIBC initialization] Couldn't configure the thread wrapper: ";
    write(1, msg, sizeof(msg));

    errno = _errno;
    perror();
  }
}

inline char itoc(int n) {
  if (n < 10)
    return '0' + n;
  else
    return 'a' + n - 10;
}

void itoa(int value, char *str, int base) {
  if (value == 0) {
    str[0] = '0';
    str[1] = '\0';
    return;
  }

  int i = 0;
  while (value > 0) {
    str[i] = itoc(value % base);
    value /= base;
    ++i;
  }

  for (int j = 0; j < i / 2; ++j) {
    char c = str[j];
    str[j] = str[i - j - 1];
    str[i - j - 1] = c;
  }
  str[i] = 0;
}

int strlen(const char *a) {
  int i;

  i = 0;

  while (a[i] != 0)
    i++;

  return i;
}

char *strcpy(char *dst, const char *src) {
  int i;
  for (i = 0; src[i] != '\0'; ++i) {
    dst[i] = src[i];
  }
  dst[i] = '\0';

  return dst;
}

char *strcat(char *dst, const char *src) {
  int last = strlen(dst);
  strcpy(&dst[last], src);

  return dst;
}

void perror() {
  const char *msg = sys_errlist[errno];
  write(1, msg, strlen(msg));
  write(1, "\n", 1);
}

void * memmove(void * dest, const void * src, unsigned long num) {
  if (src > dest)
    for (unsigned long i = 0; i < num; ++i) {
      ((char*)dest)[i] = ((char*)src)[i];
    }
  else
    for (unsigned long i = num - 1; i >= 0; --i) {
      ((char*)dest)[i] = ((char*)src)[i];
    }

  return dest;
}