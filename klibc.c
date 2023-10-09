#include <klibc.h>

inline char k_itoc(int n) {
  if (n < 10)
    return '0' + n;
  else
    return 'a' + n - 10;
}

void k_itoa(int value, char *str, int base) {
  if (value == 0) {
    str[0] = '0';
    str[1] = '\0';
    return;
  }

  int i = 0;
  while (value > 0) {
    str[i] = k_itoc(value % base);
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
