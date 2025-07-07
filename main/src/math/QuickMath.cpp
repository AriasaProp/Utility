#include "math/QuickMath.hpp"

#include <cstdint>

static const float EPSILON = 1e-50f;
static const float PI = 3.14159265f;
static const float PIHalf = 1.57079633f;

float QuickMath::inverse_sqrt(float x) {
  static const float threehalf = 1.5f;
  int i = *(int *) &x;
  i = 0x5f3759df - (i >> 1);
  float y = *(float *)&i;
  return y * (threehalf - 0.5f * x * y * y);
}
// trigonometry
/*
// input is degree x°
float QuickMath::deg_sin(float x) {
  
  return 0;
}
float QuickMath::deg_cos(float x) {
  
  return 0;
}
float QuickMath::deg_tan(float x) {
  
  return 0;
}
// input is radian π

   39916800
  100000000

*/
// taylor series
/*
static const float cosine_table[][2] = {
  {0.0, 0.0},
  {0.05233595624f},
  {0.25881904510f},
  0.30901699437f,
  {PI*0.1666666667f, 0.5f},
  0.58778525229f,
  {PI*0.25f, 0.70710678119f},
  0.80901699437f,
  {PI*0.333333333f, 0.86602540378f},
  0.95105651630f,
  0.96592582629f,
  0.99862953475f,
  {PIHalf, 1.0}
};

float QuickMath::rad_sin(float x) {
  return y;
}
float QuickMath::rad_cos(float x) {
  return y;
}
float QuickMath::rad_tan(float x) {
  float xp = x / PI; xp *= xp;
  float s = (1f - xp) * (4f - xp) * (9f - xp);
  float c = (0.25f - xp) * (2.25f - xp) * (6.25f - xp);
  return 0.09765625f * s / c;
}
// input is degree x°
float QuickMath::deg_sinh(float x) {
  
  return 0;
}
float QuickMath::deg_cosh(float x) {
  
  return 0;
}
float QuickMath::deg_tanh(float x) {
  
  return 0;
}
// input is radian π
float QuickMath::rad_sinh(float x) {
  
  return 0;
}
float QuickMath::rad_cosh(float x) {
  
  return 0;
}
float QuickMath::rad_tanh(float x) {
  
  return 0;
}
*/