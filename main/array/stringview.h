/* ================================
 *  String View Functions
 * ================================ */
#ifndef _STRINGVIEW_INCLUDED_
#define _STRINGVIEW_INCLUDED_

#include "common.h"
#include "array/dstring.h"


typedef struct { iter count; const char *cstr; } StringView;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// generate StringView from String
StringView stringview_str       (const dstring);
// chop by whitespace
StringView stringview_chop      (const dstring);
StringView stringview_chop_char (const dstring,const char);
StringView stringview_chop_chars(const dstring,const char*);
StringView stringview_chop_cstr (const dstring,const char*);
// StringView helper
int        stringview_equal     (const StringView,const StringView);

#ifdef __cplusplus
}
#endif

#endif  // _STRINGVIEW_INCLUDED_