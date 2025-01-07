#include "simple_clock.hpp"

#include <sstream>
#include <vector>

void simple_timer_t::start () {
  safe_time = clock ();
}
simple_time_t simple_timer_t::end () {
  return simple_time_t{clock () - safe_time};
}

std::ostream &operator<< (std::ostream &o, const simple_time_t &t) {
  static const char *unit[]{"m", "s", "ms", "ns", "us"};
  unsigned short arr[5];

  long double td = t.t / CLOCKS_PER_SEC;
  arr[0] = static_cast<unsigned short> (td * 0.05 / 3.0);
  td -= arr[0] * 60;
  arr[1] = static_cast<unsigned short> (td);
  td -= arr[1];
  td *= 1000.0;
  arr[2] = static_cast<unsigned short> (td);
  td -= arr[2];
  td *= 1000.0;
  arr[3] = static_cast<unsigned short> (td);
  td -= arr[3];
  td *= 1000.0;
  arr[4] = static_cast<unsigned short> (td);
	
  std::stringstream ss;
  for (unsigned int i = 0; i < 5; ++i) {
    if (arr[i])
      ss << " " << arr[i] << " " << unit[i];
  }
  o << ss.str ();

  return o;
}
