#ifndef _LINKED_ARRAY_INCLUDED_
#define _LINKED_ARRAY_INCLUDED_
/* ************************************************
 * Linked Array
 *
 * ************************************************ */
#include "common.h"
typedef struct larray larray;


larray larray_new(void);
larray larray_del(larray*);

#endif // _LINKED_ARRAY_INCLUDED_