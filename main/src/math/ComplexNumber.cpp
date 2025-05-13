#include "math/ComplexNumber.hpp"
#include "common.hpp"

#include <cmath>
#include <cstring>
#include <iostream>
#include <cctype>
#include <stdexcept>

ComplexNumber::ComplexNumber(const char *c) : real(0.0), imaginary(0.0) {
  char *d;
  float v;
  while (*c) {
    switch (*c) {
      case 'i':
      case 'I':
        imaginary += 1;
      case ' ': // ignore whitespace
        break;
      case '+':
        if (*(c+1) == 'i' || *(c+1) == 'I') {
          imaginary += 1;
          ++c;
          break;
        }
      case '-':
        if (*(c+1) == 'i' || *(c+1) == 'I') {
          imaginary -= 1;
          ++c;
          break;
        }
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        v = strtof(c, &d);
        if (*d == 'i' || *d == 'I') {
          imaginary += v;
          ++d;
        } else
          real += v;
        c = d;
        continue;
      default:
        throw c;
    }
    ++c;
  }
}

ComplexNumber::ComplexNumber(): real(0.0f), imaginary(0.0f) {}
ComplexNumber::ComplexNumber(float r): real(r), imaginary(0.0f) {}
ComplexNumber::ComplexNumber(float r, float i): real(r), imaginary(i) {}

// reinitialize
ComplexNumber &ComplexNumber::operator=(const ComplexNumber b) {
	real = b.real;
	imaginary = b.imaginary;
	return *this;
}
ComplexNumber &ComplexNumber::operator=(const float b) {
	real = b;
	imaginary = 0.0f;
	return *this;
}

//compare
static constexpr float epsilon = 1e-4f;
bool operator==(const ComplexNumber &a, const ComplexNumber &b) {
	return fabs(a.real - b.real) < epsilon && fabs(a.imaginary - b.imaginary) < epsilon;
}
bool operator!=(const ComplexNumber &a, const ComplexNumber &b) {
	return fabs(a.real - b.real) >= epsilon || fabs(a.imaginary - b.imaginary) >= epsilon;
}
// unsafe
ComplexNumber operator+(const ComplexNumber a, const ComplexNumber b) {
	return ComplexNumber{a.real + b.real, a.imaginary + b.imaginary};
}
ComplexNumber operator-(const ComplexNumber a, const ComplexNumber b) {
	return ComplexNumber{a.real - b.real, a.imaginary - b.imaginary};
}
ComplexNumber operator*(const ComplexNumber a, const ComplexNumber b) {
	return ComplexNumber{a.real * b.real - a.imaginary * b.imaginary, a.real * b.imaginary + b.real * a.imaginary};
}
ComplexNumber operator/(const ComplexNumber a, const ComplexNumber b) {
	float d = b.real * b.real + b.imaginary * b.imaginary;
	return ComplexNumber{(a.real * b.real + a.imaginary * b.imaginary) / d, (a.imaginary * b.real - a.real * b.imaginary) / d};
}
ComplexNumber operator^(const ComplexNumber a, const ComplexNumber b) {
	float magA = sqrt(a.real * a.real + a.imaginary * a.imaginary);
	float argA = atan(a.imaginary / a.real);
	float mag = pow (magA, b.real) / exp (b.imaginary * argA);
	float arg = b.imaginary * log (magA) + b.real * argA;
	return ComplexNumber{mag * (float)cos (arg), mag * (float)sin (arg)};
}

// safe
ComplexNumber &operator+=(ComplexNumber &a, const ComplexNumber b) {
	a.real += b.real;
	a.imaginary += b.imaginary;
	return a;
}
ComplexNumber &operator-=(ComplexNumber &a, const ComplexNumber b) {
	a.real -= b.real;
	a.imaginary -= b.imaginary;
	return a;
}
ComplexNumber &operator*=(ComplexNumber &a, const ComplexNumber b) {
	a.real *= b.real;
	a.real -= a.imaginary * b.imaginary;
	a.imaginary *= b.real;
	a.imaginary += a.real * b.imaginary;
	return a;
}
ComplexNumber &operator/=(ComplexNumber &a, const ComplexNumber b) {
	float d = b.real * b.real + b.imaginary * b.imaginary;
	a.real *= b.real;
	a.real += a.imaginary * b.imaginary;
	a.real /= d;
	a.imaginary *= b.real;
	a.imaginary -= a.real * b.imaginary;
	a.imaginary /= d;
	return a;
}
ComplexNumber &operator^=(ComplexNumber &a, const ComplexNumber b) {
	float magA = sqrt(a.real * a.real + a.imaginary * a.imaginary);
	float argA = atan(a.imaginary / a.real);
	float mag = pow (magA, b.real) / exp (b.imaginary * argA);
	float arg = b.imaginary * log (magA) + b.real * argA;
	a.real = mag * cos (arg);
	a.imaginary = mag * sin (arg);
	return a;
}
// output console
std::ostream &operator<<(std::ostream &o, const ComplexNumber a) {
	if (a.real || a.imaginary) {
  	if (a.real)
  	  o << a.real;
  	if (a.real && (a.imaginary > 0))
  	  o << "+";
  	if (a.imaginary)
  	  o << a.imaginary << "i";
	} else
	  o << "0";
	return o;
}
