/* *****************************************************************************
 * sort.c v0.0.0000
 * 
 * Sorting algorithma
 * 
 * 
 * 
 * *****************************************************************************/

#include "algorithm/sort.h"
#define NO_NULL        if (!(dat && bytes && cmp)) return
#define SIZE_ELIM(p)   do {\
  switch (size) { \
    case 2:\
      if (cmp((p), (p) + bytes) > 0) util_memswap((p), (p) + bytes, bytes);\
    case 0: case 1: return; \
    default: break;\
  }\
}  while (0)

/* ==========================================================
 * stooge sort
 * worst, resursively correcting data order head and tail
 * then call itself 3 times
 * ==========================================================*/
static void sort_stooge_rec(byte *, iter, iter, compare_funct);
void sort_stooge(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  sort_stooge_rec(CAST(byte*)dat, size,bytes, cmp);
}
static void sort_stooge_rec(byte *d, iter size, iter bytes, compare_funct cmp) {
  SIZE_ELIM(d);
	byte *end = d + (size - 1) * bytes;
	if (cmp(d, end) > 0) util_memswap(d, end, bytes);
	if ((d + bytes) >= end) return;
	iter k = size / 3;
	sort_stooge_rec(d            , size - k, bytes, cmp);
	sort_stooge_rec(d + k * bytes, size - k, bytes, cmp);
	sort_stooge_rec(d            , size - k, bytes, cmp);
}
/* ==========================================================
 * pancake sort
 * do data flip for data range
 *
 * ==========================================================*/
void sort_panck(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  byte *max, *i, *j, *data = CAST(byte*)dat, *end;
  while (size-- > 1) {
    // find biggest
    end = data + size * bytes;
    for (i = max = end; (i -= bytes) >= data; )
      if (cmp(max, i) < 0) max = i;
    if (max == end) continue;
    for (i = data, j = max; i < j; i += bytes, j -= bytes)
      util_memswap(i, j, bytes);
    for (i = data, j = data + size * bytes; i < j; i += bytes, j -= bytes)
      util_memswap(i, j, bytes);
  }
}
/* ==========================================================
 * gnome sort
 * worst version of insertion sort with no nested loop
 * do more evaluation when back to prev->next
 * 
 * ==========================================================*/
void sort_gnome(void *dat, iter size, iter bytes, compare_funct cmp) {
	NO_NULL;
	byte *data = CAST(byte*)dat;
	for (byte *i = data, *j = data + (size - 1) * bytes; i < j;) {
	  if ((i >= data) && (cmp(i, i + bytes) > 0)) {
		  util_memswap(i, i + bytes, bytes);
	    i -= bytes;
	  } else
	    i += bytes;
	}
}
/* ==========================================================
 * brick sort
 * sort data artenatly by index odd even
 * loop until sorted
 * ==========================================================*/
void sort_brick(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
	byte *data = CAST(byte*)dat, *end = data + (size - 1) * bytes, *i, *j;
	for (int sorted = 0, bin = 1; !sorted; bin = !bin) {
	  sorted |= 1;
	  for (i = data + bin * bytes, j = i + bytes; i < end; i += bytes * 2, j += bytes * 2) {
	    if (cmp(i, j) <= 0) continue;
      util_memswap(i, j, bytes);
      sorted &= 0;
	  }
	}
}
/* ==========================================================
 * bubble sort
 * do next evaluate with bring sorted data to end
 * # # .. .. .. .. # # # 
 * ------------------>
 * continue do evaluate till end with less loop
 * 
 * ==========================================================*/
void sort_bubble(void *dat, iter size, iter bytes, compare_funct cmp) {
	NO_NULL;
	byte *data = (byte*)dat;
	for(byte *i = data + size * bytes, *j; (i -= bytes) >= data; ) // start move
		for (j = data; j < i; j += bytes) // go prev
			if (cmp(j, j + bytes) > 0)
			  util_memswap(j, j + bytes, bytes);
}
/* ==========================================================
 * shaker sort
 * do check and swap from start to end,
 * then bounce back from end to start
 * # # .. .. .. .. # # # 
 *   ------------------>
 * <------------------
 * like optimize bubble short
 * 
 * ==========================================================*/
