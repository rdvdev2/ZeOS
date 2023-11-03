#include <sched.h>
#include <utils.h>

inline void _leave_user() {
  unsigned long current_ticks = get_ticks();
  current()->st.user_ticks += current_ticks - current()->st.elapsed_total_ticks;
  current()->st.elapsed_total_ticks = current_ticks;
}

inline void _leave_system() {
  unsigned long current_ticks = get_ticks();
  current()->st.system_ticks +=
      current_ticks - current()->st.elapsed_total_ticks;
  current()->st.elapsed_total_ticks = current_ticks;
}

inline void _leave_ready() {
  unsigned long current_ticks = get_ticks();
  current()->st.ready_ticks +=
      current_ticks - current()->st.elapsed_total_ticks;
  current()->st.elapsed_total_ticks = current_ticks;
}

void stats_user_to_system() { _leave_user(); }

void stats_system_to_user() { _leave_system(); }

void stats_system_to_ready() { _leave_system(); }

void stats_ready_to_system() {
  _leave_ready();
  ++current()->st.total_trans;
}
