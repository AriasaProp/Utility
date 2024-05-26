#include "ComplexNumber.hpp"
#include "clock_adjustment.hpp"
#include <iostream>

// test
bool ComplexNumber_test () {

  /* Defining complex numbers in cartesian form */
  // ComplexNumber number(Real, Imaginary)
  ComplexNumber C1 (2, 3);
  ComplexNumber C2 (3, 4);
  ComplexNumber C3 = C1 + C2;
  C3.PrintCartesianForm ();

  /* A clear way of displaying complex numbers in cartesian form */
  ComplexNumber i (0, 1);
  ComplexNumber number = -3 + 6 * i; // inefficient because of type converting
  std::cout << "ComplexNumber number = " << number << std::endl;

  /* Defining complex numbers in polar form */
  // ComplexNumber number(Magnitude, Argument in radians)
  ComplexNumber C4 (1, PI / 3, POLAR);
  ComplexNumber C5 (2, PI / 4, POLAR);
  ComplexNumber C6 = C4 + C5;
  C5.PrintPolarForm (DEGREES);

  /* Taking natural log of complex numbers */
  ComplexNumber C7 (5, -3);
  logC (C7).PrintCartesianForm ();

  /* Taking complex number X to power of complex number Y */
  // powC(X, Y)
  ComplexNumber C8 (-4, 6);
  ComplexNumber C9 (1, -2);
  powC (C8, C9).PrintPolarForm (DEGREES);

  /* Taking complex root Y of a complex number X */
  // rootC(X, Y)
  ComplexNumber C10 (54, 33.231);
  ComplexNumber C11 (0.231, 1644);
  rootC (C10, C11).PrintCartesianForm ();

  /* Getting the nth term in the fib sequence */
  // When n is a non-integer, the result is complex
  ComplexNumber Cfib = BinetFibFormula (2.3);
  Cfib.PrintCartesianForm ();
  // n can also be complex
  ComplexNumber C12 (3, 5);
  Cfib = BinetFibFormula (C12);
  Cfib.PrintCartesianForm ();

  return true;
}
