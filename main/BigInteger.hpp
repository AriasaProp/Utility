#ifndef Included_Big_Integer
#define Included_Big_Integer

#include <iostream>
#include <vector>

typedef unsigned int word;

struct BigInteger {
  private:
    // values
    bool neg = false;
    std::vector<word> words;
  public:
    //Constructors
    BigInteger();
    BigInteger(const BigInteger&);
    BigInteger(const signed);
    BigInteger(const char *);
    BigInteger(const std::vector<word>, bool);
    //Destructor
    ~BigInteger();
    //operator casting
    operator bool() const;
    operator int() const;
    //operator double() const;
    char *to_chars() const;
    // math operational function
    BigInteger sqrt() const;
    BigInteger pow(size_t) const;
    // re-initialize operational function
    BigInteger &operator=(const signed);
    BigInteger &operator=(const BigInteger);
    // compare operator function
    bool operator==(const signed) const;
    bool operator!=(const signed) const;
    bool operator<=(const signed) const;
    bool operator>=(const signed) const;
    bool operator<(const signed) const;
    bool operator>(const signed) const;
    bool operator==(const BigInteger) const;
    bool operator!=(const BigInteger) const;
    bool operator<=(const BigInteger) const;
    bool operator>=(const BigInteger) const;
    bool operator<(const BigInteger) const;
    bool operator>(const BigInteger) const;
    // safe operator math function
    BigInteger &operator--();
    BigInteger &operator++();
    BigInteger &operator+=(const signed);
    BigInteger &operator+=(const BigInteger);
    BigInteger &operator-=(const signed);
    BigInteger &operator-=(const BigInteger);
    BigInteger &operator*=(const signed);
    BigInteger &operator*=(const BigInteger);
    BigInteger &operator/=(const signed);
    BigInteger &operator/=(const BigInteger);
    BigInteger &operator%=(const signed);
    BigInteger &operator%=(const BigInteger);
    //safe bitwise operand
    friend BigInteger &operator>>=(BigInteger&,size_t);
    friend BigInteger &operator<<=(BigInteget&,size_t);
    friend BigInteger &operator&=(BigInteger&, const BigInteger);
    friend BigInteger &operator|=(BigInteger&, const BigInteger);
    // new object generate, operator function
    BigInteger operator+(const signed) const;
    BigInteger operator+(const BigInteger) const;
    BigInteger operator-(const signed) const;
    BigInteger operator-(const BigInteger) const;
    BigInteger operator*(const signed) const;
    BigInteger operator*(const BigInteger) const;
    BigInteger operator/(const signed) const;
    BigInteger operator/(const BigInteger) const;
    BigInteger operator%(const signed) const;
    BigInteger operator%(const BigInteger) const;
    BigInteger operator-() const;
    //new object generate, bitwise operand
    friend BigInteger operator>>(const BigInteger, size_t) const;
    friend BigInteger operator<<(const BigInteger, size_t) const;
    friend BigInteger operator&(const BigInteger, const BigInteger) const;
    friend BigInteger operator|(const BigInteger, const BigInteger) const;
    // stream operator
    friend std::ostream &operator<<(std::ostream &, const BigInteger);
};
#endif
