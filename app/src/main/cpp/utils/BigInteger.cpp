#include "BigInteger.h"

BigInteger::BigInteger() : neg(false) {}
BigInteger::BigInteger(size_t n, word w, bool neg = false) : words(n, w), neg(neg) {}
BigInteger::BigInteger(const word *a, const word *b, bool neg = false) : words(a, b), neg(neg) {}
BigInteger::BigInteger(const BigInteger &a) : neg(a.neg), words(a.words) {}
BigInteger::BigInteger(int i) : neg(i < 0)
{
	for (unsigned u = abs(i); u;)
	{
		words.push_back(u);
		// carefully shift 'u' bit-by-bit because u >>= 32 might be undefined
		for (size_t j = 0; j < WORD_BITS; j++)
			u >>= 1;
	}
}
// in constructor should add '\0' in last world to close number
BigInteger::BigInteger(const char *C) : neg(false)
{
	const char *c = C;
	// read digits
	for (; *c; c++)
	{
		// read sign
		if (*c == '-')
		{
			neg = true;
			continue;
		}
		word carry = 0;
		for (size_t i = 0; i < words.size(); i++)
		{
			word a = (*this)[i];
			word tmp = a * 10;
			carry = add_carry(&tmp, carry);
			carry += word_mul_hi(a, 10);
			(*this)[i] = tmp;
		}
		if (carry)
			words.push_back(carry);
		truncate();

		word b = *c - '0';
		if (b >= 10)
			throw("BigInteger should initialize with number from 0 to 9.");
		
		for (size_t i = 0; i < words.size() && b; i++)
			b = add_carry(&(*this)[i], b);
		if (b)
			words.push_back(b);
		truncate();
	}
}
// Destructors
~BigInteger::BigInteger()
{
	words.clear();
}

word BigInteger::&operator[](size_t i)
{
	return words[i];
}

const word BigInteger::&operator[](size_t i) const
{
	return words[i];
}

BigInteger BigInteger::&set_neg(bool neg)
{
	this->neg = neg;
	return *this;
}

BigInteger BigInteger::&truncate()
{
	while (words.size() > 0 && words.back() == 0)
		words.pop_back();
	return *this;
}

size_t BigInteger::bitlength() const
{
	if (words.size() == 0)
		return 0;
	size_t last = words.size() - 1;
	size_t result = word_bitlength((*this)[last]) + last * WORD_BITS;
	return result;
}

size_t BigInteger::count_trailing_zeros() const
{
	for (size_t i = 0; i < words.size(); i++)
	{
		word w = (*this)[i];
		if (w)
			return word_count_trailing_zeros(w) + i * WORD_BITS;
	}
	return 0;
}

static int BigInteger::cmp_abs(const BigInteger &a, const BigInteger &b)
{
	size_t na = a.words.size(), nb = b.words.size();
	if (na != nb)
	{
		return na < nb ? -1 : +1;
	}
	for (size_t i = na; i-- > 0;)
	{
		word wa = a[i], wb = b[i];
		if (wa != wb)
		{
			return wa < wb ? -1 : +1;
		}
	}
	return 0;
}

static int BigInteger::cmp(const BigInteger &a, const BigInteger &b)
{
	if (a.words.size() == 0 && b.words.size() == 0)
		return 0;
	if (!a.neg && !b.neg)
		return +cmp_abs(a, b);
	if (a.neg && b.neg)
		return -cmp_abs(a, b);
	return a.neg && !b.neg ? -1 : +1;
}

static size_t BigInteger::word_bitlength(word a)
{
	for (int i = WORD_BITS - 1; i >= 0; i--)
		if ((a >> i) & 1)
			return i + 1;
	return 0;
}

static size_t BigInteger::word_count_trailing_zeros(word a)
{
	for (int i = 0; i < (int)WORD_BITS; i++)
		if ((a >> i) & 1)
			return i;
	return WORD_BITS;
}

static word BigInteger::add_carry(word *a, word b)
{
	return (*a += b) < b;
}

static word BigInteger::sub_carry(word *a, word b)
{
	word tmp = *a;
	return (*a = tmp - b) > tmp;
}

