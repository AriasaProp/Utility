#ifndef Included_Big_Integer
#define Included_Big_Integer

#include <iostream>
#include <vector>

typedef unsigned int word;
typedef signed int s_word;

class BigInteger {
  private:
    // values
    bool neg = false;
    std::vector<word> words;
  public:
    //Constructors
    BigInteger();
    BigInteger(const BigInteger &);
    BigInteger(const signed &);
    BigInteger(const char *);
    BigInteger(const std::vector<word> &, bool);
    //Destructor
    ~BigInteger();
    //environment count
    size_t tot() const;
    char *to_chars() const;
    double to_double() const;
    bool can_convert_to_int(int *) const;
    // math operational function
    BigInteger sqrt() const;
    // re-initialize operational function
    BigInteger &operator=(const signed &);
    BigInteger &operator=(const BigInteger &);
    // safe operator math function
    BigInteger &operator--();
    BigInteger &operator++();
    BigInteger &operator+=(const s_word &);
    BigInteger &operator+=(const BigInteger &);
    BigInteger &operator-=(const s_word &);
    BigInteger &operator-=(const BigInteger &);
    BigInteger &operator*=(const s_word &);
    BigInteger &operator*=(const BigInteger &);
    BigInteger &operator/=(const s_word &);
    BigInteger &operator/=(const BigInteger &);
    BigInteger &operator%=(const s_word &);
    BigInteger &operator%=(const BigInteger &);
    BigInteger &operator^=(size_t);
    BigInteger &operator>>=(size_t);
    BigInteger &operator<<=(size_t);
    // compare operator function
    bool operator==(const BigInteger &) const;
    bool operator!=(const BigInteger &) const;
    bool operator<=(const BigInteger &) const;
    bool operator>=(const BigInteger &) const;
    bool operator<(const BigInteger &) const;
    bool operator>(const BigInteger &) const;
    // unsafe operator function
    BigInteger operator+(const s_word &) const;
    BigInteger operator+(const BigInteger &) const;
    BigInteger operator-(const s_word &) const;
    BigInteger operator-(const BigInteger &) const;
    BigInteger operator*(const s_word &) const;
    BigInteger operator*(const BigInteger &) const;
    BigInteger operator/(const s_word &) const;
    BigInteger operator/(const BigInteger &) const;
    BigInteger operator%(const s_word &) const;
    BigInteger operator%(const BigInteger &) const;
    BigInteger operator^(size_t) const;
    BigInteger operator-() const;
    BigInteger operator>>(size_t) const;
    BigInteger operator<<(size_t) const;
    // stream operator
    friend std::ostream &operator<<(std::ostream &, const BigInteger &);
};
#endif
