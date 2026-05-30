#include <stdio.h>
#include <math.h>

int main() {
  float x[] = {7.441f, 12.8891f, -0.22f, -3.79f, 0.1456, -9.91}; 
  printf("round\n");
  for (int i = 0; i < sizeof(x)/sizeof(float); ++i) {
    printf("%f -> %f | %f \n",x[i], roundf(x[i]), (float)(int)(x[i] + 0.5f - (x[i] < 0)));
  }
  printf("floor\n");
  for (int i = 0; i < sizeof(x)/sizeof(float); ++i) {
    printf("%f -> %f | %f \n",x[i], floorf(x[i]), ((float)(int)x[i]) - (x[i] < 0));
  }
  printf("ceil\n");
  for (int i = 0; i < sizeof(x)/sizeof(float); ++i) {
    printf("%f -> %f | %f \n",x[i], ceilf(x[i]), ((float)(int)x[i]) + (x[i] > 0));
  }
  return 0;
}
