/* *****************************************************************************
 * sort.c v0.0.0000
 * 
 * Sorting algorithma
 * 
 * 
 * 
 * *****************************************************************************/

#include "algorithm/sort.h"

#define VALID_ARRAY(Z) if ((Z) < 2) return

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
	VALID_ARRAY(size);
	char *data = (char*)dat;
	for(char *i = data,*j,*k = data + size * bytes, *min; i < k; i += bytes) { // loop right
		for (j = i + bytes, min = i; j < k; j += bytes) // loop left
			if (cmp(j, min) < 0) min = j;
		if (min != i) util_memswap(i, min, bytes);
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
	VALID_ARRAY(size);
	char *data = (char*)dat;
	for(char *i = data + size * bytes, *j; i > data; i -= bytes) // start move
		for (j = data; j < (i - bytes); j += bytes) // go prev
			if (cmp(j, j + bytes) > 0)
			  util_memswap(j, j + bytes, bytes);
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
	VALID_ARRAY(size);
	char *data = (char*)dat;
	for(char *i = data + bytes, *j, *k = data + size * bytes; i < k; i += bytes) // loop next
		for (j = i; (j > data) && (cmp(j, j - bytes) < 0); j -= bytes) // loop prev
			util_memswap(j, j - bytes, bytes);
}
/* ==========================================================
 * gnome sort
 * worst version of insertion sort with no nested loop
 * do more evaluation when back to prev->next
 * 
 * ==========================================================*/
void sort_gnome(void *dat, iter size, iter bytes, compare_funct cmp) {
	VALID_ARRAY(size);
	char *data = (char*)dat;
	for (char *i = data, *j = data + (size - 1) * bytes; i < j;) {
	  if ((i >= data) && (cmp(i, i + bytes) > 0)) {
		  util_memswap(i, i + bytes, bytes);
	    i -= bytes;
	  } else
	    i += bytes;
	}
}
/* ==========================================================
 * shell sort
 * do insertion sort with halfed gap size between compared data
 * 
 * 
 * ==========================================================*/
void sort_shell(void *dat, iter size, iter bytes, compare_funct cmp) {
	VALID_ARRAY(size);
	char *data = (char*)dat, *end = data + size * bytes, *i,*j;
	iter sb;
  while (size >>= 1)
    for (sb = size * bytes, i = data + sb; i < end; i += bytes)
      for(j = i; (j > data) && (cmp(j, j - sb) < 0); j -= sb)
        util_memswap(j, j - sb, bytes);
}
/* ==========================================================
 * merge sort
 * data was split and sorted each part
 * { -1 ... 1 } { -1 ... 1 }
 * 
 * ==========================================================*/
static void sort_merge__rec(char*, iter, iter, compare_funct,char*);
void sort_merge(void *dat, iter size, iter bytes, compare_funct cmp) {
	char *c = CAST(char*) util_malloc(bytes);
  sort_merge__rec(CAST(char*)dat, size, bytes, cmp, c);
  util_memfree(c);
}
static void sort_merge__rec(char *data, iter size, iter bytes, compare_funct cmp, char *c) {
  VALID_ARRAY(size);
	iter i = size >> 1;
	char *a = data,*b = data + i * bytes, *bend = data + size * bytes;
	sort_merge__rec(b, size - i, bytes, cmp, c);
	sort_merge__rec(a, i, bytes, cmp, c);
	while ((a<b) && (b<bend)) {
		// A bigger
		if (cmp(a,b)>0) {
			util_memcpy(c, b, bytes);
			util_memmove(a + bytes, a, i * bytes);
			util_memcpy(a, c, bytes);
		  b += bytes;
		}
		a += bytes;
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
	char *data = (char*)dat;
	iter i, prnt, nd;
  while (size > 1) {
    // climb target up to top
    i = size >> 1;
    while (i--) {
      prnt = i;
      nd = (prnt << 1) + 1;
      // left
      if (nd < size) {
        if (cmp(data + nd * bytes, data + prnt * bytes) < 0)
          prnt = nd;
        // right
        if ((++nd < size) && (cmp(data + nd * bytes, data + prnt * bytes) < 0))
          prnt = nd;
        if (prnt != i)
          util_memswap(data + prnt * bytes, data + i * bytes, bytes);
      }
    }
    data += bytes;
    --size;
  }
}



