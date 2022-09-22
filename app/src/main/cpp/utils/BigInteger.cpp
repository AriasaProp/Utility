#include <cmath>
#include <algorithm>
#include "utils/BigInteger.h"

const size_t WORD_BITS = sizeof(word) * CHAR_BIT;
const size_t WORD_BITS_1 = WORD_BITS - 1;
const word WORD_MASK = (word)-1;
const size_t WORD_HALF_BITS = sizeof(word) * CHAR_BIT / 2;
const word WORD_HALF_MASK = WORD_MASK >> WORD_HALF_BITS;
const double LOG2BITS = std::log(2) * WORD_BITS;

//private function for repeated use

// +1 mean a is greater, -1 mean a is less, 0 mean equal
int compare(const std::vector<word> &a, const std::vector<word> &b)
{
    size_t i = a.size();
    const size_t nb = b.size();
    if (i != nb)
        return i > nb ? +1 : -1;
    word A, B;
    while (i--)
    {
        A = a[i], B = b[i];
        if (A != B)
            return A > B ? +1 : -1;
    }
    return 0;
}

void add_a_word(std::vector<word> &a, size_t i = 0, word carry = 1)
{
    for (size_t j = a.size(); i < j && carry; i++)
        carry = (a[i] += carry) < carry;
    if (carry)
        a.push_back(carry);
}
// a should be greater or equal than carry
void sub_a_word(std::vector<word> &a, size_t i = 0, word carry = 1)
{
    for (size_t j = a.size(); i < j && carry; i++)
        carry = a[i] < (a[i] -= carry);
    while (a.size() && !a.back())
        a.pop_back();
}

// a should clone from the base, b not
void add_word(std::vector<word> &a, const std::vector<word> &b)
{
    if (a.size() < b.size())
        a.resize(b.size(), 0);
    size_t i = 0;
    word carry = 0;
    for (size_t j = b.size(); i < j; i++)
    {
        carry = (a[i] += carry) < carry;
        carry += (a[i] += b[i]) < b[i];
    }
    add_a_word(a, i, carry);
}
// a should be greater or equal than b
// a should clone from the base, b not
void sub_word(std::vector<word> &a, const std::vector<word> &b)
{
    word carry = 0;
    size_t i = 0;
    for (size_t j = b.size(); i < j; i++)
    {
        carry = a[i] < (a[i] -= carry);
        carry += a[i] < (a[i] -= b[i]);
    }
    sub_a_word(a, i, carry);
}

std::vector<word> karatsuba(const std::vector<word> &A, const std::vector<word> &B)
{
    std::vector<word> result;
    const size_t na = A.size(), nb = B.size();
    const size_t combinedN = na|nb;
    if (combinedN == 1)
    {
        word a_hi = A[0] >> WORD_HALF_BITS;
        word a_lo = A[0] & WORD_HALF_MASK;
        word b_hi = B[0] >> WORD_HALF_BITS;
        word b_lo = B[0] & WORD_HALF_MASK;
        result.push_back(A[0] * B[0]);
        word carry = ((a_lo * b_lo) >> WORD_HALF_BITS) + a_hi * b_lo;
        carry = (carry >> WORD_HALF_BITS) + ((a_lo * b_hi + (carry & WORD_HALF_MASK)) >> WORD_HALF_BITS) + a_hi * b_hi;
        if (carry)
            result.push_back(carry);
    }
    else if(combinedN)
    {
        const size_t m2 = (na >= nb) ? (na / 2 + (na & 1)) : (nb / 2 + (nb & 1));
        std::vector<word> a0, a1, b0, b1;
        if (na > m2)
        {
            auto a_split = std::next(A.begin(), m2);
            a0 = std::vector<word>(A.begin(), a_split);
            a1 = std::vector<word>(a_split, A.end());
        }
        else
            a0 = A;
        if (nb > m2)
        {
            auto b_split = std::next(B.begin(), m2);
            b0 = std::vector<word>(B.begin(), b_split);
            b1 = std::vector<word>(b_split, B.end());
        }
        else
            b0 = B;
        const std::vector<word> z0 = karatsuba(a0, b0);
        const std::vector<word> z1 = karatsuba(a1, b1);
        result = z1;
        add_word(a0, a1);
        add_word(b0, b1);
        std::vector<word> mid = karatsuba(a0, b0);
        sub_word(mid, z0);
        sub_word(mid, z1);
        result.insert(result.begin(), m2, 0);
        add_word(result, mid);
        result.insert(result.begin(), m2, 0);
        add_word(result, z0);
        while (result.size() && !result.back())
            result.pop_back();
    }
    return result;
}


