#include "util/console_out.h"
#include "common.h"

int cmp (int *i, int *j) { return (*i > *j) - (*i < *j); }

int main(int argc, char **argv) {
  iter m;
  int t;
  int data[10] = {8,0,7,4,1,3,9,2,5,6};
  PRINT_INF("[Data]:");
  for (m = 0; m < 10; ++m)
    printf(" %d", data[m]);
  printf("\n");
  int *start = data, *end = data + 10;
  int o; // order
  int *i, *j; // point
  int *a, *b, *p;
  while (start < end) {
    // left
    o = 0;
    a = start;
    i = start, j = i;
    while (++j < end && (
      ( o && ((o * cmp(i, j)) >= 0)) ||
      (!o && ((o = cmp(i, j)) || true))
    )) i = j;
    PRINT_INF("[ar%02d]:", o);
    for (p = start; p <= i; ++p)
      printf(" %d", *p);
    printf("\n");
    if (start > data) {
      int *J = i;
      // merge
      if (o < 0) { // ascending
        start -= 1;
        while (i > start && start >= data) {
          if (cmp(i, start) < 0) {
            t = *start;
            memcpy(start, start + 1, (i - start) * sizeof(int));
            *i = t;
            --start;
          }
          --i;
        }
      } else { // descending
        while (start <= i) { // go next
          a = b = start;
          while (a > data && cmp(a - 1, b) > 0) // go prev
            a -= 1;
          if (a < b) {
            t = *b;
            memmove(a + 1, a, (b - a) * sizeof(int));
            *a = t;
            PRINT_INF("[rotl]:");
            for (p = a; p <= b; ++p)
              printf(" %d", *p);
            printf("\n");
          }
          ++start;
        }
      }
      PRINT_INF("[Mr%02d]:", o);
      for (p = data; p <= J; ++p)
        printf(" %d", *p);
      printf("\n");
    } else if (o > 0) { // reverse descending order
      a = start, b = i;
      while (a < b) {
        t = *a, *a = *b, *b = t;
        ++a, --b;
      }
      PRINT_INF("[ro%02d]:", o);
      for (p = start; p <= i; ++p)
        printf(" %d", *p);
      printf("\n");
    }
    start = j;
  }
  PRINT_INF("[Data]:");
  for (m = 0; m < 10; ++m)
    printf(" %d", data[m]);
  printf("\n");
  return 0;
}