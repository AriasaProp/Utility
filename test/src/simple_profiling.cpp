#include "simple_profiling.hpp"

#include <sys/resource.h>
#include <iomanip>
#include <sstream>

simple_time_t simple_time_t::operator+ (const simple_time_t a) const {
  return simple_time_t{t+a.t};
}
simple_time_t &simple_time_t::operator+= (const simple_time_t a) {
  t += a.t;
  return *this;
}
std::ostream &operator<< (std::ostream &o, const simple_time_t &t) {
  std::stringstream ss;
  long td = t.t;
  static const int CLOCKS_PER_MS = CLOCKS_PER_SEC / 1000;
  static const int CLOCKS_PER_US = CLOCKS_PER_MS / 1000;
  unsigned int writed = 0;
  if (td > CLOCKS_PER_SEC) {
    ss << (td / CLOCKS_PER_SEC) << "s";
    td %= CLOCKS_PER_SEC;
    ++writed;
  }
  if (td > CLOCKS_PER_MS) {
    if (writed) ss << ":";
    ss << std::setw (3) << std::setfill ('0') << (td / CLOCKS_PER_MS) << "ms";
    td %= CLOCKS_PER_MS;
    ++writed;
  }
  if ((writed <= 1) && (td || !writed)) {
    if (writed) ss << ":";
    ss << std::setw (3) << std::setfill ('0') << (td / CLOCKS_PER_US) << "us";
  }
  o << ss.str ();
  return o;
}

double simple_time_t::to_sec () {
  return (double)t / CLOCKS_PER_SEC;
}
simple_timer_t::simple_timer_t () {
  safe_time = clock ();
}
void simple_timer_t::start () {
  safe_time = clock ();
}
simple_time_t simple_timer_t::end () {
  return simple_time_t{clock () - safe_time};
}

void print_resources (std::ostream &o) {
  static struct rusage usage;
  o << "Resource Usage for Current Process:\n-----------------------------------\n";
  if (getrusage(RUSAGE_SELF, &usage) >= 0) {
#define PU(A, B) if (B) o << A << ": " << B << "\n"
    
    o << "User CPU time used: " << usage.ru_utime.tv_sec << "." << usage.ru_utime.tv_usec << " sec\n";
    o << "System CPU time used: " << usage.ru_stime.tv_sec << "." << usage.ru_stime.tv_usec << " sec\n";
    PU("Maximum resident set size (kilobytes)", usage.ru_maxrss);
    PU("Integral shared memory size (kilobytes)", usage.ru_ixrss);
    PU("Integral unshared data size (kilobytes)", usage.ru_idrss);
    PU("Integral unshared stack size (kilobytes)", usage.ru_isrss);
    PU("Page reclaims (soft page faults)", usage.ru_minflt);
    PU("Page faults (hard page faults)", usage.ru_majflt);
    PU("Swaps", usage.ru_nswap);
    PU("Block input operations", usage.ru_inblock);
    PU("Block output operations", usage.ru_oublock);
    PU("IPC messages sent", usage.ru_msgsnd);
    PU("IPC messages received", usage.ru_msgrcv);
    PU("Signals received", usage.ru_nsignals);
    PU("Voluntary context switches", usage.ru_nvcsw);
    PU("Involuntary context switches", usage.ru_nivcsw);
    
#undef PU
  } else
    o << "Error getting resource usage: " << strerror(errno) << "\n";
}