//initialize BigInteger functions

//Constructors
BigInteger::BigInteger() : neg(false) {}
BigInteger::BigInteger(const BigInteger &a) : neg(a.neg), words(a.words) {}
BigInteger::BigInteger(const signed &i) : neg(i < 0)
{
    unsigned u = abs(i);
    while (u)
    {
        words.push_back(u);
        size_t i = WORD_BITS;
        while (i--)
            u >>= 1;
    }
}

// in constructor should begin with sign or number, add '\0' in last world to close number
BigInteger::BigInteger(const char *C) : neg(false)
{
    const char *c = C;
    // read sign
    if (*c == '-')
        this->neg = true, c++;
    // read digits
    word carry, tmp, a_hi, a_lo, tp;
    while (*c)
    {
        carry = 0;
        size_t i = 0, j = words.size();
        while (i < j)
        {
            tmp = this->words[i] * 10;
            carry = (tmp += carry) < carry;
            a_hi = this->words[i] >> WORD_HALF_BITS;
            a_lo = this->words[i] & WORD_HALF_MASK;
            tp = ((a_lo * 10) >> WORD_HALF_BITS) + a_hi * 10;
            carry += (tp >> WORD_HALF_BITS) + ((tp & WORD_HALF_MASK) >> WORD_HALF_BITS);
            this->words[i++] = tmp;
        }
        if (carry)
            words.push_back(carry);
        i = 0, j = words.size();
        carry = *c - '0';
        if (carry > 9)
            throw("BigInteger should initialize with number from 0 to 9.");
        add_a_word(this->words, 0, carry);
        c++;
    }
} 

BigInteger::BigInteger(const std::vector<word> &v, bool neg = false) : neg(neg), words(v) {}

// Destructors
BigInteger::~BigInteger()
{
    words.clear();
}

void BigInteger::set_bit(size_t i)
{
    const size_t i_word = i / WORD_BITS, i_bit = i % WORD_BITS;
    if (words.size() <= i_word)
        words.resize(i_word + 1, 0);
    this->words[i_word] |= ((word)1) << i_bit;
}

size_t BigInteger::tot() const
{
    return words.size() * WORD_BITS;
}

char *BigInteger::to_chars() const
{
    std::vector<char> text;
    std::vector<word> A = this->words;
    text.reserve(LOG2BITS * A.size() + 2);
    word remainder, current;
    while (A.size())
    {
        remainder = 0;
        for (auto cur = A.rbegin(), end = A.rend(); cur != end; cur++) 
        {
            current = *cur;
            remainder <<= WORD_HALF_BITS;
            remainder |= current >> WORD_HALF_BITS;
            *cur <<= WORD_HALF_BITS;
            *cur |= remainder / 10;
            remainder %= 10;
            remainder <<= WORD_HALF_BITS;
            remainder |= current & WORD_HALF_MASK;
            *cur <<= WORD_HALF_BITS;
            *cur |= remainder / 10;
            remainder %= 10;
        }
        text.push_back('0' + char(remainder));
        if (A.size() && !A.back())
            A.pop_back();
    }
    if (this->neg)
        text.push_back('-');
    std::reverse(text.begin(), text.end());
    text.push_back('\0');
    char *result = new char[text.size()];
    std::copy(text.begin(), text.end(), result);
    return result;
}

