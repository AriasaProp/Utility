#include "matrix.hpp"

#include <iostream>

bool matrix_test() {
  
  matrix2D ma(2,2, {1.0f, 2.0f, 3.0f, 4.0f});
  ma.print();
  
  matrix2D mb(2,3, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});
  mb.print();

  matrix2D mc(3,2, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});
  mc.print();

  matrix2D md(3,3, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f,7.0f, 8.0f, 9.0f});
  md.print();
  
  matrix2D mA = ma;
  
  std::cout << "a + a" << std::endl;
  (ma + ma).print();
  std::cout << "A += m2" << std::endl;
  (mA += ma).print();
  
  std::cout << "2x2 - 2x2" << std::endl;
  (mA - ma).print();
  std::cout << "A -= 2x2" << std::endl;
  (mA -= ma).print();
  
  std::cout << "2x2 * 2x2 = " << std::endl;
  (ma * ma).print();
  std::cout << "A *= 2x2 = " << std::endl;
  (mA *= ma).print();
  
  return true;
}