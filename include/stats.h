#ifndef STATS_H
#define STATS_H

struct task_struct;

/* Structure used by 'get_stats' function */
struct stats {
  unsigned long user_ticks;
  unsigned long system_ticks;
  unsigned long blocked_ticks;
  unsigned long ready_ticks;
  unsigned long elapsed_total_ticks;
  unsigned long total_trans; /* Number of times the process has got the CPU:
                                READY->RUN transitions */
  unsigned long remaining_ticks;
};

void stats_user_to_system();
void stats_system_to_user();
void stats_system_to_ready();
void stats_ready_to_system();
void stats_system_to_blocked();
void stats_blocked_to_ready(struct task_struct *thread);

#endif /* !STATS_H */
