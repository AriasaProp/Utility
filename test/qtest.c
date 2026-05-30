#include <stdio.h>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  int i = 0b10000000101000001000011010010110;
  int j = __builtin_ffs(i); // find 1 from less index + 1
  printf("ffs: %d, %d\n", i, j);
  j = __builtin_clz(i);
  printf("clz: %d, %d\n", i, j);
  j = 32 - __builtin_clz(i);
  printf("bits: %d\n", j);
  j = __builtin_parity(i);
  printf("parity: %d, %d\n", i, j);
  j = __builtin_popcount(i);
  printf("popcount: %d, %d\n", i, j);

  return 0;
}
