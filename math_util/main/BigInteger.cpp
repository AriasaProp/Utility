#include "BigInteger.hpp"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>

constexpr size_t WORD_BITS = sizeof (word) * CHAR_BIT;
constexpr size_t WORD_BITS_1 = WORD_BITS - 1;
constexpr size_t WORD_HALF_BITS = WORD_BITS / 2;
constexpr word WORD_HALF_MASK = ((word)-1) >> WORD_HALF_BITS;

typedef std::vector<word> wstack;

// private function for repeated use
//  +1 mean a is greater, -1 mean a is less, 0 mean equal
static int compare (const wstack &a, const wstack &b) {
  if (a == b) return 0;
  size_t as = a.size (), bs = b.size ();
  if (as != bs)
    return as > bs ? +1 : -1;
  while (as--)
    if (a[as] != b[as])
      return a[as] > b[as] ? +1 : -1;
  return 0;
}
static void add_a_word (wstack &a, word carry = 1) {
  for (wstack::iterator i = a.begin (), j = a.end (); (i < j) && carry; ++i)
    carry = carry > (*i += carry);
  if (carry) a.push_back (carry);
}
// a should be greater or equal than carry
static bool sub_a_word (wstack &a, word carry = 1) {
  for (wstack::iterator i = a.begin (), j = a.end (); (i < j) && carry; ++i)
    carry = *i < (*i -= carry);
  if (carry) {
    for (word &w : a)
      w = ~w;
    return true;
  }
  while (a.size () && !a.back ())
  	a.pop_back ();
  return false;
}
// params b shall be same memory as a
static void add_word (wstack &a, const wstack &b) {
	if (a.size () < b.size())
    a.resize (b.size(), 0);
  wstack::iterator i = a.begin();
  wstack::const_iterator jend = b.end(), j = b.begin();
  word carry = 0, c;
  while (j < jend) {
    c = *(j++); // because b can be same memory as a
    carry = carry > (*i += carry);
    carry += c > (*i += c);
    ++i;
  }
  j = a.cend();
  while ((i < j) && carry)
    carry = carry > (*(i++) += carry);
  if (carry)
    a.push_back (carry);
}
// params b shall be same memory as a but it always compare before operation
static bool sub_word (wstack &a, const wstack &b) {
	if (a.size () < b.size())
    a.resize (b.size(), 0);
  wstack::iterator i = a.begin(), iend = a.end();
  wstack::const_iterator jend = b.end(), j = b.begin();
  word carry = 0, c, d;
  while (j < jend) {
  	c = *(j++);
    carry = *i < (*i -= carry);
    carry += *i < (*i -= c);
    ++i;
  }
  while ((i < iend) && carry) {
  	carry = *i < (*i -= carry);
  	++i;
  }
  if (carry) {
    for (word &w : a)
      w = ~w;
    return true;
  }
  while (a.size () && !a.back ())
  	a.pop_back ();
  return false;
}
// karatsuba loop
/*
mul(a, b):
    max = sizeof a or b
  A <-	  a1  +  a2
  *   	  *       *
  B <-  	b1  +  b2
  |     	 |      |
  C  -   c1  -   c2 -> DD


    result
    @@@@
    c1
      c2
     DD
    _____+

*/
struct mul_in {
  size_t len;
  const wstack::iterator a, &a_end;
  const wstack::iterator b, &b_end;
};
static void mul_word (wstack::iterator r, const mul_in d) {
  if (d.len == 1) {
    if (d.a >= d.a_end || d.b >= d.b_end) return;
    const word Ar = *d.a;
    const word a_hi = Ar >> WORD_HALF_BITS;
    const word a_lo = Ar & WORD_HALF_MASK;
    const word &Br = *d.b;
    const word b_hi = Br >> WORD_HALF_BITS;
    const word b_lo = Br & WORD_HALF_MASK;
    *r = Ar * Br;

    word &r2 = *(r + 1);
    r2 = (a_lo * b_lo) >> WORD_HALF_BITS;
    r2 += a_hi * b_lo;
    r2 += a_lo * b_hi;
    r2 >>= WORD_HALF_BITS;
    r2 += a_hi * b_hi;
  } else {
    size_t half_len = d.len / 2;
    wstack re (d.len * 2, 0);
    mul_word (re.begin (), mul_in{half_len, d.a, d.a_end, d.b, d.b_end});
    mul_word (re.begin () + d.len, mul_in{half_len, d.a + half_len, d.a_end, d.b + half_len, d.b_end});
  }
}

