#include "util/console_out.h"
#include "common.h"

typedef struct {
	iter cap, count;
} foo;

#define reserve(a, l) do {\
	if ((a)->cap < ((a)->count + (l))) {\
		(a)->cap = ((a)->count + (l));\
  	PRINT_INF("count: %zu!\n", a->count);\
		(a)->cap = ((a)->cap & ~3) + !!((a)->cap & 3);\
	}\
  PRINT_INF("count: %zu!\n", a->count);\
  PRINT_INF("l: " #l "!\n");\
	(a)->count += (l);\
  PRINT_INF("count: %zu!\n", a->count);\
} while(0)

int main() {
  PRINT_INF("Hello, QTest!\n");
  
  foo *f = CAST(foo*)util_alloca(sizeof(foo));
  util_memset(f, 0, sizeof(foo));
  f->count = 1;
  reserve(f, 12);
  PRINT_INF("cap %zu!\n", f->cap);
  PRINT_INF("count %zu!\n", f->count);
  
  return 0;
}
