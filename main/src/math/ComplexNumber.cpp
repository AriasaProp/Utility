#include "math/ComplexNumber.hpp"

#include <cmath>
#include <cstring>
#include <iomanip>
/*
ComplexNumber::ComplexNumber(const char *c) {
  // while(*(c++)) {
    
  // }
}
*/
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
bool operator==(const ComplexNumber a, const ComplexNumber b) {
	return fabs(a.real - b.real) < epsilon && fabs(a.imaginary - b.imaginary) < epsilon;
}
bool operator!=(const ComplexNumber a, const ComplexNumber b) {
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
	o << std::fixed << std::setprecision(2) << a.real;
	if (a.imaginary)
	  o << std::showpos << std::fixed << std::setprecision(2) << a.imaginary << "i";
	return o;
}
