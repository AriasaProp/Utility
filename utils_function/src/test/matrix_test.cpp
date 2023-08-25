#include "matrix.hpp"

bool matrix_test() {
  
  
  float a[2][2] {{12.f,17.f},{13.f,11.f}};
  float b[2][2] {{7.f,4.f},{18.f,0.f}};
  matrix<2,2> mx2a(a);
  matrix<2,2> mx2b(b);
  
  mx2a.print_matrix();
  mx2b.print_matrix();
  (mx2a+mx2b).print_matrix();
  (mx2a*mx2b).print_matrix();
  
  return true;
}