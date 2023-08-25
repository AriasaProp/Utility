#include "matrix.hpp"

bool matrix_test() {
  const float data2x2[4] = {1.0f, 2.0f, 3.0f, 4.0f};
  matrix<2, 2> matrix2x2(2,2, data2x2);
  matrix2x2.print();

  const float data3x3[9] = {1.0f, 2.0f, 3.0f,4.0f, 5.0f, 6.0f,7.0f, 8.0f, 9.0f};
  matrix<3, 3> matrix3x3(3,3,data3x3);
  matrix3x3.print();
  return true;
}