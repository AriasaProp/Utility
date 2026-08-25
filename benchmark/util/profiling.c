#include "profiling.h"
#include <time.h>


#define SEC_IN_NS   1000000000
#define  MS_IN_NS      1000000
#define  US_IN_NS         1000


// microsecond
pr_time profiling_current_time(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    return 0;
  return CAST(pr_time)ts.tv_sec * SEC_IN_NS + CAST(pr_time)ts.tv_nsec;
}
pr_time profiling_time_since  (pr_time s) {
  return profiling_current_time() - s;
}
// usually print 2 unit time
void  profiling_append_as_time2 (dstring *str, pr_time s) {
  int count = 0;
  if (s > SEC_IN_NS) {
    dstring_append(str, "%lus", s / SEC_IN_NS);
    dstring_append_char(str, ' ');
    s %= SEC_IN_NS;
    ++count;
  }
  if (s > MS_IN_NS) {
    dstring_append(str, "%03lums", s / MS_IN_NS);
    s %= MS_IN_NS;
    if (count++) return;
    else dstring_append_char(str, ' ');
  }
  if (s > US_IN_NS) {
    dstring_append(str, "%03luus", s / US_IN_NS);
    s %= US_IN_NS;
    if (count++) return;
    else dstring_append_char(str, ' ');
  }
  if (count < 2) dstring_append(str, "%03luns", s);
}
void  profiling_append_as_time  (dstring *str, pr_time s) {
  if (s > SEC_IN_NS) dstring_append(str, "%lu s", s / SEC_IN_NS);
  else if (s > MS_IN_NS) dstring_append(str, "%03lums", s / MS_IN_NS);
  else if (s > US_IN_NS) dstring_append(str, "%03luus", s / US_IN_NS);
  else dstring_append(str, "%03luns", s);
}
double profiling_as_dsec(pr_time s) { return (double)s / SEC_IN_NS; }
float  profiling_as_fsec(pr_time s) { return  (float)s / SEC_IN_NS; }
double profiling_as_dms(pr_time s) { return (double)s /  MS_IN_NS; }
float  profiling_as_fms(pr_time s) { return  (float)s /  MS_IN_NS; }
double profiling_as_dus(pr_time s) { return (double)s / US_IN_NS; }
float  profiling_as_fus(pr_time s) { return  (float)s / US_IN_NS; }
double profiling_as_dns(pr_time s) { return (double)s; }
float  profiling_as_fns(pr_time s) { return  (float)s; }
