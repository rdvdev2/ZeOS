#include <sched.h>
#include <utils.h>

inline void _leave_user(struct task_struct * thread) {
  unsigned long current_ticks = get_ticks();
  thread->st.user_ticks += current_ticks - thread->st.elapsed_total_ticks;
  thread->st.elapsed_total_ticks = current_ticks;
}

inline void _leave_system(struct task_struct * thread) {
  unsigned long current_ticks = get_ticks();
  thread->st.system_ticks += current_ticks - thread->st.elapsed_total_ticks;
  thread->st.elapsed_total_ticks = current_ticks;
}

inline void _leave_ready(struct task_struct * thread) {
  unsigned long current_ticks = get_ticks();
  thread->st.ready_ticks += current_ticks - thread->st.elapsed_total_ticks;
  thread->st.elapsed_total_ticks = current_ticks;
}

inline void _leave_blocked(struct task_struct * thread) {
  unsigned long current_ticks = get_ticks();
  thread->st.blocked_ticks += current_ticks - thread->st.elapsed_total_ticks;
  thread->st.elapsed_total_ticks = current_ticks;
}

void stats_user_to_system() { _leave_user(current()); }

void stats_system_to_user() { _leave_system(current()); }

void stats_system_to_ready() { _leave_system(current()); }

void stats_ready_to_system() {
  _leave_ready(current());
  ++current()->st.total_trans;
}

void stats_system_to_blocked() {
  _leave_system(current());
}

void stats_blocked_to_ready(struct task_struct * thread) {
  _leave_blocked(thread);
}
