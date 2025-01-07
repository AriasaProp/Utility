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
  unsigned int r = static_cast<unsigned int> (td / 60.0);
  if (r) ss << r << " M", non = false;
  td -= t * 60;
  t = static_cast<unsigned int> (td);
  if (r) ss << (non ? "" : ", ") << r << " s", non = false;
  td -= t;
  td *= 1000.0;
  t = static_cast<unsigned int> (td);
  if (r) ss << (non ? "" : ", ") << r << " ms", non = false;
  td -= t;
  td *= 1000.0;
  t = static_cast<unsigned int> (td);
  if (r || non) ss << (non ? "" : ", ") << r << " us";
  o << ss.str ();
  return o;
}