static word BigInteger::word_mul_hi(word a, word b)
{
	size_t n = WORD_BITS / 2;
	word a_hi = a >> n;
	word a_lo = a & WORD_HALF_MASK;
	word b_hi = b >> n;
	word b_lo = b & WORD_HALF_MASK;
	word tmp = ((a_lo * b_lo) >> n) + a_hi * b_lo;
	tmp = (tmp >> n) + ((a_lo * b_hi + (tmp & WORD_HALF_MASK)) >> n);
	return tmp + a_hi * b_hi;
}

static BigInteger BigInteger::&sub_unsigned_overwrite(BigInteger &a, const BigInteger &b)
{
	//assert(cmp_abs(a, b) >= 0);
	size_t i, na = a.words.size(), nb = b.words.size();
	word carry = 0;
	for (i = 0; i < nb; i++)
	{
		carry = sub_carry(&a[i], carry);
		carry += sub_carry(&a[i], b[i]);
	}
	for (; i < na && carry; i++)
		carry = sub_carry(&a[i], carry);
	//assert(!carry);
	return a.truncate();
}

static BigInteger BigInteger::mul_long(const BigInteger &a, const BigInteger &b)
{
	size_t na = a.words.size(), nb = b.words.size(), nc = na + nb + 1;
	BigInteger c(nc, 0, a.neg ^ b.neg), carries(nc, 0);
	for (size_t ia = 0; ia < na; ia++)
	{
		for (size_t ib = 0; ib < nb; ib++)
		{
			size_t i = ia + ib, j = i + 1;
			// WARNING: Might overflow if word size is chosen too small
			carries[i + 1] += add_carry(&c[i], a[ia] * b[ib]);
			carries[j + 1] += add_carry(&c[j], word_mul_hi(a[ia], b[ib]));
		}
	}
	// adding c + carries
	c.words.resize(nc);
	word carry = 0;
	for (size_t i = 0; i < nc; i++)
	{
		carry = add_carry(&c[i], carry);
		carry += add_carry(&c[i], carries[i]);
	}
	if (carry)
		c.words.push_back(carry);
	return c.truncate();
}

static BigInteger BigInteger::mul_karatsuba(const BigInteger &a, const BigInteger &b)
{
	size_t na = a.words.size(), nb = b.words.size(), n = max(na, nb), m2 = n / 2 + (n & 1);
	BigInteger a_parts[2], b_parts[2];
	split(a, a_parts, 2, m2);
	split(b, b_parts, 2, m2);
	m2 *= WORD_BITS;
	BigInteger z0 = a_parts[0] * b_parts[0];
	BigInteger z1 = (a_parts[0] + a_parts[1]) * (b_parts[0] + b_parts[1]);
	BigInteger z2 = a_parts[1] * b_parts[1];
	BigInteger result = z2;
	result <<= m2;
	result += z1 - z2 - z0;
	result <<= m2;
	result += z0;
	return result;
}

static BigInteger BigInteger::mul(const BigInteger &a, const BigInteger &b)
{
	size_t karatsuba_threshold = 20;
	if (a.words.size() > karatsuba_threshold && b.words.size() > karatsuba_threshold)
	{
		return mul_karatsuba(a, b);
	}
	return mul_long(a, b);
}

static void BigInteger::div_mod(const BigInteger &numerator, BigInteger denominator, BigInteger &quotient, BigInteger &remainder)
{
	quotient = 0;
	remainder = numerator;
	if (cmp_abs(remainder, denominator) >= 0)
	{
		int n = numerator.bitlength() - denominator.bitlength();
		denominator <<= n;
		for (; n >= 0; n--)
		{
			if (cmp_abs(remainder, denominator) >= 0)
			{
				sub_unsigned_overwrite(remainder, denominator);
				quotient.set_bit(n);
			}
			denominator >>= 1;
		}
	}
	quotient.set_neg(numerator.neg ^ denominator.neg);
	remainder.set_neg(numerator.neg);
}
static void BigInteger::split(const BigInteger &a, BigInteger *parts, size_t n_parts, size_t n)
{
	size_t i = 0;
	for (size_t k = 0; k < n_parts; k++)
	{
		BigInteger &part = parts[k];
		part.words.resize(n);
		for (size_t j = 0; j < n && i < a.words.size(); j++)
			part[j] = a[i++];
		part = part.truncate();
	}
}

