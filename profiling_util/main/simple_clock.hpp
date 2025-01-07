#ifndef SIMPLE_CLOCK_
#define SIMPLE_CLOCK_

#include <chrono>
#include <iostream>

typedef std::chrono::steady_clock time_type;

struct simple_time_t {
private:
  unsigned long t;

public:
  simple_time_t ();
  simple_time_t (signed);
  simple_time_t (unsigned long);
  simple_time_t (simple_time_t &);

  simple_time_t &operator= (signed);
  simple_time_t &operator= (unsigned long);
  simple_time_t &operator= (simple_time_t &);

  simple_time_t &operator+= (signed);
  simple_time_t &operator+= (unsigned long);
  simple_time_t &operator+= (simple_time_t &);

  friend std::ostream &operator<< (std::ostream &, const simple_time_t &);
};

struct simple_timer_t {
private:
  std::chrono::time_point<time_type> safe_time;

public:
  void start ();
  simple_time_t end ();
};

#endif // SIMPLE_CLOCK_