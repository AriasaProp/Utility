#include "matrix.hpp"

bool matrix_test() {
  
  matrix<2,2> mx2a{{12,17},{13,11}};
  matrix<2,2> mx2b{{7,4},{18,0}};
  
  mx2a.print_matrix();
  mx2b.print_matrix();
  (mx2a+mx2b).print_matrix();
  (mx2a*mx2b).print_matrix();
  
  return true;
}