#include <stdio.h>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  long i = 0b10000000101000001000011010010110;
  long j = __builtin_ffs(i); // find 1 from less index + 1
  printf("ffs: %ld, %ld\n", i, j);
  j = __builtin_clzl(i);
  printf("clz: %ld, %ld\n", i, j);
  j = sizeof(long long)*8 - __builtin_clzl(i);
  printf("bits: %ld\n", j);
  j = __builtin_parityl(i);
  printf("parity: %ld, %ld\n", i, j);
  j = __builtin_popcountl(i);
  printf("popcount: %ld, %ld\n", i, j);

  return 0;
}
