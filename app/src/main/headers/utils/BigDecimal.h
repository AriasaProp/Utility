#ifndef Included_Big_Decimal
#define Included_Big_Decimal

#include "BigInteger.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <climits>

class BigDecimal
{
  private:
    // values
    BigInteger nominator, denominator;
    
  public:
    //Constructors
    BigDecimal();
    BigDecimal(const BigDecimal &);
    BigDecimal(const signed &);
    BigDecimal(const char *);
    //Destructor
    ~BigDecimal();
    //environment count
    size_t tot() const;
    char *to_chars() const;
    double to_double() const;
    int to_int() const;
    // math operational function
    BigDecimal sqrt() const;
    // re-initialize operational function
    BigDecimal &operator=(const signed &);
    BigDecimal &operator=(const BigDecimal &);
    // safe operator math function
    BigDecimal &operator--();
    BigDecimal &operator++();
    BigDecimal &operator+=(const signed &);
    BigDecimal &operator+=(const BigDecimal &);
    BigDecimal &operator-=(const signed &);
    BigDecimal &operator-=(const BigDecimal &);
    BigDecimal &operator*=(const signed &);
    BigDecimal &operator*=(const BigDecimal &);
    BigDecimal &operator/=(const signed &);
    BigDecimal &operator/=(const BigDecimal &);
    BigDecimal &operator^=(size_t);
    // compare operator function
    bool operator==(const BigDecimal &) const;
    bool operator!=(const BigDecimal &) const;
    bool operator<=(const BigDecimal &) const;
    bool operator>=(const BigDecimal &) const;
    bool operator<(const BigDecimal &) const;
    bool operator>(const BigDecimal &) const;
    // unsafe operator function
    BigDecimal operator+(const signed &) const;
    BigDecimal operator+(const BigDecimal &) const;
    BigDecimal operator-(const signed &) const;
    BigDecimal operator-(const BigDecimal &) const;
    BigDecimal operator*(const signed &) const;
    BigDecimal operator*(const BigDecimal &) const;
    BigDecimal operator/(const signed &) const;
    BigDecimal operator/(const BigDecimal &) const;
    BigDecimal operator^(size_t) const;
    BigDecimal operator-() const;
    // stream operator
    friend std::ostream &operator<<(std::ostream &, const BigDecimal &);
};
#endif
