/* *****************************************************************************
 * sort.c v0.0.0000
 * 
 * Sorting algorithm
 * 
 * 
 * 
 * *****************************************************************************/

#include "algorithm/sort.h"

#define NO_NULL        if (!dat || !bytes || size < 2 || !cmp) return
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
 * @ISSUE: Highly recursive
 *
 *     120,27,104,103,13,118,90,46,
 *
 *   (120,27,104,103,13,118) - 90,46,
 * ((120,27,104,103) - 13,118) - 90,46,
 *
 * (((120,27 - 104) - 103) - 13,118) - 90,46,
 *
 *
 * ==========================================================*/
static void sort_stooge_rec(byte *, iter, iter, compare_funct);
void sort_stooge(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  sort_stooge_rec(CAST(byte*)dat, size, bytes, cmp);
}
static void sort_stooge_rec(byte *d, iter size, iter bytes, compare_funct cmp) {
  byte *c;
  if (size < 3) {
    if (size > 1 && cmp(d, (c = d + bytes)) > 0)
      util_memswap(d, c, bytes);
    return;
  }
  iter k = size / 3;
  size -= k;
  c = d + k * bytes;
  sort_stooge_rec(d, size, bytes, cmp);
  sort_stooge_rec(c, size, bytes, cmp);
  sort_stooge_rec(d, size, bytes, cmp);
}
/* ==========================================================
 * pancake sort
 * do data flip for data range
 *
 * 0,8,12,0,16,4,16,16,4,12,8,12,4,8,0,16,12,0,8,16
 * 0,8,12,0,16,4,16,16,4,12,8,12,4,8,0,16,12,0,8,16
 *
 *
 *
 * ==========================================================*/
void sort_panck(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  byte *i, *j,
    *t = CAST(byte*)alloca(bytes),
    *data = CAST(byte*)dat,
    *end = data + size * bytes;
  while ((end -= bytes) > data) {
    // find biggest
    i = j = end;
    while (i >= data) {
      if (cmp(j, i) < 0) j = i;
      i -= bytes;
    }
    // don't flip the pan, max value was in order
    if (j < end) {
      i = data;
      while (i < j) {
        memcpy(t, i, bytes);
        memcpy(i, j, bytes);
        memcpy(j, t, bytes);
        i += bytes, j -= bytes;
      }
      i = data, j = end; 
      while (i < j) {
        memcpy(t, i, bytes);
        memcpy(i, j, bytes);
        memcpy(j, t, bytes);
        i += bytes, j -= bytes;
      }
    }
  }
}
/* ==========================================================
 * cycle sort
 * determine an element index by how much element less than
 * 
 * 0,8,12,0,16,4,16,16,4,12,8,12,4,8,0,16,12,0,8,16 : 0(0)
 *   8,12,0,16,4,16,16,4,12,8,12,4,8,0,16,12,0,8,16 : 8(6)
 *   16,12,0,16,4,16,8,4,12,8,12,4,8,0,16,12,0,8,16 : 16(15)
 *   12,12,0,16,4,16,8,4,12,8,12,4,8,0,16,16,0,8,16 : 12(11)
 *   4,12,0,16,4,16,8,4,12,8,12,12,8,0,16,16,0,8,16 : 4(3)
 *   16,12,0,4,4,16,8,4,12,8,12,12,8,0,16,16,0,8,16 : 16(16)
 *   0,12,0,4,4,16,8,4,12,8,12,12,8,0,16,16,16,8,16 : 0(0)
 *     12,0,4,4,16,8,4,12,8,12,12,8,0,16,16,16,8,16 : 12(11)
 *     8,0,4,4,16,8,4,12,8,12,12,12,0,16,16,16,8,16 : 8(6)
 *     4,0,4,4,16,8,8,12,8,12,12,12,0,16,16,16,8,16 : 4(4)
 *     16,0,4,4,4,8,8,12,8,12,12,12,0,16,16,16,8,16 : 16(16)
 *     8,0,4,4,4,8,8,12,8,12,12,12,0,16,16,16,16,16 : 8(7)
 *     12,0,4,4,4,8,8,8,8,12,12,12,0,16,16,16,16,16 : 12(12)
 *     0,0,4,4,4,8,8,8,8,12,12,12,12,16,16,16,16,16,
 *     0,4,4,4,8,8,8,8,12,12,12,12,16,16,16,16,16,
 *     4,4,4,8,8,8,8,12,12,12,12,16,16,16,16,16,
 *     4,4,8,8,8,8,12,12,12,12,16,16,16,16,16,
 *     4,8,8,8,8,12,12,12,12,16,16,16,16,16,
 *     8,8,8,8,12,12,12,12,16,16,16,16,16,
 *     8,8,8,12,12,12,12,16,16,16,16,16,
 *     8,8,12,12,12,12,16,16,16,16,16,
 *     8,12,12,12,12,16,16,16,16,16,
 *     12,12,12,12,16,16,16,16,16,
 *     12,12,12,16,16,16,16,16,
 *     12,12,16,16,16,16,16,
 *     12,16,16,16,16,16,
 *     16,16,16,16,16,
 *     16,16,16,16,
 *     16,16,16,
 *     16,16,
 * 0,0,0,0,4,4,4,8,8,8,8,12,12,12,12,16,16,16,16,16,
 *
 *
 * ==========================================================*/
