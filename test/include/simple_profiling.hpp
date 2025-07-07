#ifndef SIMPLE_PROFILING_
#define SIMPLE_PROFILING_

#include <ctime>
#include <ostream>

struct simple_time_t {
  double to_sec ();
  simple_time_t operator+ (const simple_time_t) const;
  simple_time_t &operator+= (const simple_time_t);
  friend std::ostream &operator<< (std::ostream &, const simple_time_t &);
  std::clock_t t;
};

struct simple_timer_t {
  simple_timer_t ();
  void start ();
  simple_time_t end ();
  std::clock_t safe_time;
};

void print_resources (std::ostream &);

#endif // SIMPLE_PROFILING_