/* *****************************************************************************
 * sort.c v0.0.0000
 * 
 * Sorting algorithma
 * 
 * 
 * 
 * *****************************************************************************/

#include "algorithm/sort.h"
#define NO_NULL        if (!dat || (size < 2) || !bytes || !cmp) return

/* ==========================================================
 * stooge sort
 * worst, resursively correcting data order head and tail
 * then call itself 3 times
 * ==========================================================*/
void sort_stooge(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
	char *data = CAST(char*)dat;
	char *end = data + (size - 1) * bytes;
	if (cmp(data, end) > 0)
	  util_memswap(data, end, bytes);
	if ((data + bytes) >= end) return;
	iter k = size / 3;
	sort_stooge(data            , size - k, bytes, cmp);
	sort_stooge(data + k * bytes, size - k, bytes, cmp);
	sort_stooge(data            , size - k, bytes, cmp);
}
/* ==========================================================
 * brick sort
 * sort data artenatly by index odd even
 * loop until sorted
 * ==========================================================*/
void sort_brick(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
	char *data = CAST(char*)dat, *end = data + (size - 1) * bytes, *i, *j;
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
 * gnome sort
 * worst version of insertion sort with no nested loop
 * do more evaluation when back to prev->next
 * 
 * ==========================================================*/
void sort_gnome(void *dat, iter size, iter bytes, compare_funct cmp) {
	NO_NULL;
	char *data = CAST(char*)dat;
	for (char *i = data, *j = data + (size - 1) * bytes; i < j;) {
	  if ((i >= data) && (cmp(i, i + bytes) > 0)) {
		  util_memswap(i, i + bytes, bytes);
	    i -= bytes;
	  } else
	    i += bytes;
	}
}
/* ==========================================================
 * quick sort
 * 
 * pick end data as pivot to put data on left and right
 * 
 * 
 * ==========================================================*/
void sort_quick(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  char *data = CAST(char*)dat;
  char *pivot = data + (size - 1) * bytes;
  iter c = 0;
  char *i = data, *j = data;
  while (j < pivot) {
    if (cmp(j, pivot) < 0) {
      util_memswap(i, j, bytes);
      i += bytes;
      ++c;
    }
    j += bytes;
  }
  util_memswap(i, pivot, bytes);
  sort_quick(data, c, bytes, cmp);
  if (size > c) sort_quick(i + bytes, size - c, bytes, cmp);
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
	char *start = CAST(char*)dat, *end = start + (size - 1) * bytes, *i;
	int swapped = 1;
	while (swapped) {
	  for (i = start; i < end; i += bytes) {
	    if (cmp(i, i + bytes) > 0) {
	      util_memswap(i, i + bytes, bytes);
	      swapped = 1;
	    }
	  }
	  if (!swapped) break;
    swapped = 0;
	  end -= bytes;
	  for (i = end; i > start; i -= bytes) {
	    if (cmp(i - bytes, i) > 0) {
	      util_memswap(i - bytes, i, bytes);
	      swapped = 1;
	    }
	  }
	  start += bytes;
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
	char *data = (char*)dat;
	for(char *i = data + size * bytes, *j; (i -= bytes) >= data; ) // start move
		for (j = data; j < i; j += bytes) // go prev
			if (cmp(j, j + bytes) > 0)
			  util_memswap(j, j + bytes, bytes);
}
/* ==========================================================
 * intro sort
 * introspective sort
 * split sort divide task by quick, insertion and heap sort
 * 
 * 
 * ==========================================================*/
void sort_intro(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  if (size <= 16) sort_insert(dat, size, bytes, cmp);
  else if (!CAST(int)imath_floor(2*imath_log(size)))
    sort_heap(dat, size, bytes, cmp);
  else {
    char *d = CAST(char*)dat;
    char *pivot = d + (size - 1) * bytes;
    iter c = 0;
    char *i = d, *j = d;
    while (j < pivot) {
      if (cmp(j, pivot) < 0) {
        util_memswap(i, j, bytes);
        i += bytes;
        ++c;
      }
      j += bytes;
    }
    util_memswap(i, pivot, bytes);
    sort_intro(d, c, bytes, cmp);
    if (size > c) sort_intro(i + bytes, size - c, bytes, cmp);
  }
}
/* ==========================================================
 * pancake sort
 * do data flip for data range
 *
 * ==========================================================*/
void sort_panck(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  char *max, *i, *j, *data = CAST(char*)dat, *end;
  while (size > 1) {
    // find biggest
    --size;
    end = data + size * bytes;
    for (i = end, max = end; (i -= bytes) >= data; )
      if (cmp(max, i) < 0) max = i;
    if (max == end) continue;
    for (i = data, j = max; i < j; i += bytes, j -= bytes)
      util_memswap(i, j, bytes);
    for (i = data, j = data + size * bytes; i < j; i += bytes, j -= bytes)
      util_memswap(i, j, bytes);
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
	char *data = (char*)dat;
	for(char *i = data,*j,*k = data + size * bytes, *min; i < k; i += bytes) { // loop right
		for (j = i, min = i; (j += bytes) < k; ) // loop left
			if (cmp(j, min) < 0) min = j;
		if (min != i) util_memswap(i, min, bytes);
	}
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
	char *data = (char*)dat;
	iter i, prnt, nd;
  while (size > 1) {
    // get parent
    i = size >> 1;
    // first iter may had less children
    prnt = --i;
    // left , should always exit
    nd = (prnt << 1) + 1;
    if (cmp(data + nd * bytes, data + prnt * bytes) < 0) prnt = nd;
    // right
    if ((++nd < size) && (cmp(data + nd * bytes, data + prnt * bytes) < 0)) prnt = nd;
    if (prnt != i) util_memswap(data + prnt * bytes, data + i * bytes, bytes);
    // next will always had
    while (i--) {
      prnt = i;
      // left , should always exit
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
	char *data = (char*)dat;
	for(char *i = data, *j, *k = data + size * bytes; (i += bytes) < k; ) // loop next
		for (j = i; (j > data) && (cmp(j, j - bytes) < 0); j -= bytes) // loop prev
			util_memswap(j, j - bytes, bytes);
}
/* ==========================================================
 * shell sort
 * do insertion sort with halfed gap size between compared data
 * 
 * 
 * ==========================================================*/
void sort_shell(void *dat, iter size, iter bytes, compare_funct cmp) {
	NO_NULL;
	char *data = (char*)dat, *end = data + size * bytes, *i,*j;
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
void sort_merge(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
	char *a = CAST(char*)dat;
	iter i = size >> 1;
	char *b = a + i * bytes, *d = a + size * bytes;
	sort_merge(a, i, bytes, cmp);
	sort_merge(b, size - i, bytes, cmp);
  char *c = CAST(char*)util_alloca(bytes);
  b -= bytes;
	while ((a <= b) && (b < (d -= bytes))) {
	  if (cmp(b, d) <= 0) continue;
		util_memcpy (c, b, bytes);
		util_memcpy (b, b + bytes, d - b);
		util_memcpy (d, c, bytes);
    b -= bytes;
	}
}