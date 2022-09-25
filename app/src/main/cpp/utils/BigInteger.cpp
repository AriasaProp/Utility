#include <cmath>
#include <algorithm>
#include "utils/BigInteger.h"

const size_t WORD_BITS = sizeof(word) * CHAR_BIT;
const size_t WORD_BITS_1 = WORD_BITS - 1;
const word WORD_MASK = (word)-1;
const size_t WORD_HALF_BITS = sizeof(word) * CHAR_BIT / 2;
const word WORD_HALF_MASK = WORD_MASK >> WORD_HALF_BITS;
const long double LOG2BITS = std::log10(2.99999) * WORD_BITS;

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
    const size_t j = b.size();
    if (!j)
        return;
    if (a.size() < j)
        a.resize(j, 0);
    size_t i = 0;
    word carry = 0;
    while (i < j)
    {
        carry = (a[i] += carry) < carry;
        carry += (a[i] += b[i]) < b[i];
        i++;
    }
    add_a_word(a, i, carry);
}
// a should be greater or equal than b
// a should clone from the base, b not
void sub_word(std::vector<word> &a, const std::vector<word> &b)
{
    const size_t j = b.size();
    if (!j)
        return;
    word carry = 0;
    size_t i = 0;
    while (i < j)
    {
        carry = a[i] < (a[i] -= carry);
        carry += a[i] < (a[i] -= b[i]);
        i++;
    }
    sub_a_word(a, i, carry);
}

