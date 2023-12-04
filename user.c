#include <libc.h>
#include <stats.h>

char buff[24];

int pid;

void bucle_infinito(void* n) {
  int random_number = *(int *) n;
  char random_number_buff[10] = "";
  itoa(random_number, random_number_buff, 10);
  write(1, random_number_buff, strlen(random_number_buff));

  write(1, "Hola, soy el nuevo thread!\n", 27);
  exit();
}

void echo() {
  char * buff = "";
  for (;;) {
    if (waitKey(buff, 300) == 0)
      write(1, buff, 1);
    else
      perror();
  }
}

int __attribute__((__section__(".text.main"))) main(void) {
  /* Next line, tries to move value 0 to CR3 register. This register is a
   * privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  write(1, "Hello, ZeOS!\n", 14);

  switch (fork()) {
  case 0:
    echo();
    break;
  case -1:
    perror();
    break;
  }

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
  else if (fork_ret == 0) {
    char fork_msg[] = "\n\nI'm a FORK! ===E\n\n";
    write(1, fork_msg, strlen(fork_msg));
  }

  int pid = getpid();
  char pid_buff[5] = "";
  itoa(pid, pid_buff, 10);

  char pid_msg_buff[15] = "PID: ";
  strcat(pid_msg_buff, pid_buff);
  strcat(pid_msg_buff, "\n\n");
  write(1, pid_msg_buff, strlen(pid_msg_buff));
  
 int bg = 0;
 int fg = 0;
  int next = gettime() + 300;
  
  int random_number = 5;
  if (fork_ret > 0)threadCreateWithStack(bucle_infinito, 3, (void *)&random_number);
  while (1) {
    if (gettime() > next) {
      
/*    TEST ONLY!!! Please uncomment variables bg and fg above*/
      ++bg;
      ++fg;
      char a = 'a';
      char *null_char = ((void *) 0);
      if (bg <= 0xF && fg <= 0xF)changeColor(fg,bg);
      else if (bg == 20) clrscr(&a);
      else if (bg == 30) clrscr(null_char);

      next += 300;

      char ticks_buff2[40] = "(";
      strcat(ticks_buff2, pid_buff);
      strcat(ticks_buff2, ") +300 ticks!\n");
      write(1, ticks_buff2, sizeof(ticks_buff2));

      struct stats my_stats = {};
      if (get_stats(pid, &my_stats) < 0) {
        perror();
        continue;
      }

      char stats_buff[100] = "(";
      strcat(stats_buff, pid_buff);
      strcat(stats_buff, ") USR: ");
      itoa(my_stats.user_ticks, &stats_buff[strlen(stats_buff)], 10);
      strcat(stats_buff, "; SYS: ");
      itoa(my_stats.system_ticks, &stats_buff[strlen(stats_buff)], 10);
      strcat(stats_buff, "; RDY: ");
      itoa(my_stats.ready_ticks, &stats_buff[strlen(stats_buff)], 10);
      strcat(stats_buff, "; TRS: ");
      itoa(my_stats.total_trans, &stats_buff[strlen(stats_buff)], 10);
      strcat(stats_buff, "\n");
      write(1, stats_buff, strlen(stats_buff));

    }
  }
}