static BigInteger BigInteger::div(const BigInteger &numerator, const BigInteger &denominator)
{
	BigInteger quotient, remainder;
	div_mod(numerator, denominator, quotient, remainder);
	return quotient;
}

static BigInteger BigInteger::mod(const BigInteger &numerator, const BigInteger &denominator)
{
	BigInteger quotient, remainder;
	div_mod(numerator, denominator, quotient, remainder);
	return remainder;
}

static BigInteger BigInteger::gcd(const BigInteger &a0, const BigInteger &b0)
{
	if (a0.words.size() == 1 && b0.words.size() == 1)
	{
		word c, a = a0[0], b = b0[0];
		while (b)
			c = a, a = b, b = c % b;
		return BigInteger(1, a);
	}

	BigInteger a(a0), b(b0);
	a.neg = b.neg = false;

	if (a.words.size() == 0)
		return b0;
	if (b.words.size() == 0)
		return a0;

	size_t n = a.count_trailing_zeros();
	size_t m = b.count_trailing_zeros();
	if (n > m)
	{
		swap(n, m);
		a.words.swap(b.words);
	}

	a >>= n;
	b >>= n;

	do
	{
		b >>= b.count_trailing_zeros();
		if (cmp_abs(a, b) > 0)
			a.words.swap(b.words);
		sub_unsigned_overwrite(b, a);
	} while (b.words.size() > 0);

	a <<= n;

	return a;
}

typedef void (*random_func)(uint8_t *bytes, size_t n_bytes);

static BigInteger BigInteger::random_bits(size_t n_bits, random_func func)
{
	if (n_bits == 0)
		return 0;
	size_t partial_bits = n_bits % WORD_BITS;
	size_t n_words = n_bits / WORD_BITS + (partial_bits > 0);
	size_t n_bytes = n_words * sizeof(word);
	BigInteger result(n_words, 0);
	uint8_t *bytes = (uint8_t *)&result[0];
	func(bytes, n_bytes);
	if (partial_bits)
	{
		size_t too_many_bits = WORD_BITS - partial_bits;
		result.words.back() >>= too_many_bits;
	}
	return result;
}

static BigInteger BigInteger::random_inclusive(const BigInteger &inclusive, random_func func)
{
	size_t n_bits = inclusive.bitlength();
	while (true)
	{
		BigInteger result = random_bits(n_bits, func);
		if (result <= inclusive)
			return result;
	}
}

static BigInteger BigInteger::random_exclusive(const BigInteger &exclusive, random_func func)
{
	size_t n_bits = exclusive.bitlength();
	while (true)
	{
		BigInteger result = random_bits(n_bits, func);
		if (result < exclusive)
			return result;
	}
}

static BigInteger BigInteger::random_second_exclusive(const BigInteger &inclusive_min_val, const BigInteger &exclusive_max_val, random_func func)
{
	return inclusive_min_val + random_exclusive(exclusive_max_val - inclusive_min_val, func);
}

static BigInteger BigInteger::random_both_inclusive(const BigInteger &inclusive_min_val, const BigInteger &inclusive_max_val, random_func func)
{
	return inclusive_min_val + random_inclusive(inclusive_max_val - inclusive_min_val, func);
}

BigInteger BigInteger::&set_bit(size_t i)
{
	size_t i_word = i / WORD_BITS;
	size_t i_bit = i % WORD_BITS;
	if (words.size() <= i_word)
		words.resize(i_word + 1);
	(*this)[i_word] |= ((word)1) << i_bit;
	return *this;
}

word BigInteger::get_bit(size_t i) const
{
	size_t i_word = i / WORD_BITS;
	size_t i_bit = i % WORD_BITS;
	if (i_word >= words.size())
		return 0;
	return ((*this)[i_word] >> i_bit) & 1;
}

void BigInteger::clr_bit(size_t i)
{
	size_t i_word = i / WORD_BITS;
	size_t i_bit = i % WORD_BITS;
	if (i_word >= words.size())
		return;
	word mask = 1;
	mask <<= i_bit;
	(*this)[i_word] &= ~mask;
}

