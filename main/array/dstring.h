/* ================================
 *  dstring Functions
 * ================================
 */
#ifndef _DSTRING_INCLUDED_
#define _DSTRING_INCLUDED_

#include "common.h"

typedef char *dstring;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void dstring_append_char(dstring*,char);
void dstring_append_cstr(dstring*,const char *, iter);
void dstring_append     (dstring*,const char *, ...);
bool dstring_equal      (const dstring,const dstring);
void dstring_reserve    (dstring*,iter);
iter dstring_len        (const dstring);
void dstring_clean      (dstring);
void dstring_free       (dstring*);

#ifdef __cplusplus
}
#endif

#endif // _DSTRING_INCLUDED_