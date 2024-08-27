#include "BigDecimal.hpp"

// Constructors
BigDecimal::BigDecimal () {}
BigDecimal::BigDecimal (const BigDecimal &o) : neg (o.neg), major (o.major), numerator (o.numerator), denumerator (o.denumerator) {}
BigDecimal::BigDecimal (const double) {}
BigDecimal::BigDecimal (const char *) {}
// Destructor
BigDecimal::~BigDecimal () {
  major.clear ();
  numerator.clear ();
  denumerator.clear ();
}
// operator casting
BigDecimal::operator bool () const { return false; }
BigDecimal::operator double () const { return 0.0; }
char *BigDecimal::to_chars () const { return (char *)nullptr; }
// math operational function
BigDecimal BigDecimal::sqrt () const { return BigDecimal (*this); }
BigDecimal BigDecimal::pow (const double) const { return BigDecimal (*this); }
BigDecimal BigDecimal::pow (const BigDecimal) const { return BigDecimal (*this); }
// re-initialize operational function
BigDecimal &BigDecimal::operator= (const double) { return *this; }
BigDecimal &BigDecimal::operator= (const BigDecimal o) {
  this->neg = o.neg;
  this->major = o.major;
  this->numerator = o.numerator;
  this->denumerator = o.denumerator;
  return *this;
}
// compare operator function
bool BigDecimal::operator== (const double) const { return false; }
bool BigDecimal::operator!= (const double) const { return false; }
bool BigDecimal::operator<= (const double) const { return false; }
bool BigDecimal::operator>= (const double) const { return false; }
bool BigDecimal::operator<(const double) const { return false; }
bool BigDecimal::operator> (const double) const { return false; }
bool BigDecimal::operator== (const BigDecimal) const { return false; }
bool BigDecimal::operator!= (const BigDecimal) const { return false; }
bool BigDecimal::operator<= (const BigDecimal) const { return false; }
bool BigDecimal::operator>= (const BigDecimal) const { return false; }
bool BigDecimal::operator<(const BigDecimal) const { return false; }
bool BigDecimal::operator> (const BigDecimal) const { return false; }
// safe operator math function
BigDecimal &BigDecimal::operator+= (const double) { return *this; }
BigDecimal &BigDecimal::operator+= (const BigDecimal) { return *this; }
BigDecimal &BigDecimal::operator-= (const double) { return *this; }
BigDecimal &BigDecimal::operator-= (const BigDecimal) { return *this; }
BigDecimal &BigDecimal::operator*= (const double) { return *this; }
BigDecimal &BigDecimal::operator*= (const BigDecimal) { return *this; }
BigDecimal &BigDecimal::operator/= (const double) { return *this; }
BigDecimal &BigDecimal::operator/= (const BigDecimal) { return *this; }
// unsafe operator function
BigDecimal BigDecimal::operator- () const { return BigDecimal (*this); }
BigDecimal BigDecimal::operator+ (const double) const { return BigDecimal (*this); }
BigDecimal BigDecimal::operator+ (const BigDecimal) const { return BigDecimal (*this); }
BigDecimal BigDecimal::operator- (const double) const { return BigDecimal (*this); }
BigDecimal BigDecimal::operator- (const BigDecimal) const { return BigDecimal (*this); }
BigDecimal BigDecimal::operator* (const double) const { return BigDecimal (*this); }
BigDecimal BigDecimal::operator* (const BigDecimal) const { return BigDecimal (*this); }
BigDecimal BigDecimal::operator/ (const double) const { return BigDecimal (*this); }
BigDecimal BigDecimal::operator/ (const BigDecimal) const { return BigDecimal (*this); }
// stream operator
std::ostream &operator<< (std::ostream &out, const BigDecimal) {
  out << "no output";
  return out;
}
