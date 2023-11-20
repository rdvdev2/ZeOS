/*
 * interrupt.c -
 */
#include "block.h"
#include <devices.h>
#include <entry.h>
#include <hardware.h>
#include <interrupt.h>
#include <io.h>
#include <klibc.h>
#include <list.h>
#include <msrs.h>
#include <sched.h>
#include <segment.h>
#include <sys_call_table.h>
#include <types.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register idtR;

char char_map[] = {'\0', '\0', '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
                   '9',  '0',  '\'', '¡',  '\0', '\0', 'q',  'w',  'e',  'r',
                   't',  'y',  'u',  'i',  'o',  'p',  '`',  '+',  '\0', '\0',
                   'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  'ñ',
                   '\0', 'º',  '\0', 'ç',  'z',  'x',  'c',  'v',  'b',  'n',
                   'm',  ',',  '.',  '-',  '\0', '*',  '\0', '\0', '\0', '\0',
                   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                   '\0', '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
                   '2',  '3',  '0',  '\0', '\0', '\0', '<',  '\0', '\0', '\0',
                   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

void setInterruptHandler(int vector, void (*handler)(),
                         int maxAccessibleFromPL) {
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00; /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags = flags;
  idt[vector].highOffset = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL) {
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  // flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00; /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags = flags;
  idt[vector].highOffset = highWord((DWord)handler);
}

void setIdt() {
  /* Program interrups/exception service routines */
  idtR.base = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;

  set_handlers();

  /* INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler(14, page_fault_handler_student, 0);
  setInterruptHandler(33, keyboard_handler, 0);
  setInterruptHandler(32, clock_handler, 0);
  setTrapHandler(0x80, system_call_handler, 3);

  set_idt_reg(&idtR);
}

void init_fast_syscalls() {
  writeMSR(SYSENTER_CS_MSR, __KERNEL_CS);
  writeMSR(SYSENTER_ESP_MSR, INITIAL_ESP);
  writeMSR(SYSENTER_EIP_MSR, (unsigned long)system_call_handler_sysenter);
}

void page_fault_routine_student(unsigned int eip) {
  char eip_hex[30] = "";
  k_itoa(eip, eip_hex, 16);

  printk("\n\nProcess generates a PAGE FAULT exception at EIP: 0x");
  printk(eip_hex);
  printk("\n");
  for (;;)
    ;
}

void keyboard_routine() {
  unsigned char data = inb(0x60);

  if (!(data & (1 << 7))) {
    char c = char_map[data];
    if (c == '\0')
      c = 'C';
    printc_xy(0, 0, c);

    keyboard_buffer = c;
    if (!list_empty(&keyboard_blocked)) {
      struct list_head *first = list_first(&keyboard_blocked);
      unblock(first);
    }
  }
}

void clock_routine() {
  zeos_show_clock();
  ++zeos_ticks;

  schedule();
}