// TODO : need to be more efficiently
char BigInteger::*to_chars() const
{
	std::vector<char> text;
	BigInteger tmp(*this);
	//get tens 
	size_t l = log10(WORD_MASK) * this->words.size();
	do
	{
		word remainder = 0;
		BigInteger dst(tmp.words.size(), 0);
		const word bitsShift = WORD_BITS / 2;
		for (size_t i = tmp.words.size(); i--;)
		{
			dst[i] = 0;
			//part 1
			remainder <<= bitsShift;
			remainder |= tmp[i] >> bitsShift;
			dst[i] <<= bitsShift;
			dst[i] |= remainder / 10;
			remainder %= 10;
			//part 2
			remainder <<= bitsShift;
			remainder |= tmp[i] & WORD_HALF_MASK;
			dst[i] <<= bitsShift;
			dst[i] |= remainder / 10;
			remainder %= 10;

			tmp = dst.truncate().set_neg(tmp.neg);
		}
		text.push_back('0' + char(remainder));
	} while (tmp.words.size() > 0);
	if (neg)
		text.push_back('-');

	std::reverse(text.begin(), text.end());

	text.push_back('\0');
	return text.data();
}

double BigInteger::to_double() const
{
	if (!words.size())
		return 0.0;
	double d = 0.0, base = std::pow(2.0, WORD_BITS);
	for (size_t i = words.size(); i--;)
		d = d * base + (*this)[i];
	return neg ? -d : d;
}

bool BigInteger::can_convert_to_int(intmax_t *result)
{
	if (words.size() > 1 || (*this)[0] >= INTMAX_MAX)
		return false;
	*result = (*this)[0];
	if (neg)
		*result = -(*result);
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
		{
			result = (result * base) % modulus;
		}
		base = (base * base) % modulus;
	}
	return result;
}