void sort_shaker(void *dat, iter size, iter bytes, compare_funct cmp) {
	NO_NULL;
	byte *start = CAST(byte*)dat, *end = start + (size - 1) * bytes, *i;
	int scramb = 1;
	while (scramb) {
    scramb = 0;
	  for (i = start; i < end; i += bytes) {
	    if (cmp(i, i + bytes) <= 0) continue;
      util_memswap(i, i + bytes, bytes);
      scramb = 1;
	  }
	  end -= bytes;
	  if (!scramb) break;
    scramb = 0;
	  for (i = end; i > start; i -= bytes) {
	    if (cmp(i - bytes, i) <= 0) continue;
      util_memswap(i - bytes, i, bytes);
      scramb = 1;
	  }
	  start += bytes;
	}
}
/* ==========================================================
 * insertion sort
 * start evaluate from first data, if sorted move next
 * # # # .. .. .. .. .. ->
 * if not, swap then evaluate prev
 * .. .. .. .. <- # # # .. ..   ^
 * continue evaluate next data  |
 * .. .. .. # # # -> .. .. ..
 * 
 * ==========================================================*/
void sort_insert(void *dat, iter size, iter bytes, compare_funct cmp) {
	NO_NULL;
	byte *data = (byte*)dat;
	for(byte *i = data, *j, *k = data + size * bytes; (i += bytes) < k; ) // loop next
		for (j = i; (j > data) && (cmp(j, j - bytes) < 0); j -= bytes) // loop prev
			util_memswap(j, j - bytes, bytes);
}
/* ==========================================================
 * heap sort
 * using tree node
 *                                 0
 *                1________________|_________________2
 *        3_______|_______4                 5________|________6
 *    7___|___8       9___|___10       11___|___12       13___|___14
 * 15_|_16 17_|_18 19_|_20 21_|_22  23_|_24  25_|_26  27_|_28  29_|_30
 *
 *  climb target up to top
 *  reduce tree array from front view
 *
 * ==========================================================*/
void sort_heap(void *dat, iter size, iter bytes, compare_funct cmp) {
	NO_NULL;
	byte *data = (byte*)dat;
	iter i, prnt, nd;
  while (size > 1) {
    // get parent
    i = size >> 1;
    // first iter may had less children
    prnt = --i;
    // left , should always exists
    nd = (prnt << 1) + 1;
    if (cmp(data + nd * bytes, data + prnt * bytes) < 0) prnt = nd;
    // right
    if ((++nd < size) && (cmp(data + nd * bytes, data + prnt * bytes) < 0)) prnt = nd;
    if (prnt != i) util_memswap(data + prnt * bytes, data + i * bytes, bytes);
    // next will always had
    while (i--) {
      prnt = i;
      // left , should always exists
      nd = (prnt << 1) + 1;
      if (cmp(data + nd * bytes, data + prnt * bytes) < 0) prnt = nd;
      // right
      if (cmp(data + (++nd) * bytes, data + prnt * bytes) < 0) prnt = nd;
      if (prnt != i) util_memswap(data + prnt * bytes, data + i * bytes, bytes);
    }
    data += bytes;
    --size;
  }
}
/* ==========================================================
 * selection sort
 * # # # .. ..  ..
 * -->
 * evaluate to find minimum each iteration
 * # # # .. _ ..
 *  <-------
 * swap it to the first
 *
 * ==========================================================*/
void sort_select(void *dat, iter size, iter bytes, compare_funct cmp) {
	NO_NULL;
	byte *data = (byte*)dat;
	for(byte *i = data,*j,*k = data + size * bytes, *min; i < k; i += bytes) { // loop right
		for (j = i, min = i; (j += bytes) < k; ) // loop left
			if (cmp(j, min) < 0) min = j;
		if (min != i) util_memswap(i, min, bytes);
	}
}
/* ==========================================================
 * shell sort
 * do insertion sort with halfed gap size between compared data
 * 
 * 
 * ==========================================================*/
void sort_shell(void *dat, iter size, iter bytes, compare_funct cmp) {
	NO_NULL;
	byte *data = (byte*)dat, *end = data + size * bytes, *i,*j;
	iter sb;
  while (size >>= 1) {
    for (sb = size * bytes, i = data + sb; i < end; i += bytes)
      for(j = i; (j > data) && (cmp(j, j - sb) < 0); j -= sb)
        util_memswap(j, j - sb, bytes);
  }
}
/* ==========================================================
 * merge sort
 * data was split and sorted each part
 * { -1 ... 1 } { -1 ... 1 }
 * 
 * ==========================================================*/
