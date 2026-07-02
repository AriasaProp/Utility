#include "math/bigInteger.h"
#include "util/console_out.h"
#include "common.h"
#include "array/darray.h"
#include "array/dstring.h"

int main() {
  dstring str = NULL;
  bigInteger a = bigInteger_from_cstr("320354604123224817850834072440270195965071568756");
  bigInteger b = bigInteger_from_cstr("26809178147998024201567467132");
  bigInteger c = bigInteger_from_cstr("11949437702070971651");
  
  bigInteger res = {0};
  bigInteger rem = {0};
  // bigInteger sim = {0};
  #define WORD_BITS sizeof(word) * 8
  {
    iter shift = 0;
    da_rforeach(word, ia, &a) {
      for (iter i = WORD_BITS; i--; ) {
        word t, t1;
        t1 = (*ia >> i) & 1;
        da_foreach(word, irem, &rem) {
          t = *irem;
          *irem <<= 1;
          *irem |= t1;
          t1 = (t >> (WORD_BITS - 1)) & 1;
        }
        if (t1) da_append(&rem, t1);
        t1 = bigInteger_cmp(rem, b) >= 0;
        if (t1) {
          if (shift) {
            printf("<: %010zu\n", shift);
            shift = 0;
          }
          dstring_clean(str);
          bigInteger_append_dstring(&str, rem);
          printf("*: %40s\n", str);
          dstring_clean(str);
          bigInteger_append_dstring(&str, b);
          printf("-: %40s\n", str);
          bigInteger_msub(&rem, b);
          dstring_clean(str);
          bigInteger_append_dstring(&str, rem);
          printf("%%: %40s\n", str);
        } else {
          shift++;
        }
        da_foreach(word, ires, &res) {
          t = *ires;
          *ires <<= 1;
          *ires |= t1;
          t1 = (t >> (WORD_BITS - 1)) & 1;
        }
        if (t1) da_append(&res, t1);
      }
    }
    if (shift) {
      printf("<: %010zu\n", shift);
    }
  }
  if (!bigInteger_cmp(res,c))
    PRINT_INF("cnmp: √\n");
  dstring_clean(str);
  bigInteger_append_dstring(&str, res);
  PRINT_INF("R: %45s\n", str);
  dstring_clean(str);
  bigInteger_append_dstring(&str, rem);
  PRINT_INF("%%: %45s\n", str);
  dstring_clean(str);
  bigInteger_append_dstring(&str, b);
  PRINT_INF("B: %45s\n", str);
  dstring_clean(str);
  bigInteger_append_dstring(&str, c);
  PRINT_INF("C: %45s\n", str);
  
  dstring_free(&str);
  bigInteger_free(&a);
  bigInteger_free(&b);
  bigInteger_free(&c);
  bigInteger_free(&rem);
  bigInteger_free(&res);
  return 0;
}