double BigInteger::to_double() const
{
    if (!words.size())
        return 0.0;
    double d = 0.0, base = std::pow(2.0, WORD_BITS);
    for (size_t i = words.size(); i--;)
        d = d * base + this->words[i];
    return neg ? -d : d;
}

bool BigInteger::can_convert_to_int(int *result) const
{
    if (words.size())
    {
        if (words.size() > 1 || this->words[0] >= (WORD_MASK >> 1))
            return false;
        *result = this->words[0];
        if (neg)
            *result = -(*result);
    }
    else
        *result = 0;
    return true;
}

BigInteger BigInteger::sqrt() const
{
    BigInteger n = *this;
    size_t bit;
    if (!this->words.size())
        bit = 0;
    else
    {
        bit = (this->words.size() - 1) * WORD_BITS;
        for (word a = words.back(); a; a >>= 1)
            bit++;
        if (bit & 1)
            bit ^= 1;
    }
    BigInteger result;
    for (; bit >= 0; bit -= 2)
    {
        BigInteger tmp = result;
        tmp.set_bit(bit);
        if (n >= tmp)
        {
            n -= tmp;
            result.set_bit(bit + 1);
        }
        result >>= 1;
    }
    return result;
}
//re-initialize
BigInteger &BigInteger::operator=(const signed &a)
{
    if (this->words.size())
        this->words.clear();
    this->neg = (a < 0);
    unsigned u = abs(a);
    size_t i;
    while (u)
    {
        this->words.push_back(u);
        i = WORD_BITS;
        while (i--)
            u >>= 1;
    }
    return *this;
}
BigInteger &BigInteger::operator=(const BigInteger &a)
{
    this->words = a.words;
    this->neg = a.neg;
    return *this;
}

BigInteger &BigInteger::operator--()
{
    if (this->neg)
        add_a_word(this->words);
    else
        sub_a_word(this->words);
    return *this;
}
BigInteger &BigInteger::operator++()
{
    if (this->neg)
        sub_a_word(this->words);
    else
        add_a_word(this->words);
    return *this;
}

BigInteger &BigInteger::operator+=(const BigInteger &b)
{
    return *this = (*this + b);
}

BigInteger &BigInteger::operator-=(const BigInteger &b)
{
    return *this = (*this - b);
}

BigInteger &BigInteger::operator*=(const BigInteger &b)
{
    if (!b.words.size())
    {
        this->neg = false;
        this->words.clear();
    }
    if (!this->words.size())
        return *this;
    this->neg ^= b.neg;
    this->words = karatsuba(this->words, b.words);
    return *this;
}

