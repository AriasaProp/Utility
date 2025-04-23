#include "math/BigInteger.hpp"
#include "common.hpp"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>

constexpr size_t WORD_BITS = sizeof (word) * CHAR_BIT;
constexpr size_t WORD_BITS_1 = WORD_BITS - 1;
constexpr size_t WORD_HALF_BITS = WORD_BITS / 2;
constexpr word WORD_HALF_MASK = ((word)-1) >> WORD_HALF_BITS;

using wstack = std::deque<word>;
using wit = std::deque<word>::iterator;
using writ = std::deque<word>::reverse_iterator;
using cwit = std::deque<word>::const_iterator;
using cwrit = std::deque<word>::const_reverse_iterator;

/** private function **/
//  +1 mean a is greater, -1 mean a is less, 0 mean equal
static int  compare_word      (const wstack &a, const wstack &b) {
  if (a == b) return 0;
  size_t as = a.size (), bs = b.size ();
  int r = (as > bs) - (as < bs);
  cwrit i = a.rbegin(), bi = b.rbegin(), j = a.rend();
  while (!r && (i < j))
    r = (*i > *bi) - (*i < *bi), ++i, ++bi;
  return r;
}
static void add_a_word        (wstack &a, word carry = 1) {
  for (wit i = a.begin (), j = a.end (); (i < j) && carry; ++i)
    carry = carry > (*i += carry);
  if (carry) a.push_back (carry);
}
static bool sub_a_word        (wstack &a, word carry = 1) {
  for (wit i = a.begin (), j = a.end (); (i < j) && carry; ++i)
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
static void add_word          (wstack &a, const wstack &b) {
  if (a.size () < b.size ())
    a.resize (b.size (), 0);
  wit i = a.begin ();
  wstack::const_iterator jend = b.end (), j = b.begin ();
  word carry = 0, c;
  while (j < jend) {
    c = *(j++); // because b can be same memory as a
    carry = carry > (*i += carry);
    carry += c > (*i += c);
    ++i;
  }
  j = a.cend ();
  while ((i < j) && carry)
    carry = carry > (*(i++) += carry);
  if (carry)
    a.push_back (carry);
}
static bool sub_word          (wstack &a, const wstack &b) {
  if (a.size () < b.size ())
    a.resize (b.size (), 0);
  wit i = a.begin ();
  wstack::const_iterator jend = b.end (), j = b.begin ();
  word carry = 0, c;
  while (j < jend) {
    c = *(j++);
    carry = *i < (*i -= carry);
    carry += *i < (*i -= c);
    ++i;
  }
  j = a.cend ();
  while ((i < j) && carry) {
    carry = *i < (*i -= carry);
    ++i;
  }
  if (carry) {
    for (word &w : a)
      carry &= !(w = ~w + carry);
    return true;
  }
  while (a.size () && !a.back ())
    a.pop_back ();
  return false;
}
static void shift_left_word   (wstack &a, size_t i) {
  size_t n = i % WORD_BITS;
  if (n) {
    const size_t r_shift = WORD_BITS - n;
    wit ai = a.begin();
    cwit aj = a.cend();
    word b = 0, b0 = 0;
    while (ai < aj) {
      b = *ai >> r_shift;
      *ai = (*ai << n) | b0;
      b0 = b;
      ++ai;
    }
    if (b0) a.push_back(b0);
  }
  a.insert (a.begin (), i / WORD_BITS, 0);
}
static void shift_right_word  (wstack &a, size_t i) {
  a.erase (a.begin (), a.begin () + i / WORD_BITS);
  size_t n = i % WORD_BITS;
  if (n && a.size()) {
    const size_t l_shift = WORD_BITS - n;
    writ ai = a.rbegin();
    writ aj = a.rend() - 1;
    word b = 0, b0 = 0;
    while (ai < aj) {
      b = *ai << l_shift;
      *ai = (*ai >> n) | b0;
      b0 = b;
      ++ai;
    }
    *ai = (*ai >> n) | b0;
  }
}
static wstack mul_word        (const wstack &a, const wstack &b) {
  wstack c;
  const size_t na = a.size(), nb = b.size();
  if (na && nb) {
    c.resize(na + nb, 0);
    word a_hi, b_hi, a_lo, b_lo, c0, c1;
    size_t ia, ib, ic, nc, i;
    for (ia = 0; ia < na; ++ia) {
      a_hi = a[ia] >> WORD_HALF_BITS;
      a_lo = a[ia] & WORD_HALF_MASK;
      for (ib = 0; ib < nb; ++ib) {
        b_hi = b[ib] >> WORD_HALF_BITS;
        b_lo = b[ib] & WORD_HALF_MASK;
        c0 = a[ia] * b[ib];
        i = ia + ib;
        for (ic = i, nc = c.size(); c0 && (ic < nc); ++ic)
          c0 = (c[ic] += c0) < c0;
        if (c0) c.push_back(c0);
        c0 = ((a_lo * b_lo) >> WORD_HALF_BITS) + (a_hi * b_lo);
        c1 = a_lo * b_hi;
        c1 = (c0 += c1) < c1;
        c0 = (c0 >> WORD_HALF_BITS) + (c1 << WORD_HALF_BITS);
        ++i;
        for (ic = i, nc = c.size(); c0 && (ic < nc); ++ic)
          c0 = (c[ic] += c0) < c0;
        if (c0) c.push_back(c0);
        c0 = a_hi * b_hi;
        for (ic = i, nc = c.size(); c0 && (ic < nc); ++ic)
          c0 = (c[ic] += c0) < c0;
        if (c0) c.push_back(c0);
      }
    }
    while (!c.back() && c.size())
      c.pop_back();
  }
  return c;
}
static wstack div_word        (wstack &a, wstack b) {
  wstack c;
  size_t na = a.size(), nb = b.size();
  if (nb && na) {
    size_t s = WORD_BITS * (na - nb);
    word carry;
    for (carry = a.back(); carry >>= 1; ++s) ;
    for (carry = b.back(); carry >>= 1; --s) ;
    shift_left_word(b, s);
    c.push_back(0);
    do {
      shift_left_word(c, 1);
      if (compare_word (a, b) >= 0) {
        sub_word(a, b);
        c.front() |= 1;
      }
      shift_right_word(b, 1);
    } while (s--);
    while (!c.back() && c.size())
      c.pop_back();
  }
  return c;
}

/**********************************
 *
 * Initialize BigInteger functions
 *
 **********************************/

/** Constructors **/
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
/** Destructors **/
BigInteger::~BigInteger () {
  words.clear ();
}
/** operator casting **/
BigInteger::operator bool () const {
  return !words.empty ();
}
BigInteger::operator int () const {
  if (!words.empty ())
    return (neg) ? -int (words[0]) : int (words[0]);
  return 0;
  // return (!words.empty ()) * (neg ? -int (words[0]) : int (words[0]));
}

/** math operational **/
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
      if (compare_word (remaining, res) >= 0) {
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
// returning division result and this has remaining
BigInteger BigInteger::div_mod (const BigInteger b) {
  BigInteger r(div_word(words, b.words), false);
  r.neg *= (r.words.size () != 0);
  return r;
}
/** re-initialize **/
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
/** safe operator **/
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
BigInteger &BigInteger::operator-= (const signed b) {
  const word B = word (abs (b));
  if (neg != (b < 0))
    add_a_word (words);
  else
    neg ^= sub_a_word (words, B);
  return *this;
}
BigInteger &BigInteger::operator*= (const signed b) {
  wstack B{(word)abs(b)};
  words = mul_word(words, B);
  neg = (words.size() != 0) & (neg ^ (b < 0));
  return *this;
}
BigInteger &BigInteger::operator/= (const signed b) {
  wstack B{(word)abs (b)};
  words = div_word(words, B);
  neg = (words.size() != 0) & (neg ^ (b < 0));
  return *this;
}
BigInteger &BigInteger::operator%= (const signed b) {
  wstack B{(word)abs(b)};
  div_word(words, B);
  neg &= words.size() != 0;
  return *this;
}
BigInteger &BigInteger::operator+= (const BigInteger b) {
  if (neg == b.neg)
    add_word (words, b.words);
  else
    neg ^= sub_word (words, b.words);
  return *this;
}
BigInteger &BigInteger::operator-= (const BigInteger b) {
  if (neg != b.neg)
    add_word (words, b.words);
  else
    neg ^= sub_word (words, b.words);
  return *this;
}
BigInteger &BigInteger::operator*= (const BigInteger b) {
  words = mul_word(words, b.words);
  neg = (words.size() != 0) & (neg ^ b.neg);
  return *this;
}
BigInteger &BigInteger::operator/= (const BigInteger b) {
  words = div_word(words, b.words);
  neg = (words.size() != 0) & (neg ^ b.neg);
  return *this;
}
BigInteger &BigInteger::operator%= (const BigInteger b) {
  div_word(words, b.words);
  neg &= words.size() != 0;
  return *this;
}
/** safe bitwise operand **/
BigInteger &operator>>= (BigInteger &a, size_t i) {
  shift_right_word(a.words, i);
  return a;
}
BigInteger &operator<<= (BigInteger &a, size_t i) {
  shift_left_word(a.words, i);
  return a;
}
/** compare operator **/
bool operator== (const BigInteger &a, const signed b) {
  wstack B{(word)abs(b)};
  return (a.neg == (b < 0)) && !compare_word(a.words, B);
}
bool operator!= (const BigInteger &a, const signed b) {
  wstack B{(word)abs(b)};
  return (a.neg != (b < 0)) || compare_word(a.words, B);
}
bool operator<= (const BigInteger &a, const signed b) {
  if (a.neg != (b < 0)) return a.neg;
  wstack B{(word)abs(b)};
  return (compare_word(a.words, B) <= 0) ^ a.neg;
}
bool operator>= (const BigInteger &a, const signed b) {
  if (a.neg != (b < 0)) return !a.neg;
  wstack B{(word)abs(b)};
  return (compare_word(a.words, B) >= 0) ^ a.neg;
}
bool operator<  (const BigInteger &a, const signed b) {
  if (a.neg != (b < 0)) return a.neg;
  wstack B{(word)abs(b)};
  return (compare_word(a.words, B) < 0) ^ a.neg;
}
bool operator>  (const BigInteger &a, const signed b) {
  if (a.neg != (b < 0)) return !a.neg;
  wstack B{(word)abs(b)};
  return (compare_word(a.words, B) > 0) ^ a.neg;
}
bool operator== (const BigInteger &a, const BigInteger b) {
  return (a.neg == b.neg) && !compare_word(a.words, b.words);
}
bool operator!= (const BigInteger &a, const BigInteger b) {
  return (a.neg != b.neg) || compare_word(a.words, b.words);
}
bool operator<= (const BigInteger &a, const BigInteger b) {
  if (a.neg != b.neg) return a.neg;
  return (compare_word(a.words, b.words) <= 0) ^ a.neg;
}
bool operator>= (const BigInteger &a, const BigInteger b) {
  if (a.neg != b.neg) return b.neg;
  return (compare_word(a.words, b.words) >= 0) ^ a.neg;
}
bool operator<  (const BigInteger &a, const BigInteger b) {
  if (a.neg != b.neg) return a.neg;
  return (compare_word(a.words, b.words) < 0) ^ a.neg;
}
bool operator>  (const BigInteger &a, const BigInteger b) {
  if (a.neg != b.neg) return b.neg;
  return (compare_word(a.words, b.words) > 0) ^ a.neg;
}
/** new object generate, operator **/
BigInteger BigInteger::operator+ (const signed b) const {
  return BigInteger (*this) += b;
}
BigInteger BigInteger::operator- (const signed b) const {
  return BigInteger (*this) -= b;
}
BigInteger BigInteger::operator* (const signed b) const {
  wstack B{(word)abs(b)};
  BigInteger r(mul_word(r.words, B), false);
  r.neg = (r.words.size() != 0) & (neg ^ (b < 0));
  return r;
}
BigInteger BigInteger::operator/ (const signed b) const {
  return BigInteger(*this) /= b;
}
BigInteger BigInteger::operator% (const signed b) const {
  return BigInteger(*this) %= b;
}
BigInteger BigInteger::operator+ (const BigInteger b) const {
  return BigInteger (*this) += b;
}
BigInteger BigInteger::operator- (const BigInteger b) const {
  return BigInteger (*this) -= b;
}
BigInteger BigInteger::operator* (const BigInteger b) const {
  BigInteger r(mul_word(words, b.words), neg ^ b.neg);
  r.neg &= (r.words.size() != 0);
  return r;
}
BigInteger BigInteger::operator/ (const BigInteger b) const {
  return BigInteger(*this) /= b;
}
BigInteger BigInteger::operator% (const BigInteger b) const {
  return BigInteger(*this) %= b;
}
BigInteger BigInteger::operator- () const {
  return BigInteger (words, !neg);
}
/** new object generate, bitwise operand **/
BigInteger operator>> (const BigInteger &A, size_t i) {
  BigInteger a (A);
  shift_right_word(a.words, i);
  return a;
}
BigInteger operator<< (const BigInteger &A, size_t i) {
  BigInteger a (A);
  shift_left_word(a.words, i);
  return a;
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
      while (A.size () && !A.back ())
        A.pop_back ();
    } while (!A.empty ());

    A = num.words;
    char *text = (char *)malloc (texN + 1);
    text[texN] = '\0';
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
      if (A.size () && !A.back ())
        A.pop_back ();
    } while (!A.empty ());
    while (tcr > text) *(--tcr) = ' ';
    out << text;
    free (text);
  }
  return out;
}
