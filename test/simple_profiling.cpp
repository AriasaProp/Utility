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

