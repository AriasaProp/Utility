#include "BigDecimal.hpp"


//Constructors
BigDecimal::BigDecimal() {}
BigDecimal::BigDecimal(const BigDecimal) {}
BigDecimal::BigDecimal(const double) {}
BigDecimal::BigDecimal(const char *) {}
//Destructor
BigDecimal::~BigDecimal() {}
//operator casting
BigDecimal::operator bool() const {}
BigDecimal::operator double() const {}
char *BigDecimal::to_chars() const {}
// math operational function
BigDecimal BigDecimal::sqrt() const {}
BigDecimal BigDecimal::pow(const double) const {}
BigDecimal BigDecimal::pow(const BigDecimal) const {}
// re-initialize operational function
BigDecimal &BigDecimal::operator=(const double) {}
BigDecimal &BigDecimal::operator=(const BigDecimal) {}
// compare operator function
bool BigDecimal::operator==(const double) const {}
bool BigDecimal::operator!=(const double) const {}
bool BigDecimal::operator<=(const double) const {}
bool BigDecimal::operator>=(const double) const {}
bool BigDecimal::operator<(const double) const {}
bool BigDecimal::operator>(const double) const {}
bool BigDecimal::operator==(const BigDecimal) const {}
bool BigDecimal::operator!=(const BigDecimal) const {}
bool BigDecimal::operator<=(const BigDecimal) const {}
bool BigDecimal::operator>=(const BigDecimal) const {}
bool BigDecimal::operator<(const BigDecimal) const {}
bool BigDecimal::operator>(const BigDecimal) const {}
// safe operator math function
BigDecimal &BigDecimal::operator+=(const double) {}
BigDecimal &BigDecimal::operator+=(const BigDecimal) {}
BigDecimal &BigDecimal::operator-=(const double) {}
BigDecimal &BigDecimal::operator-=(const BigDecimal) {}
BigDecimal &BigDecimal::operator*=(const double) {}
BigDecimal &BigDecimal::operator*=(const BigDecimal) {}
BigDecimal &BigDecimal::operator/=(const double) {}
BigDecimal &BigDecimal::operator/=(const BigDecimal) {}
// unsafe operator function
BigDecimal BigDecimal::operator-() const {}
BigDecimal BigDecimal::operator+(const double) const {}
BigDecimal BigDecimal::operator+(const BigDecimal) const {}
BigDecimal BigDecimal::operator-(const double) const {}
BigDecimal BigDecimal::operator-(const BigDecimal) const {}
BigDecimal BigDecimal::operator*(const double) const {}
BigDecimal BigDecimal::operator*(const BigDecimal) const {}
BigDecimal BigDecimal::operator/(const double) const {}
BigDecimal BigDecimal::operator/(const BigDecimal) const {}
// stream operator
friend std::ostream &operator<<(std::ostream &, const BigDecimal) {}