void karatsuba(std::vector<word> &dst, std::vector<word> A, std::vector<word> B)
{
    dst.clear();
    const size_t na = A.size(), nb = B.size();
    const size_t combinedN = na | nb;
    if (combinedN)
    {
        dst.reserve(na + nb);
        if (combinedN > 1)
        {
            const size_t m2 = (na >= nb) ? (na / 2 + (na & 1)) : (nb / 2 + (nb & 1)), M = m2 * 2;
            if (A.size() < M)
                A.resize(M, 0);
            auto split = std::next(A.begin(), m2);
            std::vector<word> a1 = std::vector<word>(split, A.end());
            A.resize(m2);
            if (B.size() < M)
                B.resize(M, 0);
            split = std::next(B.begin(), m2);
            std::vector<word> b1 = std::vector<word>(split, B.end());
            B.resize(m2);
            karatsuba(dst, a1, b1);
            add_word(a1, A);
            add_word(b1, B);
            karatsuba(a1, a1, b1);
            karatsuba(b1, A, B);
            sub_word(a1, b1);
            sub_word(a1, dst);
            dst.insert(dst.begin(), m2, 0);
            add_word(dst, a1);
            dst.insert(dst.begin(), m2, 0);
            add_word(dst, b1);
            while (dst.size() && !dst.back())
                dst.pop_back();
        }
        else if (A[0] && B[0])
        {
            word a_hi = A[0] >> WORD_HALF_BITS;
            word a_lo = A[0] & WORD_HALF_MASK;
            word b_hi = B[0] >> WORD_HALF_BITS;
            word b_lo = B[0] & WORD_HALF_MASK;
            dst.push_back(A[0] * B[0]);
            word carry = ((a_lo * b_lo) >> WORD_HALF_BITS) + a_hi * b_lo;
            carry = (carry >> WORD_HALF_BITS) + ((a_lo * b_hi + (carry & WORD_HALF_MASK)) >> WORD_HALF_BITS) + a_hi * b_hi;
            if (carry)
                dst.push_back(carry);
        }
    }
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
BigInteger::BigInteger(const char *c) : neg(false)
{
    // read sign
    if (*c == '-')
        this->neg = true, c++;
    // read digits
    word carry, tmp, a_hi, a_lo, tp;
    size_t i, j;
    while (*c)
    {
        carry = *c - '0';
        if (carry > 9)
            throw("BigInteger should initialize with number from 0 to 9.");
        i = 0, j = this->words.size();
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
        c++;
    }
} 

BigInteger::BigInteger(const std::vector<word> &v, bool neg = false) : neg(neg), words(v) {}

// Destructors
BigInteger::~BigInteger()
{
    words.clear();
}
//environment count
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
//math operational
BigInteger BigInteger::sqrt() const
{
    BigInteger result;
    size_t n = this->words.size();
    if (n)
    {
        int bit_l = (n - 1) * WORD_BITS;
        word carry = this->words.back();
        while (carry)
            bit_l++, carry >>= 1;
        if (bit_l & 1)
            bit_l++;
        BigInteger remaining, temp_red;
        while ((bit_l -= 2) >= 0)
        {
            carry = (this->words[bit_l / WORD_BITS] >> (bit_l % WORD_BITS)) & ((word)3);
            remaining <<= 2;
            remaining += carry;
            if (result.words.size())
            {
                result <<= 1;
                temp_red = result;
                temp_red <<= 1;
            }
            if (!temp_red.words.size())
                temp_red.words.push_back(0);
            carry = 1;
            temp_red.words[0] |= carry;
            if (compare(remaining.words, temp_red.words) >= 0)
            {
                sub_word(remaining.words, temp_red.words);
                if (!result.words.size())
                    result.words.push_back(0);
                result.words[0] |= carry;
            }
        }
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
    if (this->neg == b.neg)
    {
        std::vector<word> t = b.words;
        add_word(this->words, t);
    }
    else
    {
        switch (compare(this->words, b.words))
        {
            case -1:
            {
                std::vector<word> t = this->words;
                this->neg = b.neg;
                this->words = b.words;
                sub_word(this->words, t);
            }
                break;
            case 1:
                sub_word(this->words, b.words);
                break;
            case 0:
            default:
                this->words.clear();
                break;
        }
    }
    return *this;
}

BigInteger &BigInteger::operator-=(const BigInteger &b)
{
    if (this->neg != b.neg)
        add_word(this->words, b.words);
    else
    {
        switch (compare(this->words, b.words))
        {
            case -1:
            {
                this->neg = !this->neg;
                std::vector<word> t = this->words;
                this->words = b.words;
                sub_word(this->words, t);
            }
                break;
            case 1:
            {
                std::vector<word> t = b.words;
                sub_word(this->words, t);
            }
                break;
            case 0:
            default:
                this->words.clear();
                break;
        }
    }
    return *this;
}

BigInteger &BigInteger::operator*=(const BigInteger &b)
{
    if (!b.words.size())
    {
        this->neg = false;
        this->words.clear();
    }
    if (this->words.size())
    {
        this->neg ^= b.neg;
        karatsuba(this->words, this->words, b.words);
    }
    return *this;
}

BigInteger &BigInteger::operator/=(const BigInteger &b)
{
    std::vector<word> rem = this->words, div = b.words;
    this->words.clear();
    if (compare(rem, div) >= 0)
    {
        word carry0, carry1;
        this->neg ^= b.neg;
        //shifting count
        size_t j = rem.size();
        size_t i = div.size();
        j = (j - i) * WORD_BITS;
        for (carry0 = rem.back(); carry0; carry0 >>= 1)
            j++;
        for (carry1 = div.back(); carry1; carry1 >>= 1)
            j--;
        size_t n = j % WORD_BITS;
        if (n)
        {
            const size_t l_shift = WORD_BITS - n;
            carry0 = div.back();
            carry1 = carry0 >> l_shift;
            if (carry1)
                div.push_back(carry1);
            i--;
            while (i--)
            {
                carry1 = div[i];
                div[i + 1] = (carry0 << n) | (carry1 >> l_shift);
                carry0 = carry1;
            }
            div[0] = carry0 << n;
        }
        n = j / WORD_BITS;
        if (n)
            div.insert(div.begin(), n, 0);
        this->words.resize(n + 1, 0);
        do
        {
            if (compare(rem, div) >= 0)
            {
                sub_word(rem, div);
                this->words[j / WORD_BITS] |= word(1) << (j % WORD_BITS);
            }
            //reverse shift one by one
            carry1 = div[0];
            for (i = 0, n = div.size() - 1; i < n; i++)
            {
                carry0 = div[i + 1];
                div[i] = (carry0 << WORD_BITS_1) | (carry1 >> 1);
                carry1 = carry0;
            }
            div.back() = carry1 >> 1;
            if (!div.back())
                div.pop_back();
        } while (j-- && rem.size());
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
        j = (j - i) * WORD_BITS;
        for (carry0 = rem.back(); carry0; carry0 >>= 1)
            j++;
        for (carry1 = div.back(); carry1; carry1 >>= 1)
            j--;
        size_t n = j % WORD_BITS;
        if (n)
        {
            const size_t l_shift = WORD_BITS - n;
            carry0 = div.back();
            carry1 = carry0 >> l_shift;
            if (carry1)
                div.push_back(carry1);
            i--;
            while (i--)
            {
                carry1 = div[i];
                div[i + 1] = (carry0 << n) | (carry1 >> l_shift);
                carry0 = carry1;
            }
            div[0] = carry0 << n;
        }
        n = j / WORD_BITS;
        if (n)
            div.insert(div.begin(), n, 0);
        do
        {
            if (compare(rem, div) >= 0)
                sub_word(rem, div);
            //reverse shift one by one
            carry1 = div[0];
            for (i = 0, n = div.size() - 1; i < n; i++)
            {
                carry0 = div[i + 1];
                div[i] = (carry0 << WORD_BITS_1) | (carry1 >> 1);
                carry1 = carry0;
            }
            div.back() = carry1 >> 1;
            if (!div.back())
                div.pop_back();
        } while (j-- && rem.size());
    }
    return *this;
}

BigInteger &BigInteger::operator^=(size_t exponent)
{
    if (this->words.size())
    {
        std::vector<word> p = this->words, &r = this->words;
        r = std::vector<word>{1};
        this->neg = this->neg & (exponent & 1);
        while (exponent)
        {
            if (exponent & 1)
                karatsuba(r, r, p);
            exponent >>= 1;
            karatsuba(p, p, p);
        }
    }
    else if (!exponent)
        throw ("Undefined result!");
    return *this;
}

BigInteger &BigInteger::operator>>=(size_t n_bits)
{
    if (n_bits && this->words.size())
    {
        size_t j = n_bits / WORD_BITS;
        if (j < words.size())
        {
            std::vector<word>::iterator carried = this->words.begin();
            this->words.erase(carried, carried + j);
            n_bits %= WORD_BITS;
            if (n_bits)
            {
                std::vector<word>::iterator endCarried = this->words.end() - 1;
                const size_t r_shift = WORD_BITS - n_bits;
                while (carried != endCarried)
                {
                    *carried >>= n_bits;
                    *carried |= *(carried + 1) << r_shift;
                    carried++;
                }
                *carried >>= n_bits;
                if (*carried) 
                    this->words.pop_back();
            }
        }
        else
            this->words.clear();
    }
    return *this;
}

BigInteger &BigInteger::operator<<=(size_t bits)
{
    if (bits)
    {
        size_t n = bits % WORD_BITS;
        if (n)
        {
            const size_t l_shift = WORD_BITS - n;
            std::vector<word>::reverse_iterator carried = this->words.rbegin();
            std::vector<word>::reverse_iterator endCarried = this->words.rend() - 1;
            word lo = *carried >> l_shift;
            while (carried != endCarried)
            {
                *carried <<= n;
                *carried |= *(carried + 1) >> l_shift;
                carried++;
            }
            *carried <<= n;
            if (lo)
                this->words.push_back(lo);
        }
        n = bits / WORD_BITS;
        if (n)
            this->words.insert(this->words.begin(), n, 0);
    }
    return *this;
}
//compare operator
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
    BigInteger r;
    if (this->neg == b.neg)
    {
        r.words = this->words;
        r.neg = this->neg;
        add_word(r.words, b.words);
    }
    else
    {
        switch (compare(this->words, b.words))
        {
            case 1:
            {
                r.words = this->words;
                r.neg = this->neg;
                sub_word(r.words, b.words);
                break;
            }
            case -1:
            {
                r.words = b.words;
                r.neg = b.neg;
                sub_word(r.words, this->words);
                break;
            }
            case 0:
            default:
                break;
        }
    }
    return r;
}

BigInteger BigInteger::operator-(const BigInteger &b) const
{
    BigInteger r;
    if (this->neg != b.neg)
    {
        r.words = this->words;
        r.neg = this->neg;
        add_word(r.words, b.words);
    }
    else
    {
        switch (compare(this->words, b.words))
        {
            case 1:
            {
                r.words = this->words;
                r.neg = this->neg;
                sub_word(r.words, b.words);
                break;
            }
            case -1:
            {
                r.words = b.words;
                r.neg = b.neg;
                sub_word(r.words, this->words);
                break;
            }
            case 0:
            default:
                break;
        }
    }
    return r;
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

BigInteger BigInteger::operator-() const { return BigInteger(this->words, !this->neg); }
BigInteger BigInteger::operator>>(size_t n_bits) const { return BigInteger(*this) >>= n_bits; }
BigInteger BigInteger::operator<<(size_t n_bits) const { return BigInteger(*this) <<= n_bits; }

std::ostream &operator<<(std::ostream &out, const BigInteger &num)
{
    std::vector<char> text;
    std::vector<word> A = num.words;
    text.reserve(LOG2BITS * A.size() + 1);
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