BigInteger BigInteger::sqrt() const
{
	BigInteger n = *this;
	int bit = bitlength();
	if (bit & 1)
		bit ^= 1;
	BigInteger result = 0;
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
BigInteger BigInteger::&operator=(const BigInteger &a)
{
	words = a.words;
	neg = a.neg;
	return *this;
}

BigInteger BigInteger::&operator++()
{
	word carry = 1;
	for (size_t i = 0; i < words.size() && carry; i++)
		carry = add_carry(&(*this)[i], carry);
	if (carry)
		words.push_back(carry);
	return truncate();
}

BigInteger BigInteger::&operator+=(const BigInteger &b)
{
	size_t i, na, nb;
	word carry = 0;
	if (this->neg == b.neg)
	{
		na = this->words.size(), nb = b.words.size();
		size_t n = max(na, nb);
		this->words.resize(n);
		for (i = 0; i < nb; i++)
		{
			carry = add_carry(&(*this)[i], carry);
			carry += add_carry(&(*this)[i], b[i]);
		}
		for (; i < n && carry; i++)
			carry = add_carry(&(*this)[i], carry);
		if (carry)
			this->words.push_back(carry);
	}
	if (cmp_abs(*this, b) >= 0) {
		na = this->words.size(), nb = b.words.size();
		word carry = 0;
		for (i = 0; i < nb; i++)
		{
			carry = sub_carry(&(*this)[i], carry);
			carry += sub_carry(&(*this)[i], b[i]);
		}
		for (; i < na && carry; i++)
			carry = sub_carry(&(*this)[i], carry);
		return truncate().set_neg(this->neg);
	}
	BigInteger B = *this;
	*this = b;
	na = this->words.size(), nb = B.words.size();
	word carry = 0;
	for (i = 0; i < nb; i++)
	{
		carry = sub_carry(&(*this)[i], carry);
		carry += sub_carry(&(*this)[i], B[i]);
	}
	for (; i < na && carry; i++)
		carry = sub_carry(&(*this)[i], carry);
	return truncate().set_neg(*this->neg);
}

BigInteger BigInteger::&operator-=(const BigInteger &b)
{
	size_t i, na, nb;
	if (this->neg != b.neg) {
		na = this->words.size(), nb = b.words.size();
		size_t n = max(na, nb);
		this->words.resize(n);
		word carry = 0;
		for (i = 0; i < nb; i++)
		{
			carry = add_carry(&(*this)[i], carry);
			carry += add_carry(&(*this)[i], b[i]);
		}
		for (; i < n && carry; i++)
			carry = add_carry(&(*this)[i], carry);
		if (carry)
			this->words.push_back(carry);
		return truncate().set_neg(this->neg);
	}
	if (cmp_abs(*this, b) >= 0) {
		na = this->words.size(), nb = b.words.size();
		word carry = 0;
		for (i = 0; i < nb; i++)
		{
			carry = sub_carry(&(*this)[i], carry);
			carry += sub_carry(&(*this)[i], b[i]);
		}
		for (; i < na && carry; i++)
			carry = sub_carry(&(*this)[i], carry);
		return truncate().set_neg(this->neg);
	}
	BigInteger B = *this;
	*this = b;
	na = this->words.size(), nb = B.words.size();
	word carry = 0;
	for (i = 0; i < nb; i++)
	{
		carry = sub_carry(&(*this)[i], carry);
		carry += sub_carry(&(*this)[i], B[i]);
	}
	for (; i < na && carry; i++)
		carry = sub_carry(&(*this)[i], carry);
	return truncate().set_neg(!*this->neg);
}

BigInteger BigInteger::&operator*=(const BigInteger &b) { return *this = mul(*this, b); }
BigInteger BigInteger::&operator/=(const BigInteger &b) { return *this = div(*this, b); }
BigInteger BigInteger::&operator%=(const BigInteger &b) { return *this = mod(*this, b); }

bool BigInteger::operator==(const BigInteger &b) const { return cmp(*this, b) == 0; }
bool BigInteger::operator!=(const BigInteger &b) const { return cmp(*this, b) != 0; }
bool BigInteger::operator<=(const BigInteger &b) const { return cmp(*this, b) <= 0; }
bool BigInteger::operator>=(const BigInteger &b) const { return cmp(*this, b) >= 0; }
bool BigInteger::operator<(const BigInteger &b) const { return cmp(*this, b) < 0; }
bool BigInteger::operator>(const BigInteger &b) const { return cmp(*this, b) > 0; }

BigInteger BigInteger::operator+(const BigInteger &b) const
{
	size_t i, na, nb;
	if (this->neg == b.neg)
	{
		BigInteger a(*this);
		na = a.words.size(), nb = b.words.size();
		size_t n = max(na, nb);
		a.words.resize(n);
		word carry = 0;
		for (i = 0; i < nb; i++)
		{
			carry = add_carry(&a[i], carry);
			carry += add_carry(&a[i], b[i]);
		}
		for (; i < n && carry; i++)
			carry = add_carry(&a[i], carry);
		if (carry)
			a.words.push_back(carry);
		return a.truncate().set_neg(a.neg);
	}
	if (cmp_abs(*this, b) >= 0) {
		BigInteger a(*this);
		na = a.words.size(), nb = b.words.size();
		word carry = 0;
		for (i = 0; i < nb; i++)
		{
			carry = sub_carry(&a[i], carry);
			carry += sub_carry(&a[i], b[i]);
		}
		for (; i < na && carry; i++)
			carry = sub_carry(&a[i], carry);
		return a.truncate().set_neg(a.neg);
	}
	BigInteger a(b);
	na = a.words.size(), nb = this->words.size();
	word carry = 0;
	for (i = 0; i < nb; i++)
	{
		carry = sub_carry(&a[i], carry);
		carry += sub_carry(&a[i], (*this)[i]);
	}
	for (; i < na && carry; i++)
		carry = sub_carry(&a[i], carry);
	return a.truncate().set_neg(a.neg);
}
BigInteger BigInteger::operator-(const BigInteger &b) const
{
	size_t i, na, nb;
	if (a.neg != b.neg) {
		BigInteger a(*this);
		i, na = a.words.size(), nb = b.words.size();
		size_t n = max(na, nb);
		a.words.resize(n);
		word carry = 0;
		for (i = 0; i < nb; i++)
		{
			carry = add_carry(&a[i], carry);
			carry += add_carry(&a[i], b[i]);
		}
		for (; i < n && carry; i++)
			carry = add_carry(&a[i], carry);
		if (carry)
			a.words.push_back(carry);
		return truncate().set_neg(a.neg);
	}
	
	if (cmp_abs(a, b) >= 0)
	{
		BigInteger a(*this);
		na = a.words.size(), nb = b.words.size();
		word carry = 0;
		for (i = 0; i < nb; i++)
		{
			carry = sub_carry(&a[i], carry);
			carry += sub_carry(&a[i], b[i]);
		}
		for (; i < na && carry; i++)
			carry = sub_carry(&a[i], carry);
		return a.truncate().set_neg(a.neg);
	}
	BigInteger a(b);
	na = a.words.size(), nb = this->words.size();
	word carry = 0;
	for (i = 0; i < nb; i++)
	{
		carry = sub_carry(&a[i], carry);
		carry += sub_carry(&a[i], (*this)[i]);
	}
	for (; i < na && carry; i++)
		carry = sub_carry(&a[i], carry);
	return a.truncate().set_neg(!a.neg);
}
BigInteger BigInteger::operator*(const BigInteger &b) const { return mul(*this, b); }
BigInteger BigInteger::operator/(const BigInteger &b) const { return div(*this, b); }
BigInteger BigInteger::operator%(const BigInteger &b) const { return mod(*this, b); }
BigInteger BigInteger::operator-() const { return BigInteger(*this).set_neg(!neg); }

BigInteger BigInteger::&operator>>=(size_t n_bits)
{
	if (!n_bits)
		return *this;
	size_t n_words = n_bits / WORD_BITS;
	if (n_words >= words.size())
	{
		words.resize(0);
		return *this;
	}
	n_bits %= WORD_BITS;
	if (n_bits)
	{
		word hi, lo = (*this)[n_words];
		for (size_t i = 0; i < words.size() - n_words - 1; i++)
		{
			hi = (*this)[i + n_words + 1];
			(*this)[i] = (hi << (WORD_BITS - n_bits)) | (lo >> n_bits);
			lo = hi;
		}
		(*this)[words.size() - n_words - 1] = lo >> n_bits;
	}
	else
	{
		for (size_t i = 0; i < words.size() - n_words; i++)
			(*this)[i] = (*this)[i + n_words];
	}
	words.resize(words.size() - n_words);
	return truncate();
}

BigInteger BigInteger::&operator<<=(size_t n_bits)
{
	if (!n_bits)
		return *this;
	size_t n_words = n_bits / WORD_BITS;
	n_bits %= WORD_BITS;
	size_t old_size = words.size();
	size_t n = old_size + n_words + (n_bits != 0);
	words.resize(n);
	if (n_bits)
	{
		word lo, hi = 0;
		for (size_t i = n - 1; i > n_words; i--)
		{
			lo = (*this)[i - n_words - 1];
			(*this)[i] = (hi << n_bits) | (lo >> (WORD_BITS - n_bits));
			hi = lo;
		}
		(*this)[n_words] = hi << n_bits;
	}
	else
	{
		for (size_t i = n; i-- > n_words;)
			(*this)[i] = (*this)[i - n_words];
	}
	for (size_t i = 0; i < n_words; i++)
		(*this)[i] = 0;
	return truncate();
}
BigInteger BigInteger::operator>>(size_t n_bits) const { return BigInteger(*this) >>= n_bits; }
BigInteger BigInteger::operator<<(size_t n_bits) const { return BigInteger(*this) <<= n_bits; }


ostream &operator<<(ostream &out, const BigInteger &num)
{
    out << num.to_chars();
    return out;
}

int main(int argc, char *argv[])
{
    BigInteger re(0);
    BigInteger max("99999999999999999999999999999");
    ul start, dur1, dur2, dur3;
    do
    {
        start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
        cout << re;
        dur3 = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count() - start;
        start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
        cout << '\r' << flush;
        dur1 = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count() - start;
        start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
        re += 19291982;
        dur2 = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count() - start;
    } while (dur2 < 500 && dur3 < 500);
    cout << '\n'
         << max << endl;
    cout << "flush : " << dur1 << endl;
    cout << "addition : " << dur2 << endl;
    cout << "print : " << dur3 << endl;
}