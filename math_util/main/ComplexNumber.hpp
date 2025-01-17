#ifndef _COMPLEX_NUMBER_
#define _COMPLEX_NUMBER_

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

constexpr auto CARTESIAN = 0;
constexpr auto POLAR = 1;
constexpr auto DEGREES = 0;
constexpr auto RADIANS = 1;
constexpr auto PI = 3.14159265f;
constexpr auto PHI = 1.6180339f;
#define Degrees(x) (360.0f * x) / (2.0f * PI)

struct ComplexNumber {
private:
  /* Cartesian form */
  float Real, Imaginary;
  /* Polar form */
  float Magnitude, Argument;

public:
  ComplexNumber (float Input1 = 0.0, float Input2 = 0.0, int form = CARTESIAN);

  float GetReal () const;
  float GetImaginary () const;
  float GetMagnitude () const;
  float GetArgument () const;

  void SetReal (float input);
  void SetImaginary (float input);
  void SetMagnitude (float input);
  void SetArgument (float input);

  void SetCartesianForm ();
  void SetPolarForm ();

  void CalculateReal ();
  void CalculateImaginary ();
  void CalculateMagnitude ();
  void CalculateArgument ();

  void PrintPolarForm (std::ostream &os, int angleType = RADIANS) const;
};

std::ostream &operator<< (std::ostream &os, const ComplexNumber &rhs);
std::istream &operator>> (std::istream &os, ComplexNumber &rhs);

ComplexNumber operator+ (const ComplexNumber &lhs, const ComplexNumber &rhs);
ComplexNumber operator- (const ComplexNumber &lhs, const ComplexNumber &rhs);
ComplexNumber operator* (const ComplexNumber &lhs, const ComplexNumber &rhs);
ComplexNumber operator/ (const ComplexNumber &lhs, const ComplexNumber &rhs);

ComplexNumber logC (const ComplexNumber &X);
ComplexNumber powC (const ComplexNumber &X, const ComplexNumber &Y);
ComplexNumber rootC (const ComplexNumber &X, const ComplexNumber &Y);
ComplexNumber BinetFibFormula (const ComplexNumber &n);

template <typename Container>
void fib (int a, int b, int n, Container &fibSeq) {
  if (n > 0) {
    fibSeq.push_back (a);
    fib (b, a + b, --n, fibSeq);
  }
}

#endif //_COMPLEX_NUMBER_
