/* ================================
 *
 *  DString Function
 * ================================
 */

#include "array/dstring.h"


typedef char *dstring;

// should be exponent of 2
#define DSTRING_CAP_ROUND 4
#define DSTRING_CAP_MASK  3

typedef struct {iter cap, count; } dstring_head;

static inline dstring_head *dstring__get_head(dstring str) {
  if (str) return (CAST(dstring_head*)str) - 1;
  return CAST(dstring_head*)calloc(sizeof(dstring_head), 1);
}
static inline dstring dstring__get_string(dstring_head *sh) {
  return CAST(dstring) (!sh ? NULL : (sh + 1));
}
static inline dstring_head *dstring__reserve(dstring_head *sh, iter need) {
  need = (need & ~DSTRING_CAP_MASK) + DSTRING_CAP_ROUND * !!(need & DSTRING_CAP_MASK);
  if (!sh || (sh->cap < need)) {
    sh = CAST(dstring_head*) realloc(sh, need + sizeof(dstring_head));
    ASSERT(sh && "string fail to allocate");
    sh->cap = need;
  }
  return sh;
}
inline void dstring_append_char(dstring *str,char c) {
  dstring_head *sh = dstring__get_head(*str);
  sh = dstring__reserve(sh, sh->count + 2);
  *str = dstring__get_string(sh);
  (*str)[sh->count++] = c;
  (*str)[sh->count] = 0;
}
inline void dstring_append_cstr(dstring *str, const char *cstr, iter len) {
  dstring_head *sh = dstring__get_head(*str);
  sh = dstring__reserve(sh, sh->count + len + 1);
  *str = dstring__get_string(sh);
  memcpy(*str + sh->count, cstr, len);
  // may memcpy end with null?
  (*str)[sh->count += len] = 0;
}
inline void dstring_append(dstring *str, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int r = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  ASSERT((r >= 0) && "invalid on vsnprintf");
  r += 1;
  dstring_head *sh = dstring__get_head(*str);
  sh = dstring__reserve(sh, sh->count + r);
  *str = dstring__get_string(sh);
  va_start(args, fmt);
  r = vsnprintf(*str + sh->count, r, fmt, args);
  va_end(args);
  sh->count += r;
}
bool dstring_equal(const dstring a,const dstring b) {
	const iter l = dstring_len(a);
	return (dstring_len(b) == l) && !memcmp(a,b,l);
}
inline void dstring_reserve(dstring *str, iter sz) {
  *str = dstring__get_string(dstring__reserve(dstring__get_head(*str), sz + 1));
}
inline iter dstring_len(const dstring str) {
  const dstring_head *sh = dstring__get_head(str);
  return sh ? sh->count : 0;
}
inline void dstring_clean(dstring str) {
  dstring_head *sh = dstring__get_head(str);
  if (!sh || !sh->count) return;
  sh->count = 0;
  *(CAST(char*)str) = 0;
}
inline void dstring_free (dstring *str) {
  free(dstring__get_head(*str));
  *str = NULL;
}

