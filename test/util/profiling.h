#ifndef _PROFILING_UTIL_INCLUDED_
#define _PROFILING_UTIL_INCLUDED_

#include "common.h"

typedef ulong pr_time;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

pr_time profiling_current_time    ();
pr_time profiling_time_since      (pr_time);
double  profiling_as_dsec         (pr_time);
float   profiling_as_fsec         (pr_time);
double  profiling_as_dms          (pr_time);
float   profiling_as_fms          (pr_time);
double  profiling_as_dus          (pr_time);
float   profiling_as_fus          (pr_time);
void    profiling_append_as_time2 (String*, pr_time);
void    profiling_append_as_time  (String*, pr_time);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROFILING_UTIL_INCLUDED_