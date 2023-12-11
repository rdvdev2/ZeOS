/*
 * io.c -
 */

#include <io.h>
#include <types.h>
#include <utils.h>

/**************/
/** Screen  ***/
/**************/

Byte x, y = 19;
int foreground = 2;
int background = 0xF;

/* Read a byte from 'port' */
Byte inb(unsigned short port) {
  Byte v;

  __asm__ __volatile__("inb %w1,%0" : "=a"(v) : "Nd"(port));
  return v;
}

void scroll_screen() {
  for (int i = 1; i < NUM_ROWS; ++i) {
    copy_data((void *)(0xb8000 + i * NUM_COLUMNS * 2),
              (void *)(0xb8000 + (i - 1) * NUM_COLUMNS * 2), NUM_COLUMNS * 2);
  }

  for (int i = 0; i < NUM_COLUMNS * 2; ++i) {
    *((char *)(0xb8000 + (NUM_ROWS - 1) * NUM_COLUMNS * 2 + i)) = 0;
  }
}

void printc(char c) {
  __asm__ __volatile__("movb %0, %%al; outb $0xe9" ::"a"(
      c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c == '\0')
    return;
  else if (c == '\n') {
    x = 0;
    if (++y >= NUM_ROWS) {
      scroll_screen();
      --y;
    }
  } else {
    int color = (background << 12 | foreground << 8) & 0xFF00;
    Word ch = (Word)(c & 0x00FF) | color;
    Word *screen = (Word *)0xb8000;
    screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS) {
      x = 0;
      if (++y >= NUM_ROWS) {
        scroll_screen();
        --y;
      }
    }
  }
}

void printc_xy(Byte mx, Byte my, char c) {
  Byte cx, cy;
  cx = x;
  cy = y;
  x = mx;
  y = my;
  printc(c);
  x = cx;
  y = cy;
}

void printk(char *string) {
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}