void sort_cycle(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  byte *i, *j,
    *t = CAST(byte*)alloca(bytes),
    *d = CAST(byte*)dat,
    *e = d + (size - 1) * bytes;
  while (d < e) {
    i = d, j = e;
    while (j > d) {
      i += bytes * (cmp(d, j) > 0);
      j -= bytes;
    }
    if (i > d) {
      while (!cmp(i, d))
        i += bytes;
      memcpy(t, i, bytes);
      memcpy(i, d, bytes);
      memcpy(d, t, bytes);
    } else d += bytes;
  }
}
/* ==========================================================
 * brick sort
 * sort data artenatly by pair index odd even
 * loop until sorted
 * ==========================================================*/
void sort_brick(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  bool sorted[2] = {0}, sgn = false;
  byte *i, *j,
    *data = CAST(byte*)dat,
    *end = data + size * bytes,
    *t = CAST(byte*)alloca(bytes);
  iter b2 = 2 * bytes;
  do {
    sorted[sgn] = true;
    i = data + sgn * bytes;
    j = i + bytes;
    while (j < end) {
      if (cmp(i, j) > 0) {
        // swap
        memcpy(t, i, bytes);
        memcpy(i, j, bytes);
        memcpy(j, t, bytes);
        sorted[sgn] = false;
      }
      i += b2, j += b2;
    }
    sgn = !sgn;
  } while (!(sorted[0] && sorted[1]));
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
  byte
    *t = CAST(byte*)alloca(bytes),
    *data = CAST(byte*)dat,
    *i = data + (size - 1) * bytes,*j, *k;
  while (i >= data) {// start move from end
    j = data;
    while (j < i) { // go prev
      k = j + bytes;
      if (cmp(j, k) > 0) {
        // swap
        memcpy(t, j, bytes);
        memcpy(j, k, bytes);
        memcpy(k, t, bytes);
      }
      j = k;
    }
    i -= bytes;
  }
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
  byte
    *start = CAST(byte*)dat,
    *end = start + (size - 1) * bytes,
    *i, *j, *k, *t = CAST(byte*)alloca(bytes);
  while (start < end) {
    // ------>
    i = start;
    j = start + bytes;
    k = end;
    while (i < end) {
      if (cmp(i, j) > 0) {
        memcpy(t, i, bytes);
        memcpy(i, j, bytes);
        memcpy(j, t, bytes);
        k = i;
      }
      i += bytes, j += bytes;
    }
    if (k == end) break;
    end = k;
    // <---------
    i = end;
    j = end - bytes;
    k = start;
    while (i > start) {
      if (cmp(j, i) > 0) {
        memcpy(t, i, bytes);
        memcpy(i, j, bytes);
        memcpy(j, t, bytes);
        k = i;
      }
      i -= bytes, j -= bytes;
    }
    if (k == start) break;
    start = k;
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
 *  this was an gnome sort
 * 
 * 
 *  optimization
 *  do rotate instead swap after find position
 * 
 * 0 1 2 3 7 8 9 4 6 5
 * 0 1 2 3 7 8 9 4 6 5
 * 
 * ==========================================================*/
void sort_insert(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  byte *j,
    *data = CAST(byte*)dat,
    *end = data + (size - 1) * bytes,
    *i = end - bytes,
    *t = CAST(byte*)alloca(bytes);
  while (i >= data) { // go next
    j = i;
    while (j < end && cmp(j + bytes, i) < 0) // go prev
      j += bytes;
    if (j > i) {
      memcpy(t, i, bytes);
      memcpy(i, i + bytes, j - i);
      memcpy(j, t, bytes);
    }
    i -= bytes;
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
  iter i;
  byte *a, *b,
    *data = CAST(byte*)dat,
    *t = CAST(byte*)alloca(bytes);
  while (size > 1) {
    // get parent
    i = size >> 1;
    if (i--) { // first
      b = data + ((i << 1) + 1) * bytes;
      a = data + (size - 1) * bytes;
      // cmp left and right
      // right may not exists
      b += (b < a && cmp(b, b + bytes) > 0) * bytes;
      a = data + i * bytes;
      if (cmp(a, b) > 0) {
        memcpy(t, a, bytes);
        memcpy(a, b, bytes);
        memcpy(b, t, bytes);
      }
    } else return;
    while (i--) {
      // cmp left and right
      b = data + ((i << 1) + 1) * bytes;
      b += (cmp(b, b + bytes) > 0) * bytes;
      a = data + i * bytes;
      if (cmp(a, b) > 0) {
        memcpy(t, a, bytes);
        memcpy(a, b, bytes);
        memcpy(b, t, bytes);
      }
    }
    data += bytes;
    --size;
  }
}
/* ==========================================================
 * tim sort
 * tim sort is a hybrid sorting algorithm use merge and insertion 
 * ratio determine by min run 32-64 data
 * optimize :
 * split data by order (ascending / descending).
 * reverse decending one.
 * merge them!
 * ==========================================================*/
void sort_tim(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  byte *a, *b, *i, *j, *k,
    *data = CAST(byte*)dat,
    *t = CAST(byte*)alloca(bytes),
    *end = data + size * bytes;
  int o; // order
  // first attempt
  o = 0, a = i = j = data;
  while ((j += bytes) < end && (
    ( o && ((o * cmp(i, j)) >= 0)) ||
    (!o && ((o = cmp(i, j)) || true))
  )) i = j;
  if (o > 0) { // first reverse
    b = i;
    while (a < b) {
      memcpy(t, a, bytes);
      memcpy(a, b, bytes);
      memcpy(b, t, bytes);
      a += bytes, b -= bytes;
    }
  }
  while (j < end) {
    o *= -1;
    // find sorted length
    i = k = j;
    while ((j += bytes) < end && (o * cmp(i, j)) >= 0)
      i = j;
    // merge
    if (o < 0) {
      k -= bytes;
      while (i > k && k >= data) {
        if (cmp(i, k) < 0) {
          memcpy(t, k, bytes);
          memcpy(k, k + bytes, i - k);
          memcpy(i, t, bytes);
          k -= bytes;
        }
        i -= bytes;
      }
    } else {
      while (k <= i) { // go next
        a = b = k;
        while (a > data && cmp(a - bytes, b) > 0) // go prev
          a -= bytes;
        if (a < b) {
          memcpy(t, b, bytes);
          memmove(a + bytes, a, b - a);
          memcpy(a, t, bytes);
        }
        k += bytes;
      }
    }
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
  byte *i,*j,
    *t = CAST(byte*)alloca(bytes),
    *data = CAST(byte*)dat,
    *end = data + size * bytes;
  while (data < end) { // loop right
    i = data;
    j = i + bytes;
    while (j < end) { // loop left
      if (cmp(j, i) < 0) i = j;
      j += bytes;
    }
    if (i > data) {
      memcpy(t, data, bytes);
      memcpy(data, i, bytes);
      memcpy(i, t, bytes);
    }
    data += bytes;
  }
}
/* ==========================================================
 * shell sort
 * do insertion sort with halfed gap size between compared data
 * 
 * 0,1,3,2,6,5,14,7,15,18,13,11,16,10,17,12,8,4,9,19,
 *                     18,                       ,19
 *                  15,                        ,9
 * 
 * 
 * ==========================================================*/
void sort_shell(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  byte *i,*j,*k,
    *t = CAST(byte*)alloca(bytes),
    *data = CAST(byte*)dat,
    *end = data + size * bytes;
  iter sb;
  while (size >>= 1) {
    sb = size * bytes;
    i = data + sb;
    while (i < end) {
      j = i, k = j - sb;
      while (k >= data && cmp(j, k) < 0) {
        memcpy(t, j, bytes);
        memcpy(j, k, bytes);
        memcpy(k, t, bytes);
        j = k, k -= sb;
      }
      i += bytes;
    }
  }
}
/* ==========================================================
 * merge sort
 * data was split and sorted each part
 * 0 4 1 2 6 5 3    n1
 * 0 1 4 2 6 3 5    n2
 * 0 1 4 2 3 5 6    n4
 * optimize: no recursion
 * 
 * 
 * ==========================================================*/
void sort_merge(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  iter i = bytes, n = size * bytes;
  byte *a, *b, *c, *A, *B,
    *t = CAST(byte*)alloca(bytes),
    *start = CAST(byte*)dat,
    *end = start + (size - 1) * bytes;
  // sort each i data
  while (i <= n) {
    a = end, b = a - i, c = b - i;
    c = MAX(start - bytes,c);
    i <<= 1;
    // split
    while (b >= start) {
      // merge
      // c < b < a
      A = a, B = b;
      while (B > c && B < A) {
        if (cmp(A, B) < 0) {
          memcpy(t, B, bytes);
          memcpy(B, B + bytes, A - B);
          memcpy(A, t, bytes);
          B -= bytes;
        }
        A -= bytes;
      }
      a -= i, b -= i, c -= i;
      c = MAX(start - bytes,c);
    }
  }
}
/* ==========================================================
 * intro sort
 * introspective sort is combination of different sorting algorithm
 * depends on size like insertion, heap and quick sort.
 * optimize:
 * do quick and heap alternately and then leave small data (<16) to insertion
 * this will receive less if condition
 *
 * ==========================================================*/
static void sort_intro_rec(byte*, iter, iter, compare_funct, byte*);
void sort_intro(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  byte *t = CAST(byte*)malloc(bytes);
  sort_intro_rec(CAST(byte*)dat, size, bytes, cmp,t);
  free(t);
}
static void sort_intro_rec(byte *data, iter size, iter bytes, compare_funct cmp, byte *t) {
  if (size < 16) {
    byte *j,
      *end = data + (size - 1) * bytes,
      *i = end - bytes;
    while (i >= data) { // go next
      j = i;
      while (j < end && cmp(j + bytes, i) < 0) // go prev
        j += bytes;
      if (j > i) {
        memcpy(t, i, bytes);
        memcpy(i, i + bytes, j - i);
        memcpy(j, t, bytes);
      }
      i -= bytes;
    }
  } else if (size & 1) {
    iter i = size >> 1;
    byte *a, *b;
    // get parent
    while (i--) {
      // cmp left and right
      b = data + ((i << 1) + 1) * bytes;
      b += (cmp(b, b + bytes) > 0) * bytes;
      a = data + i * bytes;
      if (cmp(a, b) > 0) {
        memcpy(t, a, bytes);
        memcpy(a, b, bytes);
        memcpy(b, t, bytes);
      }
    }
    sort_intro_rec(data + bytes, --size, bytes, cmp, t);
  } else {
    iter p = 0;
    // quick sort algorithm
    byte *i = data + (size - 1) * bytes, *j = i - bytes;
    do {
      if (cmp(i, j) < 0) {
        ++p;
        // i as pivot
        memcpy(t, j, bytes);
        memcpy(j, j + bytes, i - j);
        memcpy(i, t, bytes);
        i -= bytes;
      }
      j -= bytes;
    } while (j >= data);
    if ((size - p) > 2)
      sort_intro_rec(data, size - p - 1, bytes, cmp, t);
    if (p > 1)
      sort_intro_rec(i + bytes, p, bytes, cmp, t);
  }
}
/* ==========================================================
 * extra sort (not correct name)
 * alternate between heap and merge sort
 *
 *
 *
 * ==========================================================*/
static void sort_extra_rec(byte*, iter, iter, compare_funct, byte*);
void sort_extra(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  byte *t = CAST(byte*)malloc(bytes);
  sort_extra_rec(CAST(byte*)dat, size, bytes, cmp, t);
  free(t);
}
static void sort_extra_rec(byte *data, iter size, iter bytes, compare_funct cmp, byte *t) {
  if (size < 2) return;
  byte *a, *b;
  iter i = size >> 1;
  if (size & 1) { // heap
    // get parent
    while (i--) {
      // cmp left and right
      b = data + ((i << 1) + 1) * bytes;
      b += (cmp(b, b + bytes) > 0) * bytes;
      a = data + i * bytes;
      if (cmp(a, b) > 0) {
        memcpy(t, a, bytes);
        memcpy(a, b, bytes);
        memcpy(b, t, bytes);
      }
    }
    sort_extra_rec(data + bytes, size - 1, bytes, cmp, t);
  } else { // merge sort of even size data
    a = data + i * bytes;
    // split of even data
    sort_extra_rec(data, i, bytes, cmp, t);
    sort_extra_rec(a, i, bytes, cmp, t);
    // merge
    b = data + (size - 1) * bytes;
    a -= bytes;
    while (b > a && a >= data) {
      if (cmp(a, b) > 0) {
        memcpy(t, a, bytes);
        memcpy(a, a + bytes, b - a);
        memcpy(b, t, bytes);
        a -= bytes;
      }
      b -= bytes;
    }
  }
}
/* ==========================================================
 * quick sort
 * pick first; data as pivot
 * pick i as left, j as right
 * iterate i from left to right until bigger than end
 * iterate j from right to left until smaller than end
 * if i < j in position swap them
 * break loop when i >= j in position
 *
 * ==========================================================*/
static void sort_quick_rec(byte*, byte*, iter, compare_funct, byte*);
void sort_quick(void *dat, iter size, iter bytes, compare_funct cmp) {
  NO_NULL;
  byte *start = CAST(byte*)dat, *t = CAST(byte*)malloc(bytes);
  sort_quick_rec(start, start + (size - 1) * bytes, bytes, cmp, t);
  free(t);
}
static void sort_quick_rec(byte *a, byte *b, iter bytes, compare_funct cmp, byte *t) {
  if (b <= a) return;
  byte *i = a + bytes, *j = b;
  for (;;) {
    while (i <= b && cmp(a, i) >= 0)
      i += bytes;
    while (j > a && cmp(j, a) >= 0)
      j -= bytes;
    if (i < j) {
      memcpy(t, i, bytes);
      memcpy(i, j, bytes);
      memcpy(j, t, bytes);
    } else
      break;
  }
  if (j > a) {
    memcpy(t, a, bytes);
    memcpy(a, j, bytes);
    memcpy(j, t, bytes);
  }
  sort_quick_rec(a, j - bytes, bytes, cmp, t);
  sort_quick_rec(j + bytes, b, bytes, cmp, t);
}


