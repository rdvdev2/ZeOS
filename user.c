#include <libc.h>

char buff[24];

int pid;

int __attribute__((__section__(".text.main"))) main(void) {
  /* Next line, tries to move value 0 to CR3 register. This register is a
   * privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  write(1, "Hello, ZeOS!\n", 14);

  int ticks = gettime();
  char ticks_buff[40] = "";
  itoa(ticks, ticks_buff, 10);

  strcat(ticks_buff, " ticks passed...\n");
  write(1, ticks_buff, strlen(ticks_buff));

  int fork_ret = fork();
  char fr_buff[10] = "";
  itoa(fork_ret, fr_buff, 10);

  char fork_msg_buff[40] = "Fork returned: ";
  strcat(fork_msg_buff, fr_buff);
  strcat(fork_msg_buff, "\n");
  write(1, fork_msg_buff, sizeof(fork_msg_buff));

  if (fork_ret < 0)
    perror();

  int pid = getpid();
  char pid_buff[5] = "";
  itoa(pid, pid_buff, 10);

  char pid_msg_buff[15] = "PID: ";
  strcat(pid_msg_buff, pid_buff);
  strcat(pid_msg_buff, "\n\n");
  write(1, pid_msg_buff, strlen(pid_msg_buff));

  int next = gettime() + 300;
  while (1) {
    if (gettime() > next) {
      next += 300;

      char ticks_buff2[40] = "(";
      strcat(ticks_buff2, pid_buff);
      strcat(ticks_buff2, ") +300 ticks!\n");
      write(1, ticks_buff2, sizeof(ticks_buff2));
    }
  }
}
