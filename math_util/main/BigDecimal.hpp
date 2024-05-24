#ifndef Included_Big_Decimal
#define Included_Big_Decimal

#include <iostream>
#include <vector>

typedef unsigned int word;

struct BigDecimal {
  private:
    // values
    bool neg = false;
    std::vector<word> major, numerator, denumerator;
  public:
    //Constructors
    BigDecimal();
    BigDecimal(const BigDecimal&);
    BigDecimal(const double);
    BigDecimal(const char *);
    //Destructor
    ~BigDecimal();
    //operator casting
    operator bool() const;
    operator double() const;
    char *to_chars() const;
    // math operational function
    BigDecimal sqrt() const;
    BigDecimal pow(const double) const;
    BigDecimal pow(const BigDecimal) const;
    // re-initialize operational function
    BigDecimal &operator=(const double);
    BigDecimal &operator=(const BigDecimal);
    // compare operator function
    bool operator==(const double) const;
    bool operator!=(const double) const;
    bool operator<=(const double) const;
    bool operator>=(const double) const;
    bool operator<(const double) const;
    bool operator>(const double) const;
    bool operator==(const BigDecimal) const;
    bool operator!=(const BigDecimal) const;
    bool operator<=(const BigDecimal) const;
    bool operator>=(const BigDecimal) const;
    bool operator<(const BigDecimal) const;
    bool operator>(const BigDecimal) const;
    // safe operator math function
    BigDecimal &operator+=(const double);
    BigDecimal &operator+=(const BigDecimal);
    BigDecimal &operator-=(const double);
    BigDecimal &operator-=(const BigDecimal);
    BigDecimal &operator*=(const double);
    BigDecimal &operator*=(const BigDecimal);
    BigDecimal &operator/=(const double);
    BigDecimal &operator/=(const BigDecimal);
    // unsafe operator function
    BigDecimal operator-() const;
    BigDecimal operator+(const double) const;
    BigDecimal operator+(const BigDecimal) const;
    BigDecimal operator-(const double) const;
    BigDecimal operator-(const BigDecimal) const;
    BigDecimal operator*(const double) const;
    BigDecimal operator*(const BigDecimal) const;
    BigDecimal operator/(const double) const;
    BigDecimal operator/(const BigDecimal) const;
    // stream operator
    friend std::ostream &operator<<(std::ostream &, const BigDecimal);
};
#endif
