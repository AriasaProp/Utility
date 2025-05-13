#ifndef COMPLEX_NUMBER_
#define COMPLEX_NUMBER_

#include <ostream>
#include <initializer_list>

struct ComplexNumber {
	float real, imaginary; // safed in cartesian form
	ComplexNumber();
	ComplexNumber(const char *);
	ComplexNumber(float);
	ComplexNumber(float,float);
	// reinitialize
	ComplexNumber &operator=(const ComplexNumber);
	ComplexNumber &operator=(const float);
};


//compare
bool operator==(const ComplexNumber&, const ComplexNumber&);
bool operator!=(const ComplexNumber&, const ComplexNumber&);

//unsafe
ComplexNumber operator+(const ComplexNumber, const ComplexNumber);
ComplexNumber operator-(const ComplexNumber, const ComplexNumber);
ComplexNumber operator*(const ComplexNumber, const ComplexNumber);
ComplexNumber operator/(const ComplexNumber, const ComplexNumber);
ComplexNumber operator^(const ComplexNumber, const ComplexNumber);

//safe
ComplexNumber &operator+=(ComplexNumber&, const ComplexNumber);
ComplexNumber &operator-=(ComplexNumber&, const ComplexNumber);
ComplexNumber &operator*=(ComplexNumber&, const ComplexNumber);
ComplexNumber &operator/=(ComplexNumber&, const ComplexNumber);
ComplexNumber &operator^=(ComplexNumber&, const ComplexNumber);

std::ostream &operator<<(std::ostream &, const ComplexNumber);

#endif // COMPLEX_NUMBER_