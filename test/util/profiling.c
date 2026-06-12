#include "profiling.h"
#include <time.h>

#define CLK_IN_US   1000000ULL / CLOCKS_PER_SEC

#define SEC_TO_US   1000000ULL
#define  MS_TO_US      1000ULL

// microsecond
pr_time profiling_current_time() {
  return clock() * CLK_IN_US;
}
pr_time profiling_time_since  (pr_time s) {
  return profiling_current_time() - s;
}
// usually print 2 unit time
void  profiling_append_as_time2 (String *str, pr_time s) {
  int count = 0;
  if (s > SEC_TO_US) {
    string_append(str, "%lus", s / SEC_TO_US);
    string_append_char(str, ' ');
    s %= SEC_TO_US;
    ++count;
  }
  if (s > MS_TO_US) {
    string_append(str, "%03lums", s / MS_TO_US);
    s %= MS_TO_US;
    if (count++) return;
    else string_append_char(str, ' ');
  }
  if (count < 2) string_append(str, "%03luus", s);
}
void  profiling_append_as_time  (String *str, pr_time s) {
  if (s > SEC_TO_US) string_append(str, "%lu s", s / SEC_TO_US);
  else if (s > MS_TO_US) string_append(str, "%03lums", s / MS_TO_US);
  else string_append(str, "%03luus", s);
}
double profiling_as_dsec(pr_time s) { return (double)s / 1000000.0; }
float  profiling_as_fsec(pr_time s) { return (float)s / 1000000.0f; }
double profiling_as_dms(pr_time s) { return (double)s / 1000.0; }
float  profiling_as_fms(pr_time s) { return (float)s / 1000.0f; }
double profiling_as_dus(pr_time s) { return (double)s; }
float  profiling_as_fus(pr_time s) { return (float)s; }