BigInteger &BigInteger::operator/=(const BigInteger &b)
{
    std::vector<word> rem = this->words, div = b.words;
    this->words.clear();
    if (compare(rem, div) >= 0)
    {
        word carry0 = 0, carry1;
        this->neg ^= b.neg;
        //shifting count
        size_t j = rem.size();
        size_t i = div.size();
        size_t n = (j - i) * WORD_BITS;
        for (carry0 = rem.back(); carry0; carry0 >>= 1)
            n++;
        for (carry1 = div.back(); carry1; carry1 >>= 1)
            n--;
        const size_t n_bits = n % WORD_BITS;
        if (n_bits)
        {
            const size_t l_shift = WORD_BITS - n_bits;
            carry0 = div.back();
            carry1 = carry0 >> l_shift;
            if (carry1)
                div.push_back(carry1);
            i--;
            while (i--)
            {
                carry1 = div[i];
                div[i + 1] = (carry0 << n_bits) | (carry1 >> l_shift);
                carry0 = carry1;
            }
            div[0] = carry0 << n_bits;
        }
        const size_t n_words = n / WORD_BITS;
        if (n_words)
            div.insert(div.begin(), n_words, 0);
        this->words.reserve(n_words + 1);
        words.resize(n_words + 1, 0);
        do
        {
            if (compare(rem, div) >= 0)
            {
                sub_word(rem, div);
                this->words[n / WORD_BITS] |= word(1) << (n % WORD_BITS);
            }
            //reverse shift one by one
            word hi, lo = div[0];
            for (i = 0, j = div.size() - 1; i < j; i++)
            {
                hi = div[i + 1];
                div[i] = (hi << WORD_BITS_1) | (lo >> 1);
                lo = hi;
            }
            div.back() = lo >> 1;
            if (!div.back())
                div.pop_back();
        } while (n-- && rem.size());
        while (this->words.size() && !this->words.back())
            this->words.pop_back();
    }
    return *this;
}
BigInteger &BigInteger::operator%=(const BigInteger &b)
{
    std::vector<word> &rem = this->words, div = b.words;
    if (compare(rem, div) >= 0)
    {
        //shifting count
        word carry0, carry1;
        size_t j = rem.size();
        size_t i = div.size();
        size_t n = (j - i) * WORD_BITS;
        for (carry0 = rem.back(); carry0; carry0 >>= 1)
            n++;
        for (carry1 = div.back(); carry1; carry1 >>= 1)
            n--;
        const size_t n_bits = n % WORD_BITS;
        if (n_bits)
        {
            const size_t l_shift = WORD_BITS - n_bits;
            carry0 = div.back();
            carry1 = carry0 >> l_shift;
            if (carry1)
                div.push_back(carry1);
            i--;
            while (i--)
            {
                carry1 = div[i];
                div[i + 1] = (carry0 << n_bits) | (carry1 >> l_shift);
                carry0 = carry1;
            }
            div[0] = carry0 << n_bits;
        }
        const size_t n_words = n / WORD_BITS;
        if (n_words)
            div.insert(div.begin(), n_words, 0);
        do
        {
            if (compare(rem, div) >= 0)
                sub_word(rem, div);
            //reverse shift one by one
            word hi, lo = div[0];
            for (i = 0, j = div.size() - 1; i < j; i++)
            {
                hi = div[i + 1];
                div[i] = (hi << WORD_BITS_1) | (lo >> 1);
                lo = hi;
            }
            div.back() = lo >> 1;
            if (!div.back())
                div.pop_back();
        } while (n-- && rem.size());
    }
    return *this;
}

BigInteger &BigInteger::operator^=(size_t exponent)
{
    std::vector<word> p = this->words, &r = this->words;
    r = std::vector<word>{1};
    this->neg = this->neg & (exponent & 1);
    while (exponent)
    {
        if (exponent & 1)
        {
            if (r.size() < p.size())
                r.resize(p.size(), 0);
            r = karatsuba(r, p);
        }
        exponent >>= 1;
        p = karatsuba(p, p);
    }
    return *this;
}


bool BigInteger::operator==(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return false;
    return compare(this->words, b.words) == 0;
}
bool BigInteger::operator!=(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return true;
    return compare(this->words,b.words) != 0;
}
bool BigInteger::operator<=(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return this->neg;
    return 0 >= (compare(this->words, b.words) * (this->neg ? -1 : +1));
}
bool BigInteger::operator>=(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return !this->neg;
    return 0 <= (compare(this->words, b.words) * (this->neg ? -1 : +1));
}
bool BigInteger::operator<(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return this->neg;
    return 0 > (compare(this->words, b.words) * (this->neg ? -1 : +1));
}
bool BigInteger::operator>(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return !this->neg;
    return 0 < (compare(this->words, b.words) * (this->neg ? -1 : +1));
}

BigInteger BigInteger::operator+(const BigInteger &b) const
{
    if (this->neg == b.neg)
    {
        BigInteger r(*this);
        add_word(r.words, b.words);
        return r;
    }
    else
    {
        switch (compare(this->words, b.words))
        {
            case 1:
            {
                BigInteger r(*this);
                sub_word(r.words, b.words);
                return r;
            }
            case -1:
            {
                BigInteger r(b.words);
                sub_word(r.words, this->words);
                return r;
            }
            case 0:
            default:
                return BigInteger();
        }
    }
}

