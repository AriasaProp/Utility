#include "BigInteger.h"
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdlib>

const size_t WORD_BITS = sizeof(word) * CHAR_BIT;
const size_t WORD_BITS_1 = WORD_BITS - 1;
const word WORD_MASK = (word)-1;
const size_t WORD_HALF_BITS = sizeof(word) * CHAR_BIT / 2;
const word WORD_HALF_MASK = WORD_MASK >> WORD_HALF_BITS;
const long double LOG2BITS = std::log10(2.99999) * WORD_BITS;

//private function for repeated use

// +1 mean a is greater, -1 mean a is less, 0 mean equal
int compare(const std::vector<word> a, const std::vector<word> b)
{
    size_t as = a.size(), bs = b.size();
    if (as != bs)
        return as > bs ? +1 : -1;
    while (as--)
        if (a[as] != b[as])
            return a[as] > b[as] ? +1 : -1;
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
//initialize BigInteger functions

//Constructors
BigInteger::BigInteger() {}
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
        n = (n - 1) * WORD_BITS;
        word carry = this->words.back();
        while (carry)
            n++, carry >>= 1;
        if (n & 1)
            n++;
        std::vector<word> remaining(1, 0), &res = result.words;
        res.push_back(0);
        size_t i, j;
        const size_t l_shift = WORD_BITS - 2;
        const word setLast = ~word(1);
        while (n)
        {
            n -= 2;
            //remaining shift left 2
            carry = remaining.back() >> l_shift;
            for (i = remaining.size() - 1; i; i--)
                remaining[i] = (remaining[i] << 2) | (remaining[i - 1] >> l_shift);
            remaining[i] <<= 2;
            remaining[i] |= (this->words[n / WORD_BITS] >> (n % WORD_BITS)) & word(3);
            if (carry)
                remaining.push_back(carry);
            //result shift 1 left with test
            carry = res.back() >> WORD_BITS_1;
            for (i = res.size() - 1; i; i--)
                res[i] = (res[i] << 1) | (res[i - 1] >> WORD_BITS_1);
            res[i] = (res[i] << 1) | 1;
            if (carry)
                res.push_back(carry);
            // compare result test with remaining
            if (compare(remaining, res) >= 0)
            {
                carry = 0;
                for (i = 0, j = res.size(); i < j; i++)
                {
                    carry = remaining[i] < (remaining[i] -= carry);
                    carry += remaining[i] < (remaining[i] -= res[i]);
                }
                for (j = remaining.size(); i < j && carry; i++)
                    carry = remaining[i] < (remaining[i] -= carry);
                res[0] |= 2;
            }
            res[0] &= setLast;
        }
        // shift 1 right to get pure result
        for (i = 0, j = res.size() - 1; i < j; i++)
            res[i] = (res[i + 1] << WORD_BITS_1) | (res[i] >> 1);
        res.back() >>= 1;
        while (res.size() && !res.back())
            res.pop_back();
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

BigInteger &BigInteger::operator+=(const s_word &b)
{
    const word B = word(abs(b));
    if (!this->words.size())
    {
        this->neg = b < 0;
        this->words.push_back(B);
    }
    else if (this->neg == (b < 0))
        add_a_word(this->words, 0, B);
    else
    {
        if ((this->words.size() > 1) || (this->words[0] > B))
            sub_a_word(this->words, 0, B);
        else if (this->words[0] < B)
        {
            this->neg = !this->neg;
            this->words[0] = B - this->words[0];
        }
        else
        {
            this->neg = false;
            this->words.clear();
        }
    }
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
        int cmp = compare(this->words, b.words);
        if (cmp < 0)
        {
            std::vector<word> t = this->words;
            this->neg = b.neg;
            this->words = b.words;
            sub_word(this->words, t);
        }
        else if (cmp > 0)
            sub_word(this->words, b.words);
        else
        {
            this->neg = false;
            this->words.clear();
        }
    }
    return *this;
}

