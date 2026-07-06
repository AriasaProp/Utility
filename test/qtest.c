#include "util/console_out.h"
#include "common.h"
#include "array/dstring.h"

bool bigger(const dstring a, const dstring b) {
  
  if (dstring_len(a) != dstring_len(b)) return dstring_len(a) > dstring_len(b);
  for (iter i = 0; i < dstring_len(a); ++i) {
    if (a[i] != b[i]) return a[i] > b[i]; 
  }
  return true;
}
bool subtract(dstring a, const dstring b) {
  if (!dstring_len(a)) return false;
  for (iter i = dstring_len(a) - dstring_len(b), j = 0, k; j < dstring_len(b); ++i, ++j) {
    char sub = b[j];
    k = i;
    do {
      while (a[k] < sub) {
        a[k] += 10 - (sub - '0');
        sub = 1;
      } else {
        a[k] += '0' - sub;
        break;
      }
    } while (k--);
  }
  return true;
}


int main() {
  dstring str = NULL;
  dstring a = NULL, b = NULL, c = NULL;
  dstring res = NULL, rem = NULL;
  dstring_append(&a, "42849112547792878652658951705");
  dstring_append(&b, "16455251965419843997");
  dstring_append(&c, "2604051373");
  
  #define WORD_BITS sizeof(word) * 8
  {
    iter shift = 0;
    for (iter i = 0; i < dstring_len(a); ++i) {
      dstring_append_char(&rem, a[i]);
      bool big = bigger(rem, b);
      
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
  PRINT_INF("R: %45s\n", res);
  PRINT_INF("%%: %45s\n", rem);
  PRINT_INF("B: %45s\n", b);
  PRINT_INF("C: %45s\n", c);
  
  dstring_free(&a);
  dstring_free(&b);
  dstring_free(&c);
  dstring_free(&res);
  dstring_free(&rem);
  return 0;
}
