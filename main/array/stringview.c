/* ================================
 *  String View Functions
 * 
 * 
 * ================================ */

#include "array/stringview.h"

#define NO_NULL(X) if (!X) return (StringView){0}
// generate StringView from String
StringView stringview_str(const dstring s) {
  NO_NULL(s);
  return (StringView){.count = string_len(s), .cstr = CAST(const char*)s};
}
// chop by whitespace
StringView stringview_chop(const dstring s) {
  return stringview_chop_char(s, CHAR_WHITESPACE);
}
StringView stringview_chop_char(const dstring s,const char d) {
  NO_NULL(s);
  StringView ret = {.count = 0, .cstr = CAST(const char*)s};
  for (iter i = string_len(s); (ret.count < i) && s[ret.count] && (s[ret.count] != d); ++(ret.count)) ;
  return ret;
}
StringView stringview_chop_chars(const dstring s,const char *d) {
  NO_NULL(s);
  NO_NULL(d);
  StringView ret = {.count = 0, .cstr = CAST(const char*)s};
  for (iter i = string_len(s),j; (ret.count < i) && s[ret.count]; ) {
    for (j = 0; d[j]; ++j) if (d[j] == s[ret.count]) return ret;
    ++(ret.count);
  }
  return ret;
}
StringView stringview_chop_cstr(const dstring s,const char *cstr) {
  NO_NULL(s);
  NO_NULL(cstr);
  StringView ret = {.count = 0, .cstr = CAST(const char*)s};
  for (iter i = string_len(s), j = util_strlen(cstr); ((i - ret.count) > j) && (ret.count < i) && s[ret.count] && !util_memcmp(s + ret.count, cstr, j); ++(ret.count)) ;
  return ret;
}
// StringView helper
int stringview_equal (const dstringView a,const dstringView b) {
  int ret = (a.count>b.count) - (a.count<b.count);
  if (!ret) ret = util_memcmp(a.cstr, b.cstr, a.count);
  return ret;
}
