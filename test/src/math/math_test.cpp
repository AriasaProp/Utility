#include <cstring>
#include <fstream>
#include <iostream>
#include <ostream>
#include <cstdlib>
#include <cstdint>

extern std::ostream *output_file;
extern char *data_address;

extern void QuickMath_test ();
extern void BigInteger_test ();
extern void ComplexNumber_test ();
extern void Matrix_test ();

void math_test () {
  strcpy(data_address, "data/math");
  QuickMath_test ();
  /*
  BigInteger_test ();
  ComplexNumber_test ();
  Matrix_test ();
  */
}