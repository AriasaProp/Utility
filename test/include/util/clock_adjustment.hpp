#ifndef CLOCK_ADJUSTMENT_
#define CLOCK_ADJUSTMENT_

namespace profiling {

struct clock_adjustment {
public:
  enum period {
    hours,
    minutes,
    seconds,
    milliseconds,
    microseconds
  };
  clock_adjustment (const char *);
  unsigned long get_clock (const period);
  ~clock_adjustment ();

private:
  struct private_data;
  private_data *data;
};

} // namespace profiling

#endif // CLOCK_ADJUSTMENT_