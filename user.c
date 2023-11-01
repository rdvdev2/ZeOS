#include <libc.h>

char buff[24];

int pid;

int __attribute__((__section__(".text.main"))) main(void) {
  /* Next line, tries to move value 0 to CR3 register. This register is a
   * privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  write(1, "Hello, ZeOS!\n", 14);

  char ticks_buff[10] = "";
  int ticks = gettime();
  itoa(ticks, ticks_buff, 10);
  write(1, ticks_buff, strlen(ticks_buff));
  write(1, " ticks passed...\n", 17);

  int fork_ret = fork();
  char fr_buff[10] = "";
  itoa(fork_ret, fr_buff, 10);
  write(1, "Fork returned: ", 15);
  write(1, fr_buff, strlen(fr_buff));
  write(1, "\n\n", 2);

  int next = gettime() + 300;
  while (1) {
    if (gettime() > next) {
      next += 300;
      char pid_buff[5] = "";
      int pid = getpid();
      itoa(pid, pid_buff, 10);

      write(1, "+300 ticks!\n", 12);
      write(1, pid_buff, strlen(pid_buff));
    }
  }
}
