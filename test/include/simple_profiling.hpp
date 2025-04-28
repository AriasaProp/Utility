#ifndef SIMPLE_PROFILING_
#define SIMPLE_PROFILING_

#include <ctime>
#include <ostream>

struct simple_time_t {
  std::clock_t t;

  double to_sec ();
  friend std::ostream &operator<< (std::ostream &, const simple_time_t &);
};

struct simple_timer_t {
private:
  std::clock_t safe_time;

public:
  simple_timer_t ();
  void start ();
  simple_time_t end ();
};

static struct mem_t {} mem_a;
std::ostream &operator<< (std::ostream &, mem_t &);

#endif // SIMPLE_PROFILING_