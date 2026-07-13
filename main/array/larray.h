#ifndef _LINKED_ARRAY_INCLUDED_
#define _LINKED_ARRAY_INCLUDED_
/* ************************************************
 * Linked Array
 *
 * ************************************************ */
#include "common.h"
typedef struct larray larray;

#ifdef __cplusplus
extern "C" {
#endif

larray larray_new(void);
larray larray_del(larray*);

#ifdef __cplusplus
}
#endif

#endif // _LINKED_ARRAY_INCLUDED_