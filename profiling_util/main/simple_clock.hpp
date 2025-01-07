#ifndef SIMPLE_CLOCK_
#define SIMPLE_CLOCK_

#include <ctime>
#include <iostream>

struct simple_time_t {
  std::clock_t t;

  friend std::ostream &operator<< (std::ostream &, const simple_time_t &);
}

struct simple_timer_t {
private:
  std::clock_t safe_time;

public:
  void start ();
  simple_time_t end ();
};

#endif // SIMPLE_CLOCK_