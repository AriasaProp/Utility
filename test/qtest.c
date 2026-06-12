#include <stdio.h>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  for (int x = 0; x < 10; ++x) {
    if (x & 1) continue;
    printf("out %d\n", x);
  }
  return 0;
}
