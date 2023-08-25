#include "matrix.hpp"

bool matrix_test() {
  
  matrix<2,2> mx2a{{12.f,17.f},{13.f,11.f}};
  matrix<2,2> mx2b{{7.f,4.f},{18.f,0.f}};
  
  mx2a.print_matrix();
  mx2b.print_matrix();
  (mx2a+mx2b).print_matrix();
  (mx2a*mx2b).print_matrix();
  
  return true;
}