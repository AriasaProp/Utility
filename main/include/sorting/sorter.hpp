#ifndef SORTER_INCLUDED_
#define SORTER_INCLUDED_

#include <cstdint>

// list of sorting algorithm
extern void insertion_sort(void*, size_t, size_t,int(*)(const void*,const void*));
extern void selection_sort(void*, size_t, size_t,int(*)(const void*,const void*));
extern void bubble_sort(void*, size_t, size_t,int(*)(const void*,const void*));
extern void merge_sort(void*, size_t, size_t,int(*)(const void*,const void*));

#endif // SORTER_INCLUDED_