#ifndef _DARRAY_INCLUDED_
#define _DARRAY_INCLUDED_
/* ================================
 *  dynamic array 
 *  Apply in macro
 *  ARRAY object format at least contain this
 * struct {
 *   T *items;
 *   iter cap;
 *   iter count;
 * } T's;
 *  
 * ================================
 */
#include "common.h"

#define darray_roundSize   8
#define darray_roundMask   7

#define darray_reserve(a, need) do { \
  if ((need) <= (a)->cap) break; \
  (a)->cap = ((need) & ~darray_roundMask) + darray_roundSize * !!((need) & darray_roundMask); \
  (a)->items = util_realloc((a)->items, (a)->cap * sizeof(*(a)->items)); \
  ASSERT((a)->items && "Fail to allocate heap"); \
} while (0)
#define darray_atleast(a, z) do { \
  if ((a)->count >= (z)) break;\
  darray_reserve((a), (z)); \
  util_memset((a)->items + (a)->count, 0, ((z)-(a)->count) * sizeof(*(a)->items)); \
  (a)->count = (z); \
} while (0)
#define darray_copy(a,b) do {\
  darray_reserve((a),(b)->count);\
  util_memcpy((a)->items, (b)->items, (b)->count * sizeof(*(a)->items)); \
  (a)->count = (b)->count; \
} while (0)
#define darray_append(a, item) do { \
  darray_reserve((a), (a)->count + 1); \
  (a)->items[(a)->count++] = (item); \
} while (0)
#define darray_free(a) do {\
  if ((a)->items) util_memfree((a)->items);\
  util_memset((a), 0, sizeof(*a)); \
} while (0)
#define darray_clean(a) (a)->count = 0
#define darray_appends(a, b, l) do { \
  darray_reserve((a), (a)->count + (l)); \
  util_memcpy((a)->items + (a)->count, (b), (l)*sizeof(*(a)->items)); \
  (a)->count += (l); \
} while (0)
#define darray_index(a, i) (a)->items[(ASSERT((i) < (a)->count), (i))]
#define darray_last(a) (a)->items[(ASSERT((a)->count), (a)->count - 1)]
#define darray_pop(a) (a)->items[(ASSERT((a)->count), --(a)->count)]
#define darray_unorder_remove(a, j) (a)->items[(j)] = (a)->items[(ASSERT((j) < (a)->count), --(a)->count)]
#define darray_remove(a, j) do { \
  ASSERT((j) < (a)->count); \
  if ((j) < --(a)->count) \
    util_memcpy(&(a)->items[(j)], &(a)->items[(j) + 1], (a)->count - (j));\
} while(0)
#define darray_foreach(T, it, a) for (T *it = (a)->items, *__end = (a)->items + (a)->count; it < __end; ++it)
#define darray_rforeach(T, it, a) for (T *it = (a)->items + (a)->count; (it--) > (a)->items; )



#endif // _DARRAY_INCLUDED_