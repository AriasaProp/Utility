#ifndef Included_Big_Integer
#define Included_Big_Integer

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstdio>

typedef unsigned int word;
typedef signed int s_word;

static const size_t WORD_BITS = sizeof(word) * CHAR_BIT;
static const size_t WORD_BITS_1 = WORD_BITS - 1;
static const word WORD_MASK = (word)-1;
static const size_t WORD_HALF_BITS = sizeof(word) * CHAR_BIT / 2;
static const word WORD_HALF_MASK = WORD_MASK >> WORD_HALF_BITS;

class BigInteger
{
  private:
    // values
    bool neg = false;
    std::vector<word> words;
    //helper
    word get_bit(size_t) const;
    void clr_bit(size_t);

    // math operational function
    BigInteger pow(size_t) const;
    BigInteger mod_pow(BigInteger, const BigInteger &) const;
    BigInteger sqrt() const;

  public:
    //Constructors
    BigInteger();
    BigInteger(size_t, word, bool);
    BigInteger(const word *, const word *, bool);
    BigInteger(const BigInteger &);
    BigInteger(const signed &);
    BigInteger(const char *);
    BigInteger(const std::vector<word> &, bool);
    //Destructor
    ~BigInteger();

    BigInteger &set_bit(size_t);
    // TODO : need to be more efficiently
    size_t tot() const;
    char *to_chars() const;
    double to_double() const;
    bool can_convert_to_int(int *) const;
    // re-initialize operational function
    BigInteger &operator=(const signed &);
    BigInteger &operator=(const BigInteger &);
    // safe operational function
    BigInteger &operator--();
    BigInteger &operator++();
    BigInteger &operator+=(const BigInteger &);
    BigInteger &operator-=(const BigInteger &);
    BigInteger &operator*=(const BigInteger &);
    BigInteger &operator/=(const BigInteger &);
    BigInteger &operator%=(const BigInteger &);
    // compare operator function
    bool operator==(const BigInteger &) const;
    bool operator!=(const BigInteger &) const;
    bool operator<=(const BigInteger &) const;
    bool operator>=(const BigInteger &) const;
    bool operator<(const BigInteger &) const;
    bool operator>(const BigInteger &) const;
    // unsafe operational function
    BigInteger operator+(const BigInteger &) const;
    BigInteger operator-(const BigInteger &) const;
    BigInteger operator*(const BigInteger &)const;
    BigInteger operator/(const BigInteger &) const;
    BigInteger operator%(const BigInteger &) const;
    BigInteger operator-() const;
    // shift operator function
    BigInteger &operator>>=(size_t);
    BigInteger &operator<<=(size_t);
    BigInteger operator>>(size_t) const;
    BigInteger operator<<(size_t) const;

    friend std::ostream &operator<<(std::ostream &, const BigInteger &);
};
#endif
