#include <cmath>
#include <chrono>
#include <iomanip>
#include "utils/BigInteger.h"
//Constructors
BigInteger::BigInteger() : neg(false) {}
BigInteger::BigInteger(size_t n, word w, bool neg = false) : neg(neg), words(n, w) {}
BigInteger::BigInteger(const word *a, const word *b, bool neg = false) : neg(neg), words(a, b) {}
BigInteger::BigInteger(const BigInteger &a) : neg(a.neg)
{
    words = a.words;
}
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
        neg = true, c++;
    // read digits
    for (; *c; c++)
    {
        word b = *c - '0';
        if (b >= 10)
            throw("BigInteger should initialize with number from 0 to 9.");
        word carry = 0, tmp, a_hi, a_lo, tp;
        size_t i;
        for (i = 0; i < words.size(); i++)
        {
            tmp = this->words[i] * 10;
            carry = ((tmp += carry) < carry);
            a_hi = this->words[i] >> WORD_HALF_BITS;
            a_lo = this->words[i] & WORD_HALF_MASK;
            tp = ((a_lo * 10) >> WORD_HALF_BITS) + a_hi * 10;
            carry += (tp >> WORD_HALF_BITS) + ((tp & WORD_HALF_MASK) >> WORD_HALF_BITS);
            this->words[i] = tmp;
        }
        if (carry)
            words.push_back(carry);
        for (i = 0; i < words.size() && b; i++)
            b = (this->words[i] += b) < b;
        if (b)
            words.push_back(b);
    }
    while (words.size() && !words.back())
        words.pop_back();
}

BigInteger::BigInteger(const std::vector<word> &a, bool neg = false) : neg(neg), words(a) {}
// Destructors
BigInteger::~BigInteger()
{
    words.clear();
}

size_t BigInteger::bitlength() const
{
    if (!words.size())
        return 0;
    size_t i = (words.size() - 1) * WORD_BITS;
    word a = words.back();
    while (a >>= 1)
        i++;
    return i;
}

size_t BigInteger::count_trailing_zeros() const
{
    for (size_t i = 0; i < words.size(); i++)
    {
        word w = this->words[i];
        if (w)
        {
            size_t count = 0;
            for (size_t j = 0; j < WORD_BITS; j++)
                if ((w >> j) & 1)
                    count += j;
            return count + i * WORD_BITS;
        }
    }
    return 0;
}

BigInteger &BigInteger::set_bit(size_t i)
{
    const size_t i_word = i / WORD_BITS, i_bit = i % WORD_BITS;
    if (words.size() <= i_word)
        words.resize(i_word + 1);
    this->words[i_word] |= ((word)1) << i_bit;
    return *this;
}

word BigInteger::get_bit(size_t i) const
{
    const size_t i_word = i / WORD_BITS, i_bit = i % WORD_BITS;
    if (i_word >= words.size())
        return 0;
    return (this->words[i_word] >> i_bit) & 1;
}

void BigInteger::clr_bit(size_t i)
{
    const size_t i_word = i / WORD_BITS, i_bit = i % WORD_BITS;
    if (i_word >= words.size())
        return;
    this->words[i_word] &= ~(1 << i_bit);
}

size_t BigInteger::tot() const
{
    return words.size() * WORD_BITS;
}