BigInteger &BigInteger::operator-=(const s_word &b)
{
    const word B = word(abs(b));
    if (!this->words.size())
    {
        this->neg = b >= 0;
        this->words.push_back(B);
    }
    else if (this->neg != (b < 0))
        add_a_word(this->words, 0, B);
    else
    {
        if ((this->words.size() > 1) || (this->words[0] > B))
            sub_a_word(this->words, 0, B);
        else if (this->words[0] < B)
        {
            this->neg = !this->neg;
            this->words[0] = B - this->words[0];
        }
        else
        {
            this->neg = false;
            this->words.clear();
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
        int cmp = compare(this->words, b.words);
        if (cmp < 0)
        {
            this->neg = !this->neg;
            std::vector<word> t = this->words;
            this->words = b.words;
            sub_word(this->words, t);
        }
        else if (cmp > 0)
        {
            std::vector<word> t = b.words;
            sub_word(this->words, t);
        }
        else
        {
            this->neg = false;
            this->words.clear();
        }
    }
    return *this;
}

BigInteger &BigInteger::operator*=(const s_word &b)
{
    if (b == 0)
    {
        this->neg = false;
        this->words.clear();
    }
    std::vector<word> &r = this->words;
    const size_t j = r.size();
    if (j)
    {
        this->neg ^= (b < 0);
        const word B = word(abs(b));
        word A[j];
        std::copy(r.begin(), r.end(), A);
        const word b_hi = B >> WORD_HALF_BITS;
        const word b_lo = B & WORD_HALF_MASK;
        word a_hi, a_lo, carry = 0, carry0;
        for (size_t i = 0; i < j; i++)
        {
            word &wA = A[i];
            a_hi = wA >> WORD_HALF_BITS;
            a_lo = wA & WORD_HALF_MASK;
            r[i] = wA * B;
            carry = (r[i] += carry) < carry;
            carry0 = (a_lo * b_lo) >> WORD_HALF_BITS;
            carry0 += a_hi * b_lo;
            carry += carry0 >> WORD_HALF_BITS;
            carry += (a_lo * b_hi + (carry0 & WORD_HALF_MASK)) >> WORD_HALF_BITS;
            carry += a_hi * b_hi;
        }
        if (carry)
            r.push_back(carry);
    }
    return *this;
}

BigInteger &BigInteger::operator*=(const BigInteger &b)
{
    const size_t nb = b.words.size();
    if (!nb)
    {
        this->neg = false;
        this->words.clear();
    }
    std::vector<word> &a = this->words;
    const size_t na = a.size();
    if (na)
    {
        this->neg ^= b.neg;
        word A[na], B[nb];
        std::copy(a.begin(), a.end(), A);
        std::copy(b.words.begin(), b.words.end(), B);
        a.clear();
        a.resize(na + nb, 0);
        word a_hi, b_hi, a_lo, b_lo, carry, carry0;
        size_t ia, ib, i, j;
        for (ia = 0; ia < na; ia++)
        {
            const word &Ar = A[ia];
            a_hi = Ar >> WORD_HALF_BITS;
            a_lo = Ar & WORD_HALF_MASK;
            carry = 0;
            for (ib = 0; ib < nb; ib++)
            {
                i = ia + ib;
                word &r = a[i];
                carry = (r += carry) < carry;
                const word &Br = B[ib];
                b_hi = Br >> WORD_HALF_BITS;
                b_lo = Br & WORD_HALF_MASK;
                // part 1
                carry0 = Ar * Br;
                carry += (r += carry0) < carry0;
                //part 2
                carry0 = (a_lo * b_lo) >> WORD_HALF_BITS;
                carry0 += a_hi * b_lo;
                //TODO:  this may overflow, if you find error in multiplication check this
                carry += carry0 >> WORD_HALF_BITS;
                carry += (a_lo * b_hi + (carry0 & WORD_HALF_MASK)) >> WORD_HALF_BITS;
                carry += a_hi * b_hi;
            }
            j = a.size();
            while (((++i) < j) && carry)
                carry = (a[i] += carry) < carry;
            if (carry)
                a.push_back(carry);
        }
        while (a.size() && !a.back())
            a.pop_back();
    }
    return *this;
}

BigInteger &BigInteger::operator/=(const s_word &b)
{
    if (b == 0)
        throw("Undefined number cause / 0 !");
    const word B = word(abs(b));
    std::vector<word> rem = this->words, div{B};
    this->words.clear();
    int cmp = compare(rem, div);
    if (cmp == 0)
        this->words.push_back(1);
    else if (cmp > 0)
    {
        //shifting count
        size_t i = div.size();
        size_t j = (rem.size() - i) * WORD_BITS;
        word carry0 = rem.back();
        while (carry0)
            j++, carry0 >>= 1;
        carry0 = div.back();
        while (carry0)
            j--, carry0 >>= 1;
        size_t n = j % WORD_BITS;
        if (n)
        {
            const size_t l_shift = WORD_BITS - n;
            carry0 = div.back() >> l_shift;
            while (--i)
                div[i] = (div[i] << n) | (div[i - 1] >> l_shift);
            div[i] <<= n;
            if (carry0)
                div.push_back(carry0);
        }
        n = j / WORD_BITS;
        if (n)
            div.insert(div.begin(), n, 0);
        this->words.resize(n + 1, 0);
        do
        {
            cmp = compare(rem, div);
            if (cmp >= 0)
            {
                this->words[j / WORD_BITS] |= word(1) << (j % WORD_BITS);
                if (cmp > 0)
                    sub_word(rem, div);
                else if (cmp == 0)
                {
                    rem.clear();
                    break;
                }
            }
            //reverse shift one by one
            for (i = 0, n = div.size() - 1; i < n; i++)
                div[i] = (div[i] >> 1) | (div[i + 1] << WORD_BITS_1);
            div[i] >>= 1;
            if (!div.back())
                div.pop_back();
        } while (j--);
        while (this->words.size() && !this->words.back())
            this->words.pop_back();
    }
    // sign set
    if (this->words.size())
        this->neg ^= (b < 0);
    else
        this->neg = false;
    // return result
    return *this;
}

BigInteger &BigInteger::operator/=(const BigInteger &b)
{
    if (!b.words.size())
        throw("Undefined number cause / 0 !");
    std::vector<word> rem = this->words, div = b.words;
    this->words.clear();
    int cmp = compare(rem, div);
    if (cmp == 0)
        this->words.push_back(1);
    else if (cmp > 0)
    {
        //shifting count
        size_t i = div.size();
        size_t j = (rem.size() - i) * WORD_BITS;
        word carry0 = rem.back();
        while (carry0)
            j++, carry0 >>= 1;
        carry0 = div.back();
        while (carry0)
            j--, carry0 >>= 1;
        size_t n = j % WORD_BITS;
        if (n)
        {
            const size_t l_shift = WORD_BITS - n;
            carry0 = div.back() >> l_shift;
            while (--i)
                div[i] = (div[i] << n) | (div[i - 1] >> l_shift);
            div[i] <<= n;
            if (carry0)
                div.push_back(carry0);
        }
        n = j / WORD_BITS;
        if (n)
            div.insert(div.begin(), n, 0);
        this->words.resize(n + 1, 0);
        do
        {
            cmp = compare(rem, div);
            if (cmp >= 0)
            {
                this->words[j / WORD_BITS] |= word(1) << (j % WORD_BITS);
                if (cmp > 0)
                    sub_word(rem, div);
                else if (cmp == 0)
                {
                    rem.clear();
                    break;
                }
            }
            //reverse shift one by one
            for (i = 0, n = div.size() - 1; i < n; i++)
                div[i] = (div[i] >> 1) | (div[i + 1] << WORD_BITS_1);
            div[i] >>= 1;
            if (!div.back())
                div.pop_back();
        } while (j--);
        while (this->words.size() && !this->words.back())
            this->words.pop_back();
    }
    // sign set
    if (this->words.size())
        this->neg ^= b.neg;
    else
        this->neg = false;
    // return result
    return *this;
}

BigInteger &BigInteger::operator%=(const s_word &b)
{
    if (b != 0)
    {
        const word B = word(abs(b));
        std::vector<word> &rem = this->words, div{B};
        int cmp = compare(rem, div);
        if (cmp == 0)
        {
            this->neg = false;
            rem.clear();
        }
        else if (cmp > 0)
        {
            //shifting count
            size_t i = div.size();
            size_t j = (rem.size() - i) * WORD_BITS;
            word carry0 = rem.back();
            while (carry0)
                j++, carry0 >>= 1;
            carry0 = div.back();
            while (carry0)
                j--, carry0 >>= 1;
            size_t n = j % WORD_BITS;
            if (n)
            {
                const size_t l_shift = WORD_BITS - n;
                carry0 = div.back() >> l_shift;
                while (--i)
                    div[i] = (div[i] << n) | (div[i - 1] >> l_shift);
                div[i] <<= n;
                if (carry0)
                    div.push_back(carry0);
            }
            n = j / WORD_BITS;
            if (n)
                div.insert(div.begin(), n, 0);
            do
            {
                cmp = compare(rem, div);
                if (cmp > 0)
                    sub_word(rem, div);
                else if (cmp == 0)
                {
                    this->neg = false;
                    rem.clear();
                    break;
                }
                //reverse shift one by one
                for (i = 0, n = div.size() - 1; i < n; i++)
                    div[i] = (div[i] >> 1) | (div[i + 1] << WORD_BITS_1);
                div[i] >>= 1;
                if (!div.back())
                    div.pop_back();
            } while (j--);
        }
    }
    return *this;
}

BigInteger &BigInteger::operator%=(const BigInteger &b)
{
    if (b.words.size())
    {
        std::vector<word> &rem = this->words, div = b.words;
        int cmp = compare(rem, div);
        if (cmp == 0)
        {
            this->neg = false;
            rem.clear();
        }
        else if (cmp > 0)
        {
            //shifting count
            size_t i = div.size();
            size_t j = (rem.size() - i) * WORD_BITS;
            word carry0 = rem.back();
            while (carry0)
                j++, carry0 >>= 1;
            carry0 = div.back();
            while (carry0)
                j--, carry0 >>= 1;
            size_t n = j % WORD_BITS;
            if (n)
            {
                const size_t l_shift = WORD_BITS - n;
                carry0 = div.back() >> l_shift;
                while (--i)
                    div[i] = (div[i] << n) | (div[i - 1] >> l_shift);
                div[i] <<= n;
                if (carry0)
                    div.push_back(carry0);
            }
            n = j / WORD_BITS;
            if (n)
                div.insert(div.begin(), n, 0);
            do
            {
                cmp = compare(rem, div);
                if (cmp > 0)
                    sub_word(rem, div);
                else if (cmp == 0)
                {
                    this->neg = false;
                    rem.clear();
                    break;
                }
                //reverse shift one by one
                for (i = 0, n = div.size() - 1; i < n; i++)
                    div[i] = (div[i] >> 1) | (div[i + 1] << WORD_BITS_1);
                div[i] >>= 1;
                if (!div.back())
                    div.pop_back();
            } while (j--);
        }
    }
    return *this;
}

BigInteger &BigInteger::operator^=(size_t exponent)
{
    size_t na = this->words.size();
    if (na)
    {
        std::vector<word> p = this->words, &R = this->words;
        R.clear();
        R.push_back(1);
        word A[na * exponent], B[na * exponent];
        this->neg = this->neg & (exponent & 1);
        word a_hi, a_lo, b_hi, b_lo, carry, carry0;
        size_t ia, ib, i, j, nb;
        while (exponent)
        {
            nb = p.size();
            if (exponent & 1)
            {
                na = R.size();
                std::copy(R.begin(), R.end(), A);
                std::copy(p.begin(), p.end(), B);
                R.clear();
                R.resize(na + nb, 0);
                for (ia = 0; ia < na; ia++)
                {
                    const word &Ar = A[ia];
                    a_hi = Ar >> WORD_HALF_BITS;
                    a_lo = Ar & WORD_HALF_MASK;
                    carry = 0;
                    for (ib = 0; ib < nb; ib++)
                    {
                        i = ia + ib;
                        word &r = R[i];
                        carry = (r += carry) < carry;
                        const word &Br = B[ib];
                        b_hi = Br >> WORD_HALF_BITS;
                        b_lo = Br & WORD_HALF_MASK;
                        // part 1
                        carry0 = Ar * Br;
                        carry += (r += carry0) < carry0;
                        //part 2
                        carry0 = a_lo * b_lo;
                        carry0 >>= WORD_HALF_BITS;
                        carry0 += a_hi * b_lo;
                        //TODO:  this may overflow, if you find error in multiplication check this
                        carry += (carry0 >> WORD_HALF_BITS) + ((a_lo * b_hi + (carry0 & WORD_HALF_MASK)) >> WORD_HALF_BITS) + a_hi * b_hi;
                    }
                    j = R.size();
                    while (((++i) < j) && carry)
                        carry = (R[i] += carry) < carry;
                    if (carry)
                        R.push_back(carry);
                }
                while (R.size() && !R.back())
                    R.pop_back();
            }
            exponent >>= 1;
            {
                std::copy(p.begin(), p.end(), A);
                p.clear();
                p.resize(nb * 2, 0);
                for (ia = 0; ia < nb; ia++)
                {
                    const word &Ar = A[ia];
                    a_hi = Ar >> WORD_HALF_BITS;
                    a_lo = Ar & WORD_HALF_MASK;
                    carry = 0;
                    for (ib = 0; ib < nb; ib++)
                    {
                        i = ia + ib;
                        word &r = p[i];
                        carry = (r += carry) < carry;
                        const word &Br = A[ib];
                        b_hi = Br >> WORD_HALF_BITS;
                        b_lo = Br & WORD_HALF_MASK;
                        // part 1
                        carry0 = Ar * Br;
                        carry += (r += carry0) < carry0;
                        //part 2
                        carry0 = a_lo * b_lo;
                        carry0 >>= WORD_HALF_BITS;
                        carry0 += a_hi * b_lo;
                        //TODO:  this may overflow, if you find error in multiplication check this
                        carry += (carry0 >> WORD_HALF_BITS) + ((a_lo * b_hi + (carry0 & WORD_HALF_MASK)) >> WORD_HALF_BITS) + a_hi * b_hi;
                    }
                    j = p.size();
                    while (((++i) < j) && carry)
                        carry = (p[i] += carry) < carry;
                    if (carry)
                        p.push_back(carry);
                }
                while (p.size() && !p.back())
                    p.pop_back();
            }
        }
    }
    else if (!exponent)
        throw("Undefined result!");
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
                *carried >>= n_bits;
                while (carried != endCarried)
                {
                    *carried |= *(carried + 1) << r_shift;
                    carried++;
                    *carried >>= n_bits;
                }
                if (*endCarried == 0)
                    this->words.pop_back();
            }
        }
        else
        {
            this->neg = false;
            this->words.clear();
        }
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
    return compare(this->words, b.words) != 0;
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
        return b.neg;
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

