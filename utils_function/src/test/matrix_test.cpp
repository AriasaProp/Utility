#include "matrix.hpp"

bool matrix_test() {
  std::array<std::array<float, 2>, 2> data2x2 = {{1.0, 2.0}, {3.0, 4.0}};
  matrix<2, 2> matrix2x2(data2x2);
  matrix2x2.print();

  std::array<std::array<float, 3>, 3> data3x3 = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};
  matrix<3, 3> matrix3x3(data3x3);
  matrix3x3.print();
  return true;
}