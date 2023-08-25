#include "matrix.hpp"

bool matrix_test() {
  const float data2x2[2][2] = {{1.0, 2.0}, {3.0, 4.0}};
  matrix<2, 2> matrix2x2(data2x2);
  matrix2x2.print();

  const float data3x3[2][3] = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};
  matrix<3, 3> matrix3x3(data3x3);
  matrix3x3.print();
  return true;
}