BigInteger BigInteger::operator+(const s_word &b) const
{
    BigInteger r;
    const word B = word(abs(b));
    if (!this->words.size())
    {
        r.neg = b < 0;
        r.words.push_back(B);
    }
    else if (this->neg == (b < 0))
    {
        r.words = this->words;
        r.neg = this->neg;
        add_a_word(r.words, 0, B);
    }
    else
    {
        if ((this->words.size() > 1) || (this->words[0] > B))
        {
            r.neg = this->neg;
            r.words = this->words;
            sub_a_word(r.words, 0, B);
        }
        else if (this->words[0] < B)
        {
            r.neg = !this->neg;
            r.words.push_back(B - this->words[0]);
        }
    }
    return r;
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
        int cmp = compare(this->words, b.words);
        if (cmp > 0)
        {
            r.words = this->words;
            r.neg = this->neg;
            sub_word(r.words, b.words);
        }
        else if (cmp < 0)
        {
            r.words = b.words;
            r.neg = b.neg;
            sub_word(r.words, this->words);
        }
    }
    return r;
}

BigInteger BigInteger::operator-(const s_word &b) const
{
    BigInteger r;
    const word B = word(abs(b));
    if (!this->words.size())
    {
        r.neg = b >= 0;
        r.words.push_back(B);
    }
    else if (this->neg != (b < 0))
    {
        r.words = this->words;
        r.neg = this->neg;
        add_a_word(r.words, 0, B);
    }
    else
    {
        if ((this->words.size() > 1) || (this->words[0] > B))
        {
            r.words = this->words;
            r.neg = this->neg;
            sub_a_word(r.words, 0, B);
        }
        else if (this->words[0] < B)
        {
            r.neg = !this->neg;
            r.words[0] = B - this->words[0];
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
        int cmp = compare(this->words, b.words);
        if (cmp > 0)
        {
            r.words = this->words;
            r.neg = this->neg;
            sub_word(r.words, b.words);
        }
        else if (cmp < 0)
        {
            r.words = b.words;
            r.neg = !this->neg;
            sub_word(r.words, this->words);
        }
    }
    return r;
}

BigInteger BigInteger::operator*(const s_word &b) const
{
    return BigInteger(*this) *= b;
}

BigInteger BigInteger::operator*(const BigInteger &b) const
{
    return BigInteger(*this) *= b;
}

BigInteger BigInteger::operator/(const s_word &b) const
{
    return BigInteger(*this) /= b;
}

BigInteger BigInteger::operator/(const BigInteger &b) const
{
    return BigInteger(*this) /= b;
}

BigInteger BigInteger::operator%(const s_word &b) const
{
    return BigInteger(*this) %= b;
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