BigInteger BigInteger::operator-(const BigInteger &b) const
{
    if (this->neg != b.neg)
    {
        BigInteger r(*this);
        add_word(r.words, b.words);
        return r;
    }
    else
    {
        switch (compare(this->words, b.words))
        {
            case 1:
            {
                BigInteger r(*this);
                sub_word(r.words, b.words);
                return r;
            }
            case -1:
            {
                BigInteger r(b.words, !this->neg);
                sub_word(r.words, this->words);
                return r;
            }
            case 0:
            default:
                return BigInteger();
        }
    }
}

BigInteger BigInteger::operator*(const BigInteger &b) const
{
    return BigInteger(*this) *= b;
}

BigInteger BigInteger::operator/(const BigInteger &b) const
{
    return BigInteger(*this) /= b;
}

BigInteger BigInteger::operator%(const BigInteger &b) const
{
    return BigInteger(*this) %= b;
}

BigInteger BigInteger::operator^(size_t exponent) const
{
    return BigInteger(*this) ^= exponent;
}

BigInteger BigInteger::operator-() const
{
    return BigInteger(this->words, !this->neg);
}

BigInteger &BigInteger::operator>>=(size_t n_bits)
{
    if (n_bits)
    {
        size_t j = n_bits / WORD_BITS;
        if (j >= words.size())
        {
            this->words.clear();
            return *this;
        }
        if (j)
            this->words.erase(this->words.begin(), this->words.begin() + j);
        const size_t n_len = words.size();
        n_bits %= WORD_BITS;
        if (n_bits)
        {
            word hi, lo = this->words[0];
            const size_t r_shift = WORD_BITS - n_bits;
            for (size_t i = 0; i < (n_len - 1); i++)
            {
                hi = this->words[i + 1];
                this->words[i] = (hi << r_shift) | (lo >> n_bits);
                lo = hi;
            }
            this->words.back() = lo >> n_bits;
            while (this->words.size() && !this->words.back())
                this->words.pop_back();
        }
    }
    return *this;
}

BigInteger &BigInteger::operator<<=(size_t n_bits)
{
    if (!n_bits)
        return *this;
    const size_t j = n_bits / WORD_BITS;
    n_bits %= WORD_BITS;
    if (n_bits)
    {
        const size_t l_shift = WORD_BITS - n_bits;
        size_t i = this->words.size() - 1;
        word hi = this->words.back(), lo = hi >> l_shift;
        if (lo)
            this->words.push_back(lo);
        while (i--)
        {
            lo = this->words[i];
            this->words[i + 1] = (hi << n_bits) | (lo >> l_shift);
            hi = lo;
        }
        this->words[0] = hi << n_bits;
    }
    if (j)
        this->words.insert(this->words.begin(), j, 0);
    return *this;
}

BigInteger BigInteger::operator>>(size_t n_bits) const { return BigInteger(*this) >>= n_bits; }
BigInteger BigInteger::operator<<(size_t n_bits) const { return BigInteger(*this) <<= n_bits; }

std::ostream &operator<<(std::ostream &out, const BigInteger &num)
{
    std::vector<char> text;
    std::vector<word> A = num.words;
    text.reserve(LOG2BITS * A.size());
    word remainder, current;
    while (A.size())
    {
        remainder = 0;
        for (auto cur = A.rbegin(); cur != A.rend(); cur++)
        {
            current = *cur;
            remainder <<= WORD_HALF_BITS;
            remainder |= current >> WORD_HALF_BITS;
            *cur = remainder / 10;
            remainder %= 10;
            remainder <<= WORD_HALF_BITS;
            remainder |= current & WORD_HALF_MASK;
            *cur <<= WORD_HALF_BITS;
            *cur |= remainder / 10;
            remainder %= 10;
        }
        text.push_back('0' + char(remainder));
        if (A.size() && !A.back())
            A.pop_back();
    }
    if (num.neg)
        out << "-";
    std::reverse(text.begin(), text.end());
    text.push_back('\0');
    out << text.data();
    return out;
}


