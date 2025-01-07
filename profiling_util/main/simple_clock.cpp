#include "simple_clock.hpp"

#include <vector>
#include <sstream>

simple_time_t::simple_time_t (){};
simple_time_t::simple_time_t (signed a) : t (a){};
simple_time_t::simple_time_t (unsigned long a) : t (a){};
simple_time_t::simple_time_t (simple_time_t &a) : t (a.t){};

simple_time_t &simple_time_t::operator= (signed a) {
  t = a;
  return *this;
}
simple_time_t &simple_time_t::operator= (unsigned long a) {
  t = a;
  return *this;
}
simple_time_t &simple_time_t::operator= (simple_time_t &a) {
  t = a.t;
  return *this;
}

simple_time_t &simple_time_t::operator+= (signed a) {
  t += a;
  return *this;
}
simple_time_t &simple_time_t::operator+= (unsigned long a) {
  t += a;
  return *this;
}
simple_time_t &simple_time_t::operator+= (simple_time_t &a) {
  t += a.t;
  return *this;
}

void simple_timer_t::start () {
  safe_time = time_type::now ();
}
simple_time_t simple_timer_t::end () {
  unsigned long tm = std::chrono::duration_cast<std::chrono::microseconds> (time_type::now () - safe_time).count ();
  return simple_time_t (tm);
}


std::ostream &operator<< (std::ostream &o, const simple_time_t &t) {
  static const char *unit[]{"us", "ns", "ms", "s", "m", "h", "D", "Week", "M"};
  static const int td[]{1000, 1000, 1000, 60, 60, 24, 7, 30 / 7, 12};
  unsigned long l = t.t, i = 0;
  std::vector<unsigned long> arr;
  do {
    if (i > 9)
      break;
    arr.push_back (l % td[i]);
  } while ((l /= td[i++]) != 0);
  std::stringstream ss;
  for (auto x = arr.rbegin (), y = std::min (x + 3, arr.rend ()); x < y; ++x) {
    ss << " " << *x << " " << unit[--i];
  }
  o << ss.str ();

  return o;
}
