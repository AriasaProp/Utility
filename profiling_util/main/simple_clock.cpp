#include "simple_clock.hpp"

#include <sstream>

void simple_timer_t::start () {
  safe_time = clock ();
}
simple_time_t simple_timer_t::end () {
  return simple_time_t{clock () - safe_time};
}

std::ostream &operator<< (std::ostream &o, const simple_time_t &t) {
  static const char *unit[]{"m", "s", "ms", "us"};
  double td = t.t * 1.0 / CLOCKS_PER_SEC;
  bool non = true;
  std::stringstream ss;
  unsigned int t = static_cast<unsigned int> (td / 60.0);
  if (t) ss << t << " M", non = false;
  td -= t * 60;
  t = static_cast<unsigned int> (td);
  if (t) ss << (non ? "" : ", ") << t << " s", non = false;
  td -= t;
  td *= 1000.0;
  t = static_cast<unsigned int> (td);
  if (t) ss << (non ? "" : ", ") << t << " ms", non = false;
  td -= t;
  td *= 1000.0;
  t = static_cast<unsigned int> (td);
  if (t || non) ss << (non ? "" : ", ") << t << " us";
  o << ss.str ();
  return o;
}
