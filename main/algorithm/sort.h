/* *****************************************************************************
 * sort.h v0.0.0000
 * 
 * Sorting algorithma
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef _SORT_INCLUDED_
#define _SORT_INCLUDED_

#include "common.h"

typedef int(*compare_funct)(const void*,const void*);

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// list of sorting algorithm
void sort_stooge (void*, iter, iter,compare_funct);
void sort_brick  (void*, iter, iter,compare_funct);
void sort_gnome  (void*, iter, iter,compare_funct);
void sort_quick  (void*, iter, iter,compare_funct);
void sort_shaker (void*, iter, iter,compare_funct);
void sort_bubble (void*, iter, iter,compare_funct);
void sort_intro  (void*, iter, iter,compare_funct);
void sort_panck  (void*, iter, iter,compare_funct);
void sort_select (void*, iter, iter,compare_funct);
void sort_heap   (void*, iter, iter,compare_funct);
void sort_shell  (void*, iter, iter,compare_funct);
void sort_insert (void*, iter, iter,compare_funct);
void sort_merge  (void*, iter, iter,compare_funct);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _SORT_INCLUDED_
