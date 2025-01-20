#include "ComplexNumber.hpp"

#include <ostream>

extern std::ostream *output_file;

// test
bool ComplexNumber_test () {

  /* Defining complex numbers in cartesian form */
  // ComplexNumber number(Real, Imaginary)
  ComplexNumber C1 (2, 3);
  ComplexNumber C2 (3, 4);
  ComplexNumber C3 = C1 + C2;
  *output_file << C3 << std::endl;

  /* A clear way of displaying complex numbers in cartesian form */
  ComplexNumber i (0, 1);
  ComplexNumber number = -3 + 6 * i; // inefficient because of type converting
  *output_file << "ComplexNumber number = " << number << std::endl;

  /* Defining complex numbers in polar form */
  // ComplexNumber number(Magnitude, Argument in radians)
  ComplexNumber C4 (1, PI / 3, POLAR);
  ComplexNumber C5 (2, PI / 4, POLAR);
  ComplexNumber C6 = C4 + C5;
  C5.PrintPolarForm (output_file, DEGREES);

  /* Taking natural log of complex numbers */
  ComplexNumber C7 (5, -3);
  *output_file << logC (C7) << std::endl;

  /* Taking complex number X to power of complex number Y */
  // powC(X, Y)
  ComplexNumber C8 (-4, 6);
  ComplexNumber C9 (1, -2);
  powC (C8, C9).PrintPolarForm (output_file, DEGREES);

  /* Taking complex root Y of a complex number X */
  // rootC(X, Y)
  ComplexNumber C10 (54, 33.231);
  ComplexNumber C11 (0.231, 1644);
  *output_file << rootC (C10, C11) << std::endl;

  /* Getting the nth term in the fib sequence */
  // When n is a non-integer, the result is complex
  ComplexNumber Cfib = BinetFibFormula (2.3);
  *output_file << Cfib << std::endl;
  // n can also be complex
  ComplexNumber C12 (3, 5);
  Cfib = BinetFibFormula (C12);
  *output_file << Cfib << std::endl;

  return true;
}
