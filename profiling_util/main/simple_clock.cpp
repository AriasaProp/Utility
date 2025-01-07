#include "simple_clock.hpp"

#include <sstream>
double simple_time_t::to_sec () {
  return t * 1.0 / CLOCKS_PER_SEC;
}

void simple_timer_t::start () {
  safe_time = clock ();
}
simple_time_t simple_timer_t::end () {
  return simple_time_t{clock () - safe_time};
}

std::ostream &operator<< (std::ostream &o, const simple_time_t &t) {
  std::stringstream ss;
  double td = t.t * 1.0 / CLOCKS_PER_SEC;
  unsigned int writed = 0;
  unsigned int r = static_cast<unsigned int> (td);
  if (r) ss << (writed ? "" : ", ") << r << " s", ++writed;
  if (writed > 1) return o;
  td -= r;
  td *= 1000.0;
  r = static_cast<unsigned int> (td);
  if (r) ss << (writed ? "" : ", ") << std::setw (3) << std::setfill ('0') << r << " ms", ++writed;
  if (writed > 1) return o;
  td -= r;
  td *= 1000.0;
  r = static_cast<unsigned int> (td);
  if (r || (writed == 0)) ss << (writed ? "" : ", ") << std::setw (3) << std::setfill ('0') << r << " us";
  o << ss.str ();
  return o;
}
