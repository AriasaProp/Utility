#ifndef CLOCK_ADJUSTMENT_
#define CLOCK_ADJUSTMENT_

struct clock_adjustment {
public:
  enum period {
    hours,
    minutes,
    seconds,
    milliseconds,
    microseconds
  };
  clock_adjustment(const char*);
  unsigned long get_clock(const period);
  ~clock_adjustment();
private:
  struct private_data data;
};

#endif //CLOCK_ADJUSTMENT_