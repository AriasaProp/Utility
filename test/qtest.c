#include <stdio.h>
#include <stdint.h>
#include "util/console_out.h"
#include "common.h"
#include "math/bigInteger.h"

int main() {
  String str = NULL;
  bigInteger a = bigInteger_from_cstr("1272677979517283508059040439013026201563327218445133958644");
  bigInteger b = bigInteger_from_cstr("31199663619599895025687631330");
  bigInteger c = bigInteger_from_cstr("40791400671313604730524349082");
  
  bigInteger res = {0};
  bigInteger rem = {0};
  bigInteger sim = {0};
#define WORD_BITS sizeof(word) * 8
  {
    word t, t1;
    iter i;
    da_rforeach(word, ia, &a) {
      for (i = WORD_BITS; i--; ) {
        t1 = ((*ia >> i) & 1);
        da_foreach(word, irem, &rem) {
          t = *irem;
          *irem <<= 1;
          *irem |= t1;
          t1 = (t >> (WORD_BITS - 1)) & 1;
        }
        if (t1) da_append(&rem, t1);
        t1 = (bigInteger_cmp(rem, b) >= 0);
        if (t1) {
          {
            string_clean(str);
            bigInteger_append_string(&str, rem);
            PRINT_INF("cur: %45s\n", str);
          }
          {
            string_clean(str);
            bigInteger_append_string(&str, b);
            PRINT_INF("sub: %45s\n", str);
          }
          bigInteger_msub(&rem, b);
          {
            string_clean(str);
            bigInteger_append_string(&str, rem);
            PRINT_INF(" = : %45s\n", str);
          }
          bigInteger_set (&sim, rem);
          bigInteger_madd(&sim, b);
          {
            string_clean(str);
            bigInteger_append_string(&str, sim);
            PRINT_INF("sim: %45s\n", str);
          }
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
    {
      string_clean(str);
      bigInteger_append_string(&str, res);
      PRINT_INF("res:"GREEN" %45s\n"RESET, str);
    }
  }
  int cmp = bigInteger_cmp(c, res);
  string_clean(str);
  bigInteger_append_string(&str, c);
  PRINT_INF("exp: %s%45s\n"RESET, cmp ? RED : GREEN, str);
  
  string_free(&str);
  bigInteger_free(&a);
  bigInteger_free(&b);
  bigInteger_free(&c);
  bigInteger_free(&rem);
  bigInteger_free(&res);
  return 0;
}