// initialize BigInteger functions

// Constructors
BigInteger::BigInteger () {}
BigInteger::BigInteger (const BigInteger &a) : neg (a.neg), words (a.words) {}
BigInteger::BigInteger (const signed i) : neg (i < 0) {
  unsigned u = abs (i);
  while (u) {
    words.push_back (u);
    size_t i = WORD_BITS;
    while (i--)
      u >>= 1;
  }
}
// in constructor should begin with sign or number, add '\0' in last world to close number
BigInteger::BigInteger (const char *c) : neg (false) {
  // read sign
  if (*c == '-')
    neg = true, c++;
  // read digits
  word carry, tmp, a_hi, a_lo, tp;
  while (*c) {
    carry = *c - '0';
    if (carry > 9)
      throw ("BigInteger should initialize with number from 0 to 9.");
    for (word &cur : words) {
      tmp = cur * 10;
      carry = (tmp += carry) < carry;
      a_hi = cur >> WORD_HALF_BITS;
      a_lo = cur & WORD_HALF_MASK;
      tp = ((a_lo * 10) >> WORD_HALF_BITS) + a_hi * 10;
      carry += (tp >> WORD_HALF_BITS) + ((tp & WORD_HALF_MASK) >> WORD_HALF_BITS);
      cur = tmp;
    }
    if (carry)
      words.push_back (carry);
    c++;
  }
}
BigInteger::BigInteger (const wstack v, bool neg = false) : neg (neg), words (v) {}
// Destructors
BigInteger::~BigInteger () {
  words.clear ();
}
// operator casting
BigInteger::operator bool () const {
  return words.size () > 0;
}
BigInteger::operator int () const {
  if (words.size ()) {
    if (neg)
      return -(int (words[0]));
    else
      return int (words[0]);
  }
  return 0;
}

char *BigInteger::to_chars () const {
  if (words.empty ())
    return new char[1]{'0'};

  size_t texN = neg ? 1 : 0;
  word rmr, current;
  // grt decimal count
  wstack A = words;
  do {
    rmr = 0;
    for (wstack::reverse_iterator cur = A.rbegin (); cur != A.rend (); ++cur) {
      current = *cur;
      rmr <<= WORD_HALF_BITS;
      rmr |= current >> WORD_HALF_BITS;
      *cur = rmr / 10;
      rmr %= 10;
      rmr <<= WORD_HALF_BITS;
      rmr |= current & WORD_HALF_MASK;
      *cur <<= WORD_HALF_BITS;
      *cur |= rmr / 10;
      rmr %= 10;
    }
    ++texN;
    if (!A.back ())
      A.pop_back ();
  } while (!A.empty ());

  A = words;
  char *text = new char[texN];
  if (neg) *text = '-';
  char *tcr = text + texN;
  while (!A.empty ()) {
    rmr = 0;
    for (wstack::reverse_iterator cur = A.rbegin (); cur != A.rend (); ++cur) {
      current = *cur;
      rmr <<= WORD_HALF_BITS;
      rmr |= current >> WORD_HALF_BITS;
      *cur = rmr / 10;
      rmr %= 10;
      rmr <<= WORD_HALF_BITS;
      rmr |= current & WORD_HALF_MASK;
      *cur <<= WORD_HALF_BITS;
      *cur |= rmr / 10;
      rmr %= 10;
    }
    *(--tcr) = '0' + char (rmr);
    if (!A.back ())
      A.pop_back ();
  }
  return text;
}

