#include <memcpy.h>
#include <types.h>

void* memcpy(void* dst, void* src, size_t len) {
  for (int i = 0; i < len; ++i) {
    ((Byte *) dst)[i] = ((Byte *) src)[i];
  }
  
  return dst;
}
