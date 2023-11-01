/*
 * libc.c
 */

#include <errors.h>
#include <libc.h>

#include <types.h>

int errno;

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
