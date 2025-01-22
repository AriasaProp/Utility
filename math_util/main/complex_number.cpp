#include "complex_number.hpp"

#include <cmath>
#include <cstring>
#include <iomanip>

static inline float _cos(float in) {
	return static_cast<float>(cos(in));
}
static inline float _sin(float in) {
	return static_cast<float>(sin(in));
}

complex_number::complex_number(): real(0.0f), imaginary(0.0f) {}
complex_number::complex_number(float r): real(r), imaginary(0.0f) {}
complex_number::complex_number(float r, float i): real(r), imaginary(i) {}

// reinitialize
complex_number &complex_number::operator=(const complex_number b) {
	real = b.real;
	imaginary = b.imaginary;
	return *this;
}
complex_number &complex_number::operator=(const float b) {
	real = b;
	imaginary = 0.0f;
	return *this;
}

//compare
bool operator==(const complex_number a, const complex_number b) {
	return !memcmp(&a, &b, sizeof(complex_number));
}
bool operator!=(const complex_number a, const complex_number b) {
	return memcmp(&a, &b, sizeof(complex_number));
}

// unsafe
complex_number operator+(const complex_number a, const complex_number b) {
	return complex_number{a.real + b.real, a.imaginary + b.imaginary};
}
complex_number operator-(const complex_number a, const complex_number b) {
	return complex_number{a.real - b.real, a.imaginary - b.imaginary};
}
complex_number operator*(const complex_number a, const complex_number b) {
	return complex_number{a.real * b.real - a.imaginary * b.imaginary, a.real * b.imaginary + b.real * a.imaginary};
}
complex_number operator/(const complex_number a, const complex_number b) {
	float d = b.real * b.real + b.imaginary * b.imaginary;
	return complex_number{(a.real * b.real + a.imaginary * b.imaginary) / d, (a.imaginary * b.real - a.real * b.imaginary) / d};
}
complex_number operator^(const complex_number a, const complex_number b) {
	float magA = sqrt(a.real * a.real + a.imaginary * a.imaginary);
	float argA = atan(a.imaginary / a.real);
	float mag = pow (magA, b.real) / exp (b.imaginary * argA);
	float arg = b.imaginary * log (magA) + b.real * argA;
	return complex_number{mag * _cos (arg), mag * _sin (arg)};
}

// safe
complex_number &operator+=(complex_number &a, const complex_number b) {
	a.real += b.real;
	a.imaginary += b.imaginary;
	return a;
}
complex_number &operator-=(complex_number &a, const complex_number b) {
	a.real -= b.real;
	a.imaginary -= b.imaginary;
	return a;
}
complex_number &operator*=(complex_number &a, const complex_number b) {
	a.real *= b.real;
	a.real -= a.imaginary * b.imaginary;
	a.imaginary *= b.real;
	a.imaginary += a.real * b.imaginary;
	return a;
}
complex_number &operator/=(complex_number &a, const complex_number b) {
	float d = b.real * b.real + b.imaginary * b.imaginary;
	a.real *= b.real;
	a.real += a.imaginary * b.imaginary;
	a.real /= d;
	a.imaginary *= b.real;
	a.imaginary -= a.real * b.imaginary;
	a.imaginary /= d;
	return a;
}
complex_number &operator^=(complex_number &a, const complex_number b) {
	float magA = sqrt(a.real * a.real + a.imaginary * a.imaginary);
	float argA = atan(a.imaginary / a.real);
	float mag = pow (magA, b.real) / exp (b.imaginary * argA);
	float arg = b.imaginary * log (magA) + b.real * argA;
	a.real = mag * _cos (arg);
	a.imaginary = mag * _sin (arg);
	return a;
}
// output console
std::ostream &operator<<(std::ostream &o, const complex_number a) {
	o << "( " << std::setprecision(2) << a.real << " + " << std::setprecision(2) << a.imaginary << " )";
	return o;
}
