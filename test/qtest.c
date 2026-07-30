#include "util/console_out.h"
#include "common.h"
#include <byteswap.h>

int main() {
  PRINT_INF("Hello, QTest!\n");
  int i = 0xfedcba98, j;
  j = i;
  util_memflip(&j, 4);
  
  PRINT_INF("initial %x\n", i);
  PRINT_INF("bswap   %x\n", bswap_32(i));
  PRINT_INF("bltn    %x\n", __builtin_bswap32(i));
  PRINT_INF("flip    %x\n", j);
  
  return 0;
}
