#ifndef COMPLEX_NUMBER_
#define COMPLEX_NUMBER_

#include <ostream>
#include <initializer_list>

struct complex_number {
	float real, imaginary; // safed in cartesian form
	complex_number();
	complex_number(float);
	complex_number(float,float);
};

// reinitialize
complex_number &operator=(complex_number&, const complex_number);
complex_number &operator=(complex_number&, const float);

//compare
bool operator==(const complex_number, const complex_number);
bool operator!=(const complex_number, const complex_number);

//unsafe
complex_number operator+(const complex_number, const complex_number);
complex_number operator-(const complex_number, const complex_number);
complex_number operator*(const complex_number, const complex_number);
complex_number operator/(const complex_number, const complex_number);
complex_number operator^(const complex_number, const complex_number);

//safe
complex_number &operator+=(complex_number&, const complex_number);
complex_number &operator-=(complex_number&, const complex_number);
complex_number &operator*=(complex_number&, const complex_number);
complex_number &operator/=(complex_number&, const complex_number);
complex_number &operator^=(complex_number&, const complex_number);

std::ostream &operator<<(std::ostream &, const complex_number);

#endif // COMPLEX_NUMBER_