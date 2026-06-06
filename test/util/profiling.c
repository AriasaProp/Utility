#include "profiling.h"
#if !defined(NO_STDC)
  #include <time.h>
#endif // NO_STDC

#define SEC_IN_NS   1000000000ULL
#define SEC_IN_US      1000000ULL
#define SEC_IN_MS         1000ULL

#define SEC_TO_NS   1000000000ULL
#define  MS_TO_NS      1000000ULL
#define  US_TO_NS         1000ULL

#define LONG_SEC 1000000000000ULL

pr_time profiling_current_time() {
  struct timespec tv;
  if (clock_gettime(CLOCK_MONOTONIC, &tv) == -1)
    return clock() * (SEC_IN_NS / CLOCKS_PER_SEC);
  return tv.tv_sec * SEC_IN_NS + tv.tv_nsec;
}
pr_time profiling_time_since  (pr_time s) {
  struct timespec tv;
  if (clock_gettime(CLOCK_MONOTONIC, &tv) == -1)
    return (clock() * (SEC_IN_NS / CLOCKS_PER_SEC)) - s;
  return (tv.tv_sec * SEC_IN_NS + tv.tv_nsec) - s;
}
// usually print 2 unit time
void  profiling_append_as_time2 (String *str, pr_time s) {
  int count = 0;
  if (s > SEC_TO_NS) {
    string_append(str, "%lus", s / SEC_TO_NS);
    if (s > LONG_SEC) return;
    string_append_cstr(str, " ", 1);
    s %= SEC_TO_NS;
    ++count;
  }
  if (s > MS_TO_NS) {
    string_append(str, "%03lums", s / MS_TO_NS);
    s %= MS_TO_NS;
    if (count++) return;
    else string_append_cstr(str, " ", 1);
  }
  if (s > US_TO_NS) {
    string_append(str, "%03luus", s / US_TO_NS);
    s %= US_TO_NS;
    if (count++) return;
    else string_append_cstr(str, " ", 1);
  }
  if (s) string_append(str, "%03luns", s);
  else if (!count) string_append(str, "0");
}
void  profiling_append_as_time  (String *str, pr_time s) {
  if (s > SEC_TO_NS) {
    string_append(str, "%lus", s / SEC_TO_NS);
  } else if (s > MS_TO_NS) {
    string_append(str, "%03lums", s / MS_TO_NS);
  } else if (s > US_TO_NS) {
    string_append(str, "%03luus", s / US_TO_NS);
  } else {
    string_append(str, "%03luns", s);
  }
}
double profiling_as_dsec(pr_time s) { return (double)s / 1000000000.0; }
float  profiling_as_fsec(pr_time s) { return (float)s / 1000000000.0f; }
double profiling_as_dms(pr_time s) { return (double)s / 1000000.0; }
float  profiling_as_fms(pr_time s) { return (float)s / 1000000.0f; }
double profiling_as_dus(pr_time s) { return (double)s / 1000.0; }
float  profiling_as_fus(pr_time s) { return (float)s / 1000.0f; }
