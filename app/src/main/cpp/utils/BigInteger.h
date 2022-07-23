#include <cmath>
#include <iostream>
#include <vector>
#include <chrono>
#include <memory>
#include <thread>
#include <cstdlib>
#include <cstdio>
#include <stdint.h>
#include <limits.h>
#include <algorithm>

typedef uintmax_t word;

static const word WORD_MASK = (word)-1;
static const size_t WORD_BITS = sizeof(word) * CHAR_BIT;
static const word WORD_HALF_MASK = WORD_MASK >> WORD_BITS / 2;
	
class BigInteger
{
  public:
    std::vector<word> words;
    bool neg;
	//Constructors
    BigInteger();
    BigInteger(size_t n, word w, bool neg = false);
    BigInteger(const word *a, const word *b, bool neg = false);
    BigInteger(const BigInteger &a);
    BigInteger(int i);
    BigInteger(const char *C);
	//Destructor
    ~BigInteger();
	
	//helper
    word &operator[](size_t i);
    const word &operator[](size_t i);
    BigInteger &set_neg(bool neg);
    BigInteger &truncate();
    size_t bitlength();
    size_t count_trailing_zeros();
    static int cmp_abs(const BigInteger &a, const BigInteger &b);
    static int cmp(const BigInteger &a, const BigInteger &b);
    static size_t word_bitlength(word a);
    static size_t word_count_trailing_zeros(word a);
    static word add_carry(word *a, word b);
    static word sub_carry(word *a, word b);
    static word word_mul_hi(word a, word b);
    static BigInteger &sub_unsigned_overwrite(BigInteger &a, const BigInteger &b);
    static BigInteger mul_long(const BigInteger &a, const BigInteger &b);
    static BigInteger mul_karatsuba(const BigInteger &a, const BigInteger &b);
    static BigInteger mul(const BigInteger &a, const BigInteger &b);
    static void div_mod(const BigInteger &numerator, BigInteger denominator, BigInteger &quotient, BigInteger &remainder);
    static void split(const BigInteger &a, BigInteger *parts, size_t n_parts, size_t n);
    static BigInteger div(const BigInteger &numerator, const BigInteger &denominator);
    static BigInteger mod(const BigInteger &numerator, const BigInteger &denominator);
    static BigInteger sub(const BigInteger &a, const BigInteger &b);
    static BigInteger gcd(const BigInteger &a0, const BigInteger &b0);
    typedef void (*random_func)(uint8_t *bytes, size_t n_bytes);
    static BigInteger random_bits(size_t n_bits, random_func func);
    static BigInteger random_inclusive(const BigInteger &inclusive, random_func func);
    static BigInteger random_exclusive(const BigInteger &exclusive, random_func func);
    static BigInteger random_second_exclusive(const BigInteger &inclusive_min_val, const BigInteger &exclusive_max_val, random_func func);
    static BigInteger random_both_inclusive(const BigInteger &inclusive_min_val, const BigInteger &inclusive_max_val, random_func func);
    BigInteger &set_bit(size_t i);
    word get_bit(size_t i) const;
    void clr_bit(size_t i);
    // TODO : need to be more efficiently
    char *to_chars() const;
    double to_double() const;
    bool can_convert_to_int(intmax_t *result);
	// math operational function
    BigInteger pow(size_t exponent) const;
    BigInteger mod_pow(BigInteger exponent, const BigInteger &modulus) const;
    BigInteger sqrt() const;
	// re-initialize operational function
    BigInteger &operator=(const BigInteger &a);
	// safe operational function
    BigInteger &operator++();
    BigInteger &operator+=(const BigInteger &b);
    BigInteger &operator-=(const BigInteger &b);
    BigInteger &operator*=(const BigInteger &b);
    BigInteger &operator/=(const BigInteger &b);
    BigInteger &operator%=(const BigInteger &b);
	// compare operator function 
    bool operator==(const BigInteger &b) const;
    bool operator!=(const BigInteger &b) const;
    bool operator<=(const BigInteger &b) const;
    bool operator>=(const BigInteger &b) const;
    bool operator<(const BigInteger &b) const;
    bool operator>(const BigInteger &b) const;
	// unsafe operational function
    BigInteger operator+(const BigInteger &b) const;
    BigInteger operator-(const BigInteger &b) const;
    BigInteger operator*(const BigInteger &b) const;
    BigInteger operator/(const BigInteger &b) const;
    BigInteger operator%(const BigInteger &b) const;
    BigInteger operator-() const;
	// shift operator function
    BigInteger &operator>>=(size_t n_bits);
    BigInteger &operator<<=(size_t n_bits);
    BigInteger operator>>(size_t n_bits) const;
    BigInteger operator<<(size_t n_bits) const;

    friend ostream &operator<<(ostream &out, const BigInteger &num);
};