// math operational
BigInteger BigInteger::sqrt () const {
  BigInteger result;
  size_t n = words.size ();
  if (n) {
    n = (n - 1) * WORD_BITS;
    word carry = words.back ();
    while (carry)
      n++, carry >>= 1;
    if (n & 1)
      n++;
    wstack remaining (1, 0), &res = result.words;
    res.push_back (0);
    size_t i, j;
    const size_t l_shift = WORD_BITS - 2;
    const word setLast = ~word (1);
    while (n) {
      n -= 2;
      // remaining shift left 2
      carry = remaining.back () >> l_shift;
      for (i = remaining.size () - 1; i; i--)
        remaining[i] = (remaining[i] << 2) | (remaining[i - 1] >> l_shift);
      remaining[i] <<= 2;
      remaining[i] |= (words[n / WORD_BITS] >> (n % WORD_BITS)) & word (3);
      if (carry)
        remaining.push_back (carry);
      // result shift 1 left with test
      carry = res.back () >> WORD_BITS_1;
      for (i = res.size () - 1; i; i--)
        res[i] = (res[i] << 1) | (res[i - 1] >> WORD_BITS_1);
      res[i] = (res[i] << 1) | 1;
      if (carry)
        res.push_back (carry);
      // compare result test with remaining
      if (compare (remaining, res) >= 0) {
        carry = 0;
        for (i = 0, j = res.size (); i < j; i++) {
          carry = remaining[i] < (remaining[i] -= carry);
          carry += remaining[i] < (remaining[i] -= res[i]);
        }
        for (j = remaining.size (); (i < j) && carry; i++)
          carry = remaining[i] < (remaining[i] -= carry);
        res[0] |= 2;
      }
      res[0] &= setLast;
    }
    // shift 1 right to get pure result
    for (i = 0, j = res.size () - 1; i < j; i++)
      res[i] = (res[i + 1] << WORD_BITS_1) | (res[i] >> 1);
    res.back () >>= 1;
    while (res.size () && !res.back ())
      res.pop_back ();
  }
  return result;
}
BigInteger BigInteger::pow (size_t exponent) const {
  BigInteger result = *this;
  size_t na = result.words.size ();
  if (na) {
    wstack p = result.words, &R = result.words;
    R.clear ();
    R.push_back (1);
    word A[na * exponent], B[na * exponent];
    result.neg = result.neg & (exponent & 1);
    word a_hi, a_lo, b_hi, b_lo, carry, carry0;
    size_t ia, ib, i = 0, j, nb;
    while (exponent) {
      nb = p.size ();
      if (exponent & 1) {
        na = R.size ();
        std::copy (R.begin (), R.end (), A);
        std::copy (p.begin (), p.end (), B);
        R.clear ();
        R.resize (na + nb, 0);
        for (ia = 0; ia < na; ia++) {
          const word &Ar = A[ia];
          a_hi = Ar >> WORD_HALF_BITS;
          a_lo = Ar & WORD_HALF_MASK;
          carry = 0;
          for (ib = 0; ib < nb; ib++) {
            i = ia + ib;
            word &r = R[i];
            carry = (r += carry) < carry;
            const word &Br = B[ib];
            b_hi = Br >> WORD_HALF_BITS;
            b_lo = Br & WORD_HALF_MASK;
            // part 1
            carry0 = Ar * Br;
            carry += (r += carry0) < carry0;
            // part 2
            carry0 = a_lo * b_lo;
            carry0 >>= WORD_HALF_BITS;
            carry0 += a_hi * b_lo;
            // TODO:  this may overflow, if you find error in multiplication check this
            carry += (carry0 >> WORD_HALF_BITS) + ((a_lo * b_hi + (carry0 & WORD_HALF_MASK)) >> WORD_HALF_BITS) + a_hi * b_hi;
          }
          j = R.size ();
          while (((++i) < j) && carry)
            carry = (R[i] += carry) < carry;
          if (carry)
            R.push_back (carry);
        }
        while (R.size () && !R.back ())
          R.pop_back ();
      }
      exponent >>= 1;
      std::copy (p.begin (), p.end (), A);
      p.clear ();
      p.resize (nb * 2, 0);
      for (ia = 0; ia < nb; ia++) {
        const word &Ar = A[ia];
        a_hi = Ar >> WORD_HALF_BITS;
        a_lo = Ar & WORD_HALF_MASK;
        carry = 0;
        for (ib = 0; ib < nb; ib++) {
          i = ia + ib;
          word &r = p[i];
          carry = (r += carry) < carry;
          const word &Br = A[ib];
          b_hi = Br >> WORD_HALF_BITS;
          b_lo = Br & WORD_HALF_MASK;
          // part 1
          carry0 = Ar * Br;
          carry += (r += carry0) < carry0;
          // part 2
          carry0 = a_lo * b_lo;
          carry0 >>= WORD_HALF_BITS;
          carry0 += a_hi * b_lo;
          // TODO:  this may overflow, if you find error in multiplication check this
          carry += (carry0 >> WORD_HALF_BITS) + ((a_lo * b_hi + (carry0 & WORD_HALF_MASK)) >> WORD_HALF_BITS) + a_hi * b_hi;
        }
        j = p.size ();
        while (((++i) < j) && carry)
          carry = (p[i] += carry) < carry;
        if (carry)
          p.push_back (carry);
      }
      while (p.size () && !p.back ())
        p.pop_back ();
    }
  } else if (!exponent)
    return BigInteger (1);
  return result;
}
// re-initialize
BigInteger &BigInteger::operator= (const signed a) {
  if (words.size ())
    words.clear ();
  neg = (a < 0);
  unsigned u = abs (a);
  size_t i;
  while (u) {
    words.push_back (u);
    i = WORD_BITS;
    while (i--)
      u >>= 1;
  }
  return *this;
}
BigInteger &BigInteger::operator= (const BigInteger a) {
  words = a.words;
  neg = a.neg;
  return *this;
}
// safe operator
BigInteger &BigInteger::operator-- () {
  if (neg)
    add_a_word (words);
  else
    neg ^= sub_a_word (words);
  return *this;
}
BigInteger &BigInteger::operator++ () {
  if (neg)
    neg ^= sub_a_word (words);
  else
    add_a_word (words);
  return *this;
}
BigInteger &BigInteger::operator+= (const signed b) {
  const word B = word (abs (b));
  if (neg == (b < 0))
    add_a_word (words, B);
  else
    neg ^= sub_a_word (words, B);
  return *this;
}
BigInteger &BigInteger::operator+= (const BigInteger &b) {
  if (neg == b.neg)
    add_word (words, b.words);
  else
    neg ^= sub_word (words, b.words);
  return *this;
}
BigInteger &BigInteger::operator-= (const signed b) {
  const word B = word (abs (b));
  if (neg != (b < 0))
    add_a_word (words);
  else
    neg ^= sub_a_word (words, B);
  return *this;
}
BigInteger &BigInteger::operator-= (const BigInteger &b) {
  if (neg != b.neg)
    add_word (words, b.words);
  else
    neg ^= sub_word (words, b.words);
  return *this;
}
BigInteger &BigInteger::operator*= (const signed b) {
  BigInteger r = *this * b;
  words = std::move (r.words);
  neg = r.neg;
  return *this;
}
BigInteger &BigInteger::operator*= (const BigInteger b) {
  BigInteger r = *this * b;
  words = std::move (r.words);
  neg = r.neg;
  return *this;
}
BigInteger &BigInteger::operator/= (const signed b) {
  if (!b) throw ("Undefined number cause / 0 !");
  wstack rem = words, div{word (abs (b))};
  words.clear ();
  if (compare (rem, div) >= 0) {
    // shifting count
    size_t i = div.size ();
    size_t j = (rem.size () - i) * WORD_BITS;
    word carry0 = rem.back ();
    while (carry0)
      j++, carry0 >>= 1;
    carry0 = div.back ();
    while (carry0)
      j--, carry0 >>= 1;
    size_t n = j % WORD_BITS;
    if (n) {
      const size_t l_shift = WORD_BITS - n;
      carry0 = div.back () >> l_shift;
      while (--i)
        div[i] = (div[i] << n) | (div[i - 1] >> l_shift);
      div[i] <<= n;
      if (carry0)
        div.push_back (carry0);
    }
    n = j / WORD_BITS;
    if (n)
      div.insert (div.begin (), n, 0);
    words.resize (n + 1, 0);
    do {
      if (compare (rem, div) >= 0) {
        words[j / WORD_BITS] |= word (1) << (j % WORD_BITS);
        sub_word (rem, div);
        if (!rem.size ()) break;
      }
      // reverse shift one by one
      for (i = 0, n = div.size () - 1; i < n; i++)
        div[i] = (div[i] >> 1) | (div[i + 1] << WORD_BITS_1);
      div[i] >>= 1;
      while (!div.back ())
        div.pop_back ();
    } while (j--);
    while (words.size () && !words.back ())
      words.pop_back ();
    neg ^= (b < 0);
  } else
    neg = false;
  // return result
  return *this;
}
BigInteger &BigInteger::operator/= (const BigInteger b) {
  if (!b.words.size ())
    throw ("Undefined number cause / 0 !");
  wstack rem = words, div = b.words;
  words.clear ();
  if (compare (rem, div) >= 0) {
    // shifting count
    size_t i = div.size ();
    size_t j = (rem.size () - i) * WORD_BITS;
    word carry0 = rem.back ();
    while (carry0)
      j++, carry0 >>= 1;
    carry0 = div.back ();
    while (carry0)
      j--, carry0 >>= 1;
    size_t n = j % WORD_BITS;
    if (n) {
      const size_t l_shift = WORD_BITS - n;
      carry0 = div.back () >> l_shift;
      while (--i)
        div[i] = (div[i] << n) | (div[i - 1] >> l_shift);
      div[i] <<= n;
      if (carry0)
        div.push_back (carry0);
    }
    n = j / WORD_BITS;
    if (n)
      div.insert (div.begin (), n, 0);
    words.resize (n + 1, 0);
    do {
      if (compare (rem, div) >= 0) {
        words[j / WORD_BITS] |= word (1) << (j % WORD_BITS);
        sub_word (rem, div);
        if (!rem.size ())
          break;
      }
      // reverse shift one by one
      for (i = 0, n = div.size () - 1; i < n; i++)
        div[i] = (div[i] >> 1) | (div[i + 1] << WORD_BITS_1);
      div[i] >>= 1;
      if (!div.back ())
        div.pop_back ();
    } while (j--);
    while (words.size () && !words.back ())
      words.pop_back ();
    neg ^= b.neg;
  } else {
    neg = false;
  }
  return *this;
}
BigInteger &BigInteger::operator%= (const signed b) {
  if (b != 0) {
    wstack &rem = words, div{word (abs (b))};
    int cmp = compare (rem, div);
    if (cmp >= 0) {
      // shifting count
      size_t i = div.size ();
      size_t j = (rem.size () - i) * WORD_BITS;
      word carry0 = rem.back ();
      while (carry0)
        j++, carry0 >>= 1;
      carry0 = div.back ();
      while (carry0)
        j--, carry0 >>= 1;
      size_t n = j % WORD_BITS;
      if (n) {
        const size_t l_shift = WORD_BITS - n;
        carry0 = div.back () >> l_shift;
        while (--i)
          div[i] = (div[i] << n) | (div[i - 1] >> l_shift);
        div[i] <<= n;
        if (carry0)
          div.push_back (carry0);
      }
      n = j / WORD_BITS;
      if (n)
        div.insert (div.begin (), n, 0);
      do {
        cmp = compare (rem, div);
        if (cmp > 0)
          sub_word (rem, div);
        else if (cmp == 0) {
          neg = false;
          rem.clear ();
          break;
        }
        // reverse shift one by one
        for (i = 0, n = div.size () - 1; i < n; i++)
          div[i] = (div[i] >> 1) | (div[i + 1] << WORD_BITS_1);
        div[i] >>= 1;
        if (!div.back ())
          div.pop_back ();
      } while (j--);
    }
  }
  return *this;
}
BigInteger &BigInteger::operator%= (const BigInteger b) {
  if (b.words.size ()) {
    wstack &rem = words, div = b.words;
    int cmp = compare (rem, div);
    if (cmp >= 0) {
      // shifting count
      size_t i = div.size ();
      size_t j = (rem.size () - i) * WORD_BITS;
      word carry0 = rem.back ();
      while (carry0)
        j++, carry0 >>= 1;
      carry0 = div.back ();
      while (carry0)
        j--, carry0 >>= 1;
      size_t n = j % WORD_BITS;
      if (n) {
        const size_t l_shift = WORD_BITS - n;
        carry0 = div.back () >> l_shift;
        while (--i)
          div[i] = (div[i] << n) | (div[i - 1] >> l_shift);
        div[i] <<= n;
        if (carry0)
          div.push_back (carry0);
      }
      n = j / WORD_BITS;
      if (n)
        div.insert (div.begin (), n, 0);
      do {
        cmp = compare (rem, div);
        if (cmp > 0)
          sub_word (rem, div);
        else if (cmp == 0) {
          neg = false;
          rem.clear ();
          break;
        }
        // reverse shift one by one
        for (i = 0, n = div.size () - 1; i < n; i++)
          div[i] = (div[i] >> 1) | (div[i + 1] << WORD_BITS_1);
        div[i] >>= 1;
        if (!div.back ())
          div.pop_back ();
      } while (j--);
    }
  }
  return *this;
}
// safe bitwise operand
BigInteger &operator>>= (BigInteger &a, size_t n_bits) {
  if (n_bits && a.words.size ()) {
    size_t j = n_bits / WORD_BITS;
    if (j < a.words.size ()) {
      wstack::iterator carried = a.words.begin ();
      a.words.erase (carried, carried + j);
      n_bits %= WORD_BITS;
      if (n_bits) {
        wstack::iterator endCarried = a.words.end () - 1;
        const size_t r_shift = WORD_BITS - n_bits;
        *carried >>= n_bits;
        while (carried != endCarried) {
          *carried |= *(carried + 1) << r_shift;
          carried++;
          *carried >>= n_bits;
        }
        if (*endCarried == 0)
          a.words.pop_back ();
      }
    } else {
      a.neg = false;
      a.words.clear ();
    }
  }
  return a;
}
BigInteger &operator<<= (BigInteger &a, size_t bits) {
  if (bits) {
    size_t n = bits % WORD_BITS;
    if (n) {
      const size_t l_shift = WORD_BITS - n;
      wstack::reverse_iterator carried = a.words.rbegin ();
      wstack::reverse_iterator endCarried = a.words.rend () - 1;
      word lo = *carried >> l_shift;
      while (carried != endCarried) {
        *carried <<= n;
        *carried |= *(carried + 1) >> l_shift;
        carried++;
      }
      *carried <<= n;
      if (lo)
        a.words.push_back (lo);
    }
    n = bits / WORD_BITS;
    if (n)
      a.words.insert (a.words.begin (), n, 0);
  }
  return a;
}
BigInteger &operator&= (BigInteger &a, const BigInteger b) {
  a.neg &= b.neg;
  size_t lu = (a.words.size () < b.words.size ()) ? a.words.size () : b.words.size ();
  a.words.resize (lu);
  while (lu--)
    a.words[lu] &= b.words[lu];
  while (a.words.size () && !a.words.back ())
    a.words.pop_back ();
  return a;
}
BigInteger &operator|= (BigInteger &a, const BigInteger b) {
  a.neg &= b.neg;
  size_t la = a.words.size (), lb = b.words.size ();
  size_t lu = (la > lb) ? la : lb;
  a.words.resize (lu);
  while (lb--)
    a.words[lb] |= b.words[lb];
  return a;
}
// compare operator
bool operator== (const BigInteger &a, const signed b) {
  if (a.neg != (b < 0))
    return false;
  if (a.words.size () != 1)
    return false;
  const signed B = std::abs (b);
  if (a.words[0] != B)
    return false;
  return true;
}
bool operator!= (const BigInteger &a, const signed b) {
  if (a.neg != (b < 0))
    return true;
  if (a.words.size () != 1)
    return true;
  const signed B = std::abs (b);
  if (a.words[0] != B)
    return true;
  return false;
}
bool operator<= (const BigInteger &a, const signed b) {
  if (a.neg != (b < 0))
    return a.neg;
  if (a.words.size () != 1)
    return (a.words.size () < 1) ^ a.neg;
  const signed B = std::abs (b);
  if (a.words[0] != B)
    return (a.words[0] < B) ^ a.neg;
  return true;
}
bool operator>= (const BigInteger &a, const signed b) {
  if (a.neg != (b < 0))
    return !a.neg;
  if (a.words.size () != 1)
    return (a.words.size () > 1) ^ a.neg;
  const signed B = std::abs (b);
  if (a.words[0] != B)
    return (a.words[0] > B) ^ a.neg;
  return true;
}
bool operator<(const BigInteger &a, const signed b) {
  if (a.neg != (b < 0))
    return a.neg;
  if (a.words.size () != 1)
    return (a.words.size () < 1) ^ a.neg;
  const signed B = std::abs (b);
  if (a.words[0] != B)
    return (a.words[0] < B) ^ a.neg;
  return false;
}
bool operator> (const BigInteger &a, const signed b) {
  if (a.neg != (b < 0))
    return !a.neg;
  if (a.words.size () != 1)
    return (a.words.size () > 1) ^ a.neg;
  const signed B = std::abs (b);
  if (a.words[0] != B)
    return (a.words[0] > B) ^ a.neg;
  return false;
}
bool operator== (const BigInteger &a, const BigInteger &b) {
  if (a.neg != b.neg)
    return false;
  size_t i = a.words.size (), j = b.words.size ();
  if (i != j)
    return false;
  while (i--)
    if (a.words[i] != b.words[i])
      return false;
  return true;
}
bool operator!= (const BigInteger &a, const BigInteger &b) {
  if (a.neg != b.neg)
    return true;
  size_t i = a.words.size (), j = b.words.size ();
  if (i != j)
    return true;
  while (i--)
    if (a.words[i] != b.words[i])
      return true;
  return false;
}
bool operator<= (const BigInteger &a, const BigInteger &b) {
  if (a.neg != b.neg)
    return a.neg;
  size_t i = a.words.size (), j = b.words.size ();
  if (i != j)
    return (i < j) ^ a.neg;
  while (i--)
    if (a.words[i] != b.words[i])
      return (a.words[i] < b.words[i]) ^ a.neg;
  return true;
}
bool operator>= (const BigInteger &a, const BigInteger &b) {
  if (a.neg != b.neg)
    return b.neg;
  size_t i = a.words.size (), j = b.words.size ();
  if (i != j)
    return (i > j) ^ a.neg;
  while (i--)
    if (a.words[i] != b.words[i])
      return (a.words[i] > b.words[i]) ^ a.neg;
  return true;
}
bool operator<(const BigInteger &a, const BigInteger &b) {
  if (a.neg != b.neg)
    return a.neg;
  size_t i = a.words.size (), j = b.words.size ();
  if (i != j)
    return (i < j) ^ a.neg;
  while (i--)
    if (a.words[i] != b.words[i])
      return (a.words[i] < b.words[i]) ^ a.neg;
  return false;
}
bool operator> (const BigInteger &a, const BigInteger &b) {
  if (a.neg != b.neg)
    return b.neg;
  size_t i = a.words.size (), j = b.words.size ();
  if (i != j)
    return (i > j) ^ a.neg;
  while (i--)
    if (a.words[i] != b.words[i])
      return (a.words[i] > b.words[i]) ^ a.neg;
  return false;
}
// new object generate, operator
BigInteger BigInteger::operator+ (const signed b) const {
  return BigInteger (*this) += b;
}
BigInteger BigInteger::operator+ (const BigInteger b) const {
  return BigInteger (*this) += b;
}
BigInteger BigInteger::operator- (const signed b) const {
  return BigInteger (*this) -= b;
}
BigInteger BigInteger::operator- (const BigInteger b) const {
  return BigInteger (*this) -= b;
}
BigInteger BigInteger::operator* (const signed b) const {
  BigInteger result;
  if (b && words.size ()) {
    result.neg = neg ^ (b < 0);
    wstack &r = result.words;
    r.resize (words.size ());
    r.reserve (words.size () + 1);
    const word B = word (abs (b));
    const word b_hi = B >> WORD_HALF_BITS;
    const word b_lo = B & WORD_HALF_MASK;
    word a_hi, a_lo, carry = 0, carry0;
    for (size_t i = 0; i < words.size (); ++i) {
      const word &wA = words[i];
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
      r.push_back (carry);
  }
  return result;
}
BigInteger BigInteger::operator* (const BigInteger b) const {
  BigInteger result;
  const wstack &B = b.words;
  const size_t na = words.size (), nb = B.size ();
  if (na && nb) {
    wstack &a = result.words;
    a.resize (na + nb, 0);
    a.reserve (na + nb + 1);
    result.neg = neg ^ b.neg;
    word a_hi, b_hi, a_lo, b_lo, carry, carry0;
    size_t ia, ib, i, j;
    for (ia = 0; ia < na; ia++) {
      const word &Ar = words[ia];
      a_hi = Ar >> WORD_HALF_BITS;
      a_lo = Ar & WORD_HALF_MASK;
      carry = 0;
      for (ib = 0; ib < nb; ib++) {
        i = ia + ib;
        word &r = a[i];
        carry = (r += carry) < carry;
        const word &Br = B[ib];
        b_hi = Br >> WORD_HALF_BITS;
        b_lo = Br & WORD_HALF_MASK;
        // part 1
        carry0 = Ar * Br;
        carry += (r += carry0) < carry0;
        // part 2
        carry0 = (a_lo * b_lo) >> WORD_HALF_BITS;
        carry0 += a_hi * b_lo;
        // TODO:  this may overflow, if you find error in multiplication check this
        carry += carry0 >> WORD_HALF_BITS;
        carry += (a_lo * b_hi + (carry0 & WORD_HALF_MASK)) >> WORD_HALF_BITS;
        carry += a_hi * b_hi;
      }
      j = a.size ();
      while (((++i) < j) && carry)
        carry = (a[i] += carry) < carry;
      if (carry)
        a.push_back (carry);
    }
    while (a.size () && !a.back ())
      a.pop_back ();
  }
  return result;
}
BigInteger BigInteger::operator/ (const signed b) const {
  return BigInteger (*this) /= b;
}
BigInteger BigInteger::operator/ (const BigInteger b) const {
  return BigInteger (*this) /= b;
}
BigInteger BigInteger::operator% (const signed b) const {
  return BigInteger (*this) %= b;
}
BigInteger BigInteger::operator% (const BigInteger b) const {
  return BigInteger (*this) %= b;
}
BigInteger BigInteger::operator- () const {
  return BigInteger (words, !neg);
}
// new object generate, bitwise operand
BigInteger operator>> (const BigInteger &A, size_t n_bits) {
  BigInteger a (A);
  if (n_bits && a.words.size ()) {
    size_t j = n_bits / WORD_BITS;
    if (j < a.words.size ()) {
      wstack::iterator carried = a.words.begin ();
      a.words.erase (carried, carried + j);
      n_bits %= WORD_BITS;
      if (n_bits) {
        wstack::iterator endCarried = a.words.end () - 1;
        const size_t r_shift = WORD_BITS - n_bits;
        *carried >>= n_bits;
        while (carried != endCarried) {
          *carried |= *(carried + 1) << r_shift;
          carried++;
          *carried >>= n_bits;
        }
        if (*endCarried == 0)
          a.words.pop_back ();
      }
    } else {
      a.neg = false;
      a.words.clear ();
    }
  }
  return a;
}
BigInteger operator<< (const BigInteger &A, size_t bits) {
  BigInteger a (A);
  size_t n = bits % WORD_BITS;
  if (n) {
    const size_t l_shift = WORD_BITS - n;
    wstack::reverse_iterator carried = a.words.rbegin ();
    wstack::reverse_iterator endCarried = a.words.rend () - 1;
    word lo = *carried >> l_shift;
    while (carried != endCarried) {
      *carried <<= n;
      *carried |= *(carried + 1) >> l_shift;
      carried++;
    }
    *carried <<= n;
    if (lo)
      a.words.push_back (lo);
  }
  n = bits / WORD_BITS;
  a.words.insert (a.words.begin (), n, 0);
  return a;
}
BigInteger operator& (const BigInteger a, const BigInteger b) {
  size_t la = a.words.size (), lb = b.words.size ();
  size_t lu = (la < lb) ? la : lb;
  wstack r = a.words;
  r.resize (lu);
  while (lu--) r[lu] &= b.words[lu];
  while (r.size () && !r.back ())
    r.pop_back ();
  return BigInteger (r, a.neg & b.neg);
}
BigInteger operator| (const BigInteger a, const BigInteger b) {
  size_t la = a.words.size (), lb = b.words.size ();
  size_t lu = (la > lb) ? la : lb;
  wstack r = a.words;
  r.resize (lu);
  while (lb--) r[lb] |= b.words[lb];
  return BigInteger (r, a.neg & b.neg);
}

std::ostream &operator<< (std::ostream &out, const BigInteger num) {
  if (num.words.empty ()) {
    out << "0";
  } else {
    if (num.neg)
      out << "-";
    size_t texN = 0;
    word rmr, current;
    // grt decimal count
    wstack A = num.words;
    do {
      rmr = 0;
      for (wstack::reverse_iterator cur = A.rbegin (); cur != A.rend (); ++cur) {
        current = *cur;
        rmr <<= WORD_HALF_BITS;
        rmr |= current >> WORD_HALF_BITS;
        *cur = rmr / 10;
        rmr %= 10;
        rmr <<= WORD_HALF_BITS;
        rmr |= current & WORD_HALF_MASK;
        *cur <<= WORD_HALF_BITS;
        *cur |= rmr / 10;
        rmr %= 10;
      }
      ++texN;
      if (!A.back ())
        A.pop_back ();
    } while (!A.empty ());

    A = num.words;
    char *text = new char[texN];
    char *tcr = text + texN;
    do {
      rmr = 0;
      for (wstack::reverse_iterator cur = A.rbegin (); cur != A.rend (); ++cur) {
        current = *cur;
        rmr <<= WORD_HALF_BITS;
        rmr |= current >> WORD_HALF_BITS;
        *cur = rmr / 10;
        rmr %= 10;
        rmr <<= WORD_HALF_BITS;
        rmr |= current & WORD_HALF_MASK;
        *cur <<= WORD_HALF_BITS;
        *cur |= rmr / 10;
        rmr %= 10;
      }
      *(--tcr) = '0' + char (rmr);
      if (!A.back ())
        A.pop_back ();
    } while (!A.empty ());
    out << text;
    delete[] text;
  }
  return out;
}