// TODO : need to be more efficiently
char *BigInteger::to_chars() const
{
    std::vector<char> text;
    std::vector<word> A = this->words;
    text.reserve(std::log(2) * A.size() * WORD_BITS + 2);
    word remainder, current;
    while (A.size())
    {
        remainder = 0;
        for (size_t i = A.size(); i--;)
        {
            current = A[i];
            A[i] = 0;
            //part 1
            remainder <<= WORD_HALF_BITS;
            remainder |= current >> WORD_HALF_BITS;
            A[i] <<= WORD_HALF_BITS;
            A[i] |= remainder / 10;
            remainder %= 10;
            //part 2
            remainder <<= WORD_HALF_BITS;
            remainder |= current & WORD_HALF_MASK;
            A[i] <<= WORD_HALF_BITS;
            A[i] |= remainder / 10;
            remainder %= 10;
        }
        while (A.size() && !A.back())
            A.pop_back();
        text.push_back('0' + char(remainder));
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

BigInteger BigInteger::pow(size_t exponent) const
{
    BigInteger result(1), p(*this);
    for (; exponent; exponent >>= 1)
    {
        if (exponent & 1)
        {
            result = result * p;
            exponent--;
        }
        p *= p;
    }
    return result;
}

BigInteger BigInteger::mod_pow(BigInteger exponent, const BigInteger &modulus) const
{
    BigInteger result(1), base = (*this) % modulus;
    for (; exponent.words.size() > 0; exponent >>= 1)
    {
        if (exponent.get_bit(0))
            result = (result * base) % modulus;
        base = (base * base) % modulus;
    }
    return result;
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
BigInteger &BigInteger::operator=(const signed &a)
{
    neg = (a < 0);
    if (words.size())
        words.clear();
    unsigned u = abs(a);
    while (u)
    {
        words.push_back(u);
        size_t i = WORD_BITS;
        while (i--)
            u >>= 1;
    }
    return *this;
}
BigInteger &BigInteger::operator=(const BigInteger &a)
{
    words = a.words;
    neg = a.neg;
    return *this;
}

BigInteger &BigInteger::operator--()
{
    word carry0 = 1;
    size_t i;
    const size_t n = words.size();
    if (this->neg)
    {
        for (i = 0; i < n && carry0; i++)
            carry0 = ((this->words[i] += carry0) < carry0);
        if (carry0)
            this->words.push_back(carry0);
    }
    else
    {
        for (i = 0; i < n && carry0; i++)
            carry0 = this->words[i] < (this->words[i] -= carry0);
        while (words.size() && !words.back())
            words.pop_back();
    }
    return *this;
}
BigInteger &BigInteger::operator++()
{
    word carry0 = 1;
    size_t i;
    const size_t n = words.size();
    if (this->neg)
    {
        for (i = 0; i < n && carry0; i++)
            carry0 = this->words[i] < (this->words[i] -= carry0);
        while (words.size() && !words.back())
            words.pop_back();
    }
    else
    {
        for (i = 0; i < n && carry0; i++)
            carry0 = ((this->words[i] += carry0) < carry0);
        if (carry0)
            this->words.push_back(carry0);
    }
    return *this;
}

BigInteger &BigInteger::operator+=(const BigInteger &b)
{
    word carry0 = 0, carry1;
    std::vector<word> &A = this->words, B = b.words;
    size_t na = A.size(), nb = B.size(), i;
    if (this->neg == b.neg)
    {
        const size_t n = na >= nb ? na : nb;
        A.resize(n);
        for (i = 0; i < nb; i++)
        {
            carry0 = ((A[i] += carry0) < carry0);
            carry0 += ((A[i] += B[i]) < B[i]);
        }
        for (; i < n && carry0; i++)
            carry0 = ((A[i] += carry0) < carry0);
        if (carry0)
            A.push_back(carry0);
    }
    else
    {
        if (na != nb)
        {
            if (na < nb)
            {
                A.swap(B);
                this->neg = !this->neg;
                na = A.size(), nb = B.size();
            }
        }
        else
        {
            i = na;
            while (i--)
            {
                carry0 = A[i], carry1 = B[i];
                if (carry0 != carry1)
                {
                    if (carry0 < carry1)
                    {
                        A.swap(B);
                        this->neg = !this->neg;
                    }
                    break;
                }
            }
        }
        carry0 = 0;
        for (i = 0; i < nb; i++)
        {
            carry0 = A[i] < (A[i] -= carry0);
            carry0 += A[i] < (A[i] -= B[i]);
        }
        for (; i < na && carry0; i++)
            carry0 = A[i] < (A[i] -= carry0);
        while (A.size() && !A.back())
            A.pop_back();
    }
    return *this;
}

BigInteger &BigInteger::operator-=(const BigInteger &b)
{
    word carry0 = 0, carry1;
    std::vector<word> &A = this->words, B = b.words;
    size_t na = A.size(), nb = B.size(), i;
    if (this->neg != b.neg)
    {
        const size_t n = na >= nb ? na : nb;
        A.resize(n);
        for (i = 0; i < nb; i++)
        {
            carry0 = (A[i] += carry0) < carry0;
            carry0 += (A[i] += B[i]) < B[i];
        }
        for (; i < n && carry0; i++)
            carry0 = (A[i] += carry0) < carry0;
        if (carry0)
            A.push_back(carry0);
    }
    else
    {
        if (na != nb)
        {
            if (na < nb)
            {
                A.swap(B);
                na = A.size(), nb = B.size();
                this->neg = !this->neg;
            }
        }
        else
        {
            i = na;
            while (i--)
            {
                carry0 = A[i], carry1 = B[i];
                if (carry0 != carry1)
                {
                    if (carry0 < carry1)
                    {
                        A.swap(B);
                        this->neg = !this->neg;
                    }
                    break;
                }
            }
        }
        carry0 = 0;
        for (i = 0; i < nb; i++)
        {
            carry0 = A[i] < (A[i] -= carry0);
            carry0 += A[i] < (A[i] -= B[i]);
        }
        for (; i < na && carry0; i++)
            carry0 = A[i] < (A[i] -= carry0);
        while (A.size() && !A.back())
            A.pop_back();
    }
    return *this;
}
BigInteger &BigInteger::operator*=(const BigInteger &b)
{
    if (!b.words.size())
    {
        words.clear();
        neg = false;
    }
    if (!this->words.size())
        return *this;

    const size_t na = this->words.size(), nb = b.words.size();
    this->neg ^= b.neg;
    if ((na <= nb ? na : nb) > 20)
    {
        const size_t n = na >= nb ? na : nb;
        const size_t m2 = n / 2 + (n & 1);
        std::vector<word> A = this->words, B = b.words;
        A.resize(m2 * 2);
        B.resize(m2 * 2);
        BigInteger a0(m2, 0), a1(m2, 0), b0(m2, 0), b1(m2, 0);
        auto a_split = std::next(A.cbegin(), m2);
        std::copy(A.cbegin(), a_split, a0.words.begin());
        std::copy(a_split, A.cend(), a1.words.begin());
        auto b_split = std::next(B.cbegin(), m2);
        std::copy(B.cbegin(), b_split, b0.words.begin());
        std::copy(b_split, B.cend(), b1.words.begin());
        const BigInteger z0 = a0 * b0, z1 = a1 * b1;
        *this = z1;
        this->words.insert(this->words.begin(), m2, 0);
        *this += (a1 + a0) * (b0 + b1) - z1 - z0;
        this->words.insert(this->words.begin(), m2, 0);
        *this += z0;
        return *this;
    }
    const std::vector<word> aw = this->words, &bw = b.words;
    this->words.clear();
    this->words.resize(na + nb + 1, 0);
    word carry[2], A[2], B[2]; // temporary value
    for (size_t ia = 0, ib; ia < na; ia++)
    {
        A[1] = aw[ia] >> WORD_HALF_BITS;
        A[0] = aw[ia] & WORD_HALF_MASK;
        for (ib = 0; ib < nb; ib++)
        {
            auto i = this->words.begin() + ia + ib;
            auto j = this->words.end();
            B[1] = bw[ib] >> WORD_HALF_BITS;
            B[0] = bw[ib] & WORD_HALF_MASK;
            carry[0] = aw[ia] * bw[ib];
            carry[1] = ((A[0] * B[0]) >> WORD_HALF_BITS) + A[1] * B[0];
            carry[0] = (*(i++) += carry[0]) < carry[0];
            carry[1] = (carry[1] >> WORD_HALF_BITS) + ((A[0] * B[1] + (carry[1] & WORD_HALF_MASK)) >> WORD_HALF_BITS) + A[1] * B[1];
            carry[0] = ((*i += carry[0]) < carry[0]) + ((*(i++) += carry[1]) < carry[1]);
            while (carry[0] && (i != j))
                carry[0] = (*(i++) += carry[0]) < carry[0];
            if (carry[0])
                this->words.push_back(carry[0]);
        }
    }
    while (this->words.size() && !this->words.back())
        this->words.pop_back();
    return *this;
}

BigInteger &BigInteger::operator/=(const BigInteger &b)
{
    word carry0 = 0, carry1;
    std::vector<word> rem = this->words, div = b.words;
    this->words.clear();
    //compare function
    bool cmp = true;
    size_t na = rem.size(), nb = div.size(), i;
    if (na != nb)
        cmp = na > nb;
    else
    {
        i = na;
        while (i--)
        {
            carry0 = rem[i];
            carry1 = div[i];
            if (carry0 != carry1)
            {
                cmp = carry0 > carry1;
                break;
            }
        }
    }
    if (cmp)
    {
        this->neg ^= b.neg;
        //shifting count
        size_t n = (na - nb) * WORD_BITS;
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
            for (i = nb - 1; i--;)
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
        do
        {
            //compare function
            cmp = true;
            na = rem.size(), nb = div.size();
            if (na != nb)
                cmp = na > nb;
            else
            {
                i = na;
                while (i--)
                {
                    carry0 = rem[i];
                    carry1 = div[i];
                    if (carry0 != carry1)
                    {
                        cmp = carry0 > carry1;
                        break;
                    }
                }
            }
            if (cmp)
            {
                carry0 = 0;
                for (i = 0; i < nb; i++)
                {
                    carry0 = rem[i] < (rem[i] -= carry0);
                    carry0 += rem[i] < (rem[i] -= div[i]);
                }
                for (; i < na && carry0; i++)
                {
                    carry0 = rem[i] < (rem[i] -= carry0);
                }
                while (rem.size() && !rem.back())
                    rem.pop_back();
                set_bit(n);
            }
            //reverse shift one by one
            nb--;
            word hi, lo = div[0];
            for (i = 0; i < nb; i++)
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
BigInteger &BigInteger::operator%=(const BigInteger &b)
{
    //compare function
    bool cmp = true;
    word carry0, carry1;
    std::vector<word> &rem = this->words, div = b.words;
    size_t na = rem.size(), nb = div.size(), i;
    if (na != nb)
        cmp = na > nb;
    else
    {
        i = na;
        while (i--)
        {
            carry0 = rem[i];
            carry1 = div[i];
            if (carry0 != carry1)
            {
                cmp = carry0 > carry1;
                break;
            }
        }
    }
    if (cmp)
    {
        //shifting count
        size_t n = (na - nb) * WORD_BITS;
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
            for (i = nb - 1; i--;)
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
            //compare function
            cmp = true;
            na = rem.size(), nb = div.size();
            if (na != nb)
                cmp = na > nb;
            else
            {
                i = na;
                while (i--)
                {
                    carry0 = rem[i];
                    carry1 = div[i];
                    if (carry0 != carry1)
                    {
                        cmp = carry0 > carry1;
                        break;
                    }
                }
            }
            if (cmp)
            {
                carry0 = 0;
                for (i = 0; i < nb; i++)
                {
                    carry0 = rem[i] < (rem[i] -= carry0);
                    carry0 += rem[i] < (rem[i] -= div[i]);
                }
                for (; i < na && carry0; i++)
                    carry0 = rem[i] < (rem[i] -= carry0);
                while (rem.size() && !rem.back())
                    rem.pop_back();
            }
            //reverse shift one by one
            nb--;
            word hi, lo = div[0];
            for (i = 0; i < nb; i++)
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

bool BigInteger::operator==(const BigInteger &b) const
{
    if (this->neg != b.neg || this->words.size() != b.words.size())
        return false;
    for (size_t i = this->words.size(); i--;)
        if (this->words[i] != b.words[i])
            return false;
    return true;
}
bool BigInteger::operator!=(const BigInteger &b) const
{
    if (this->neg != b.neg || this->words.size() != b.words.size())
        return true;
    for (size_t i = this->words.size(); i--;)
        if (this->words[i] != b.words[i])
            return true;
    return false;
}
bool BigInteger::operator<=(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return this->neg;
    size_t i = this->words.size(), nb = b.words.size();
    if (i != nb)
        return (this->neg) ? i > nb : i < nb;
    word A, B;
    while (i--)
    {
        A = this->words[i];
        B = b.words[i];
        if (A != B)
            return (this->neg) ? A > B : A < B;
    }
    return true;
}
bool BigInteger::operator>=(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return !this->neg;
    size_t i = this->words.size(), nb = b.words.size();
    if (i != nb)
        return this->neg ? i < nb : i > nb;
    word A, B;
    while (i--)
    {
        A = this->words[i];
        B = b.words[i];
        if (A != B)
            return (this->neg) ? A < B : A > B;
    }
    return true;
}
bool BigInteger::operator<(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return this->neg;
    size_t i = this->words.size(), nb = b.words.size();
    if (i != nb)
        return (this->neg) ? i > nb : i < nb;
    word A, B;
    while (i--)
    {
        A = this->words[i];
        B = b.words[i];
        if (A != B)
            return (this->neg) ? A > B : A < B;
    }
    return false;
}
bool BigInteger::operator>(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return !this->neg;
    size_t i = this->words.size(), nb = b.words.size();
    if (i != nb)
        return (this->neg) ? i < nb : i > nb;
    word A, B;
    while (i--)
    {
        A = this->words[i];
        B = b.words[i];
        if (A != B)
            return (this->neg) ? A < B : A > B;
    }
    return false;
}

BigInteger BigInteger::operator+(const BigInteger &b) const
{
    BigInteger r(*this);
    word carry0 = 0, carry1;
    std::vector<word> &A = r.words, B = b.words;
    size_t na = A.size(), nb = B.size(), i;
    if (r.neg == b.neg)
    {
        const size_t n = std::max(na, nb);
        A.resize(n);
        for (i = 0; i < nb; i++)
        {
            carry0 = (A[i] += carry0) < carry0;
            carry0 += (A[i] += B[i]) < B[i];
        }
        for (; i < n && carry0; i++)
            carry0 = ((A[i] += carry0) < carry0);
        if (carry0)
            A.push_back(carry0);
    }
    else
    {
        if (na != nb)
        {
            if (na < nb)
            {
                A.swap(B);
                na = A.size(), nb = B.size();
                r.neg = !r.neg;
            }
        }
        else
        {
            i = na;
            while (i--)
            {
                carry0 = A[i], carry1 = B[i];
                if (carry0 != carry1)
                {
                    if (carry0 < carry1)
                    {
                        A.swap(B);
                        r.neg = !r.neg;
                    }
                    break;
                }
            }
        }
        carry0 = 0;
        for (i = 0; i < nb; i++)
        {
            carry0 = A[i] < (A[i] -= carry0);
            carry0 += A[i] < (A[i] -= B[i]);
        }
        for (; i < na && carry0; i++)
            carry0 = A[i] < (A[i] -= carry0);
        while (A.size() && !A.back())
            A.pop_back();
    }
    return r;
}

BigInteger BigInteger::operator-(const BigInteger &b) const
{
    BigInteger r(*this);
    std::vector<word> &A = r.words, B = b.words;
    size_t na = A.size(), nb = B.size(), i;
    word carry0 = 0, carry1 = 0;
    if (this->neg != b.neg)
    {
        const size_t n = std::max(na, nb);
        A.resize(n);
        for (i = 0; i < nb; i++)
        {
            carry0 = (A[i] += carry0) < carry0;
            carry0 += (A[i] += B[i]) < B[i];
        }
        for (; i < n && carry0; i++)
            carry0 = ((A[i] += carry0) < carry0);
        if (carry0)
            A.push_back(carry0);
    }
    else
    {
        if (na != nb)
        {
            if (na < nb)
            {
                A.swap(B);
                na = A.size(), nb = B.size();
                r.neg = !r.neg;
            }
        }
        else
        {
            i = na;
            while (i--)
            {
                carry0 = A[i], carry1 = B[i];
                if (carry0 != carry1)
                {
                    if (carry0 < carry1)
                    {
                        A.swap(B);
                        r.neg = !r.neg;
                    }
                    break;
                }
            }
        }
        carry0 = 0;
        for (i = 0; i < nb; i++)
        {
            carry0 = A[i] < (A[i] -= carry0);
            carry0 += A[i] < (A[i] -= B[i]);
        }
        for (; i < na && carry0; i++)
            carry0 = A[i] < (A[i] -= carry0);
        while (A.size() && !A.back())
            A.pop_back();
    }
    return r;
}

BigInteger BigInteger::operator*(const BigInteger &b) const
{
    if (!b.words.size() || !this->words.size())
        return BigInteger();
    const size_t na = this->words.size(), nb = b.words.size();
    BigInteger result;
    result.neg = this->neg ^ b.neg;
    if ((na <= nb ? na : nb) > 20)
    {
        const size_t n = na >= nb ? na : nb;
        const size_t m2 = n / 2 + (n & 1);
        std::vector<word> A = this->words, B = b.words;
        A.resize(m2 * 2, 0);
        B.resize(m2 * 2, 0);
        BigInteger a0(m2, 0), a1(m2, 0), b0(m2, 0), b1(m2, 0);
        const auto a_split = std::next(A.cbegin(), m2);
        std::copy(A.cbegin(), a_split, a0.words.begin());
        std::copy(a_split, A.cend(), a1.words.begin());
        const auto b_split = std::next(B.cbegin(), m2);
        std::copy(B.cbegin(), b_split, b0.words.begin());
        std::copy(b_split, B.cend(), b1.words.begin());
        const BigInteger z0 = a0 * b0, z1 = a1 * b1;
        result = z1;
        result.words.insert(result.words.begin(), m2, 0);
        result += (a1 + a0) * (b0 + b1) - z1 - z0;
        result.words.insert(result.words.begin(), m2, 0);
        result += z0;
        return result;
    }
    std::vector<word> &rsw = result.words;
    rsw.resize(na + nb + 1);
    word carry[2], A[2], B[2]; // temporary value
    for (size_t ia = 0, ib; ia < na; ia++)
    {
        A[1] = this->words[ia] >> WORD_HALF_BITS;
        A[0] = this->words[ia] & WORD_HALF_MASK;
        for (ib = 0; ib < nb; ib++)
        {
            auto i = rsw.begin() + ia + ib;
            auto j = rsw.end();
            B[1] = b.words[ib] >> WORD_HALF_BITS;
            B[0] = b.words[ib] & WORD_HALF_MASK;
            carry[0] = this->words[ia] * b.words[ib];
            carry[1] = ((A[0] * B[0]) >> WORD_HALF_BITS) + A[1] * B[0];
            carry[0] = (*(i++) += carry[0]) < carry[0];
            carry[1] = (carry[1] >> WORD_HALF_BITS) + ((A[0] * B[1] + (carry[1] & WORD_HALF_MASK)) >> WORD_HALF_BITS) + A[1] * B[1];
            carry[0] = ((*i += carry[0]) < carry[0]) + ((*(i++) += carry[1]) < carry[1]);
            while (carry[0] && (i != j))
                carry[0] = (*(i++) += carry[0]) < carry[0];
            if (carry[0])
                rsw.push_back(carry[0]);
        }
    }
    while (rsw.size() && !rsw.back())
        rsw.pop_back();
    return result;
}

BigInteger BigInteger::operator/(const BigInteger &b) const
{
    BigInteger quotient; //compare function
    bool cmp = true;
    word carry0, carry1;
    std::vector<word> rem = this->words, div = b.words;
    size_t na = rem.size(), nb = div.size(), i;
    if (na != nb)
        cmp = na > nb;
    else
    {
        i = na;
        while (i--)
        {
            carry0 = rem[i];
            carry1 = div[i];
            if (carry0 != carry1)
            {
                cmp = carry0 > carry1;
                break;
            }
        }
    }
    if (cmp)
    {
        //shifting count
        size_t n = (na - nb) * WORD_BITS;
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
            for (i = nb - 1; i--;)
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
        quotient.words.reserve(n_words + 1);
        for (; n && (na = rem.size()); n--)
        {
            //compare function
            cmp = true;
            nb = div.size();
            if (na != nb)
                cmp = na > nb;
            else
            {
                i = na;
                while (i--)
                {
                    carry0 = rem[i];
                    carry1 = div[i];
                    if (carry0 != carry1)
                    {
                        cmp = carry0 > carry1;
                        break;
                    }
                }
            }
            if (cmp)
            {
                carry0 = 0;
                for (i = 0; i < nb; i++)
                {
                    carry0 = rem[i] < (rem[i] -= carry0);
                    carry0 += rem[i] < (rem[i] -= div[i]);
                }
                for (; i < na && carry0; i++)
                {
                    carry0 = rem[i] < (rem[i] -= carry0);
                }
                while (rem.size() && !rem.back())
                    rem.pop_back();
                quotient.set_bit(n);
            }
            nb--;
            word hi, lo = div[0];
            for (i = 0; i < nb; i++)
            {
                hi = div[i + 1];
                div[i] = (hi << WORD_BITS_1) | (lo >> 1);
                lo = hi;
            }
            div.back() = lo >> 1;
            if (!div.back())
                div.pop_back();
        }
        quotient.neg = this->neg ^ b.neg;
    }
    return quotient;
}

BigInteger BigInteger::operator%(const BigInteger &b) const
{
    BigInteger remainder(*this);
    //compare function
    bool cmp = true;
    word carry0 = 0, carry1;
    std::vector<word> &rem = remainder.words, div = b.words;
    size_t na = rem.size(), nb = div.size(), i;
    if (na != nb)
        cmp = na > nb;
    else
    {
        i = na;
        while (i--)
        {
            carry0 = rem[i];
            carry1 = div[i];
            if (carry0 != carry1)
            {
                cmp = carry0 > carry1;
                break;
            }
        }
    }
    if (cmp)
    {
        //shifting count
        size_t n = (na - nb) * WORD_BITS;
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
            for (i = nb - 1; i--;)
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
        for (; n && (na = rem.size()); n--)
        {
            //compare function
            cmp = true;
            nb = div.size();
            if (na != nb)
                cmp = na > nb;
            else
            {
                i = na;
                while (i--)
                {
                    carry0 = rem[i];
                    carry1 = div[i];
                    if (carry0 != carry1)
                    {
                        cmp = carry0 > carry1;
                        break;
                    }
                }
            }
            if (cmp)
            {
                carry0 = 0;
                for (i = 0; i < nb; i++)
                {
                    carry0 = rem[i] < (rem[i] -= carry0);
                    carry0 += rem[i] < (rem[i] -= div[i]);
                }
                for (; i < na && carry0; i++)
                    carry0 = rem[i] < (rem[i] -= carry0);
                while (rem.size() && !rem.back())
                    rem.pop_back();
            }
            nb--;
            word hi, lo = div[0];
            for (i = 0; i < nb; i++)
            {
                hi = div[i + 1];
                div[i] = (hi << WORD_BITS_1) | (lo >> 1);
                lo = hi;
            }
            div.back() = lo >> 1;
            if (!div.back())
                div.pop_back();
        }
    }
    return remainder;
}

BigInteger BigInteger::operator-() const
{
    BigInteger ret(*this);
    ret.neg = !this->neg;
    return ret;
}

BigInteger &BigInteger::operator>>=(size_t n_bits)
{
    if (!n_bits)
        return *this;
    size_t j = n_bits / WORD_BITS;
    if (j >= words.size())
    {
        words.resize(0);
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
    text.reserve(std::log(2) * A.size() * WORD_BITS);
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
        while (A.size() && !A.back())
            A.pop_back();
    }
    if (num.neg)
        out << "-";
    std::reverse(text.begin(), text.end());
    text.push_back('\0');
    out << text.data();
    return out;
}
#include <random>
auto gen = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine());
int main(int argc, char *argv[])
{
	std::cout << "pi generator test" << std::endl;
	std::FILE *file = std::fopen("piDigits.txt", "r");
	if (!file)
	{
		std::cout << "failed load pi samples" << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "file loaded " << std::endl;
	std::cout << "BigInteger output test : " << std::endl;
	try
	{
		long double rate, endCount;
		BigInteger q = 1, r = 0, t = 1, k = 1, l = 3, n = 3;
		int N = 3, check;
		uintmax_t correct = 0;
		auto restart = std::chrono::high_resolution_clock::now();
		//std::chrono::duration<long double, std::micro> endCount;
		while (!std::feof(file) && correct < 5000)
		{
			if (!n.can_convert_to_int(&N))
			{
				std::cout << n << std::endl;
				throw "n value broken";
			}
			if ((q * 4 + r - t) < (t * n))
			{
				check = std::getc(file) - '0';
				if (N != check)
				{
					std::cout << std::endl;
					std::cout << N << " != " << check << std::endl;
					throw "pi digits result wrong";
				}
				//correct++;
				endCount = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<long double, std::micro>(std::chrono::high_resolution_clock::now() - restart)).count();
				rate = (long double)correct * 1000000. / endCount;
				std::cout << '\r' << std::flush << " pi : " << std::setw(7) << std::setfill('0') << (++correct) << " rate : " << std::setw(7) << std::setfill('0') << std::setprecision(10) << rate << " digit/sec";
				n = ((q * 3 + r) - n * t) * 10 / t;
				r = (r - t * N) * 10;
				q *= 10;
			}
			else
			{
				n = (q * (k * 7 + 2) + r * l) / (t * l);
				r = (q * 2 + r) * l;
				q *= k;
				t *= l;
				++k;
				l += 2;
			}
		}
		endCount = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<long double, std::micro>(std::chrono::high_resolution_clock::now() - restart)).count();
		rate = endCount / 1000000.;
		std::cout << std::endl
				  << std::setw(10) << std::setfill('0') << std::setprecision(10) << rate << " seconds for 5000 digits correct";
	}
	catch (const char *ex)
	{
		std::cout << std::endl
				  << ex;
	}
	std::fclose(file);
	delete (file);
}