static void sort_merge_rec(byte *, iter, iter, compare_funct, byte*);
void sort_merge(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  byte *c = CAST(byte*)util_malloc(bytes);
	sort_merge_rec(CAST(byte*)dat, size, bytes, cmp, c);
	util_memfree(c);
}
static void sort_merge_rec(byte *a, iter size, iter bytes, compare_funct cmp, byte *c) {
  SIZE_ELIM(a);
	iter i = size >> 1;
	byte *b = a + i * bytes, *d = a + size * bytes;
	sort_merge_rec(a, i, bytes, cmp, c);
	sort_merge_rec(b, size - i, bytes, cmp, c);
  b -= bytes;
	while ((a <= b) && (b < (d -= bytes))) {
	  if (cmp(b, d) <= 0) continue;
		util_memcpy (c, b, bytes);
		util_memcpy (b, b + bytes, d - b);
		util_memcpy (d, c, bytes);
    b -= bytes;
	}
}
/* ==========================================================
 * intro sort
 * introspective sort
 * split sort divide task by quick, insertion and heap sort
 * 
 * 
 * ==========================================================*/
static void sort_intro_rec(byte *, iter, iter, compare_funct);
void sort_intro(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  sort_intro_rec(CAST(byte*)dat, size, bytes, cmp);
}
static void sort_intro_rec(byte *d, iter size, iter bytes, compare_funct cmp) {
  SIZE_ELIM(d);
  if (size <= 16) {
    // insertion sort internal
  	for(byte *i = d, *j, *k = d + size * bytes; (i += bytes) < k; ) // loop next
  		for (j = i; (j > d) && (cmp(j, j - bytes) < 0); j -= bytes) // loop prev
  		  util_memswap(j, j - bytes, bytes);
  } else if (!CAST(int)imath_floor(2*imath_log(size))) {
  	// heap sort internal
  	iter i, prnt, nd;
    do {
      // get parent
      i = size >> 1;
      // first iter may had less children
      prnt = --i;
      // left , should always exists
      nd = (prnt << 1) + 1;
      if (cmp(d + nd * bytes, d + prnt * bytes) < 0) prnt = nd;
      // right
      if ((++nd < size) && (cmp(d + nd * bytes, d + prnt * bytes) < 0)) prnt = nd;
      if (prnt != i) util_memswap(d + prnt * bytes, d + i * bytes, bytes);
      // next will always had
      while (i--) {
        prnt = i;
        // left , should always exists
        nd = (prnt << 1) + 1;
        if (cmp(d + nd * bytes, d + prnt * bytes) < 0) prnt = nd;
        // right
        if (cmp(d + (++nd) * bytes, d + prnt * bytes) < 0) prnt = nd;
        if (prnt != i) util_memswap(d + prnt * bytes, d + i * bytes, bytes);
      }
      d += bytes;
    } while (--size);
  } else {
    // call partition function to find Partition Index
    // Initialize pivot to be the first element
    iter i = 1, j = size - 1;
    while (i < j) {
      // Find the first element greater than the pivot (from starting)
      // first element are pivot, so skip
      while ((i < size - 1) && (cmp(d, d + i * bytes) > 0))
        ++i;
      // Find the first element smaller than the pivot (from last)
      while (j && (cmp(d + j * bytes, d) > 0))
        --j;
      if (i < j) util_memswap(d + i * bytes, d + j * bytes, bytes);
    }
    util_memswap(d, d + j * bytes, bytes);
    // Recursively call quickSort() for left and right
    // half based on Partition Index
    sort_intro_rec(d, j, bytes, cmp);
    if (size > j + 2)
      sort_intro_rec(d + (j + 1) * bytes,  size - (j + 1), bytes, cmp);
  }
}
/* ==========================================================
 * quick sort
 * 
 * pick end data as pivot to put data on left and right
 * 
 * 
 * ==========================================================*/
static void sort_quick_rec(byte *, iter, iter, compare_funct);
void sort_quick(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  sort_quick_rec(CAST(byte*)dat, size, bytes, cmp);
}
static void sort_quick_rec(byte *d, iter size, iter bytes, compare_funct cmp) {
  SIZE_ELIM(d);
  // call partition function to find Partition Index
  // Initialize pivot to be the first element
  iter i = 1, j = size - 1;
  while (i < j) {
    // Find the first element greater than the pivot (from starting)
    // first element are pivot, so skip
    while ((i < size - 1) && (cmp(d, d + i * bytes) > 0))
      ++i;
    // Find the first element smaller than the pivot (from last)
    while (j && (cmp(d + j * bytes, d) > 0))
      --j;
    if (i < j) util_memswap(d + i * bytes, d + j * bytes, bytes);
  }
  util_memswap(d, d + j * bytes, bytes);
  // Recursively call quickSort() for left and right
  // half based on Partition Index
  sort_quick_rec(d, j, bytes, cmp);
  if (size > j + 2)
    sort_quick_rec(d + (j + 1) * bytes,  size - (j + 1), bytes, cmp);
}