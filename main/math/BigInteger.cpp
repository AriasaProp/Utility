#include "math/BigInteger.hpp"
#include "common.hpp"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>

using wstack = std::vector<word>;
using wit = wstack::iterator;
using cwit = wstack::const_iterator;
using rwit = wstack::reverse_iterator;
using crwit = wstack::const_reverse_iterator;

constexpr size_t WORD_BITS = sizeof (word) * CHAR_BIT;
constexpr size_t WORD_BITS_1 = WORD_BITS - 1;
constexpr size_t WORD_HALF_BITS = WORD_BITS / 2;
constexpr word WORD_HALF_MASK = ((word)-1) >> WORD_HALF_BITS;

/** private function **/
//  +1 mean a is greater, -1 mean a is less, 0 mean equal
static int cmp_word (const bool an, const wstack &a, const signed b) {
  size_t na = a.size();
  if (na == 0 && b == 0) return 0;
  int r = (b < 0) - an;
  if (r == 0) {
    r = (na > 1) - (na < 1);
    if (!r) {
      word B = abs(b);
      r = (a[0] > B) - (a[0] < B);
    }
    if (an) r = -r;
  }
  return r;
}
static int cmp_word (const wstack &a, const wstack &b) {
  size_t na = a.size(), nb = b.size();
  int r = (na > nb) - (na < nb);
  while ((r == 0) && na)
    --na, r = (a[na] > b[na]) - (a[na] < b[na]);
  return r;
}
static int cmp_word (const bool an, const wstack &a, const bool bn, const wstack &b) {
  int r = bn - an;
  if (r == 0) {
    r = cmp_word(a, b);
    r = an ? -r : r;
  }
  return r;
}
// do addition & subtraction
static bool lowf_word (wstack &a, bool f, word carry = 1) {
  wit i = a.begin(), j = a.end();
  bool inv = false;
  if (f) { // subtraction
    while ((i < j) && carry)
      carry = *i < (*i -= carry), ++i;
    if (carry) {
      for (word &w : a)
        w = ~w;
      inv = true;
    }
  } else { // addition
    while ((i < j) && carry)
      carry = carry > (*(i++) += carry);
    if (carry) a.push_back (carry);
  }
  while (a.size () && !a.back ())
    a.pop_back ();
  return inv;
}
static bool lowf_word (wstack &a, bool f, const wstack &b) {
  if (a.size () < b.size ()) a.resize (b.size (), 0);
  wit i = a.begin ();
  cwit jend = b.cend (), j = b.cbegin ();
  word carry = 0, c;
  bool inv = false;
  if (f) {
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
      inv = true;
    }
  } else {
    while (j < jend) {
      c = *(j++); // because b can be same memory as a
      carry = carry > (*i += carry);
      carry += c > (*i += c);
      ++i;
    }
    j = a.cend ();
    while ((i < j) && carry)
      carry = carry > (*(i++) += carry);
    if (carry) a.push_back (carry);
  }
  while (a.size () && !a.back ())
    a.pop_back ();
  return inv;
}
// multiplication logic
static wstack mul_word (const wstack &a, const wstack &b) {
  wstack c;
  c.resize(a.size() + b.size(), 0);
  wit ic = c.begin(), ic1, jc;
  cwit ia = a.cbegin(), ja = a.cend(), ib, jb;
  word carry, temp0, temp1;
  word ahi, alo, bhi, blo;
  while(ia < ja) {
    ahi = *ia >> WORD_HALF_BITS;
    alo = *ia & WORD_HALF_MASK;
    ic1 = ic;
    ib = b.cbegin(), jb = b.cend();
    carry = 0;
    // b mul
    while (ib < jb) {
      temp0 = *ia * *ib;
      carry += temp0 > (*ic1 += temp0);
      ++ic1;
      carry = carry > (*ic1 += carry);
      bhi = *ib >> WORD_HALF_BITS;
      blo = *ib & WORD_HALF_MASK;
      temp0 = (alo * blo) >> WORD_HALF_BITS;
      temp0 += alo * bhi;
      temp1 = ahi * blo;
      temp1 = (temp0 += temp1) < temp1;
      temp0 = (temp0 >> WORD_HALF_BITS) + (temp1 << WORD_HALF_BITS);
      carry += temp0 > (*ic1 += temp0);
      temp0 = ahi * bhi;
      carry += temp0 > (*ic1 += temp0);
      ++ib;
    }
    jc = c.end();
    while(((++ic1) < jc) && carry)
      carry = carry > (*ic1 += carry);
    if (carry) c.push_back(carry);
    ++ia, ++ic;
  }
  while (c.size() && !c.back())
    c.pop_back();
  return c;
}
static wstack mul_word (const wstack &a, const word b) {
  wstack c;
  c.resize(a.size() + 1, 0);
  wit ic = c.begin(), ic1, jc;
  cwit ia = a.cbegin(), ja = a.cend();
  word carry, temp0, temp1;
  word ahi, alo, bhi = b >> WORD_HALF_BITS, blo = b & WORD_HALF_MASK;
  while(ia < ja) {
    ahi = *ia >> WORD_HALF_BITS;
    alo = *ia & WORD_HALF_MASK;
    ic1 = ic;
    // b mul
    temp0 = *ia * b;
    carry = temp0 > (*ic1 += temp0);
    ++ic1;
    carry = carry > (*ic1 += carry);
    temp0 = (alo * blo) >> WORD_HALF_BITS;
    temp0 += alo * bhi;
    temp1 = ahi * blo;
    temp1 = (temp0 += temp1) < temp1;
    temp0 = (temp0 >> WORD_HALF_BITS) + (temp1 << WORD_HALF_BITS);
    carry += temp0 > (*ic1 += temp0);
    temp0 = ahi * bhi;
    carry += temp0 > (*ic1 += temp0);
    jc = c.end();
    while(((++ic1) < jc) && carry)
      carry = carry > (*ic1 += carry);
    if (carry) c.push_back(carry);
    ++ia, ++ic;
  }
  while (c.size() && !c.back())
    c.pop_back();
  return c;
}
static void shift_left_word(wstack &a, size_t i) {
  size_t bit_shift = i % WORD_BITS;
  if (bit_shift) {
    size_t rbit_shift = WORD_BITS - bit_shift;
    word carry = 0, c;
    for (word &val : a) {
      c = val;
      val <<= bit_shift;
      val |= carry;
      carry = c >> rbit_shift;
    }
    if (carry) a.push_back(carry);
  }
  a.insert(a.begin(), i / WORD_BITS, 0);
}
static void shift_right_word(wstack &a, size_t i) {
  a.erase(a.begin(), MIN(a.begin() + i / WORD_BITS, a.end()));

  size_t bit_shift = i % WORD_BITS;
  if (bit_shift) {
    size_t rbit_shift = WORD_BITS - bit_shift;
    word carry = 0, c;
    size_t idx = a.size();
    while (idx--) {
      c = a[idx];
      a[idx] = (c >> bit_shift) | carry;
      carry = c << rbit_shift;
    }
    if (a.size() && !a.back()) a.pop_back();
  }
}
/*
a + b * 10 + c * 100 + ...
¥
c / ¥ => res + rem

*/
static wstack div_word(wstack &a, wstack b) {
  size_t s = a.size(), nb = b.size();
  wstack c;
  if (s && nb && (s >= nb)) {
    s = (s - nb) * WORD_BITS;
    s += (size_t)(log((a.back() + 1) / b.back()) / log(2));
    shift_left_word(b, s);
    c.push_back(0);
    do {
      shift_left_word(c, 1);
      if (cmp_word(a, b) >= 0) {
        lowf_word(a, true, b);
        c[0] |= 1;
      }
      shift_right_word(b, 1);
    } while (s--);
    while (c.size() && !c.back()) c.pop_back();
  }
  return c;
}
static wstack div_word(wstack &a, const word b) {
  wstack c = std::move(a);
  word rmr = 0, curr = 0;
  for (rwit i = c.rbegin (), j = c.rend(); i < j; ++i) {
    curr = *i;
    rmr <<= WORD_HALF_BITS;
    rmr |= curr >> WORD_HALF_BITS;
    *i = rmr / b;
    rmr %= b;
    rmr <<= WORD_HALF_BITS;
    rmr |= curr & WORD_HALF_MASK;
    *i <<= WORD_HALF_BITS;
    *i |= rmr / b;
    rmr %= b;
  }
  while (c.size() && !c.back()) c.pop_back();
  a = wstack{rmr};
  while (a.size() && !a.back()) a.pop_back();
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
  word carry, tmp, tp;
  word ahi, alo;
  while (*c) {
    carry = *c - '0';
    if (carry > 9)
      throw ("BigInteger should initialize with number from 0 to 9.");
    for (word &cur : words) {
      tmp = cur * 10;
      carry = (tmp += carry) < carry;
      ahi = cur >> WORD_HALF_BITS;
      alo = cur & WORD_HALF_MASK;
      tp = ((alo * 10) >> WORD_HALF_BITS) + (ahi * 10);
      carry += tp >> WORD_HALF_BITS;
      cur = tmp;
    }
    if (carry)
      words.push_back (carry);
    c++;
  }
}
BigInteger::BigInteger (const wstack &v, bool neg = false) : neg (neg), words (v) {}
/** Destructors **/
BigInteger::~BigInteger () {}
/** operator casting **/
BigInteger::operator bool () const {
  return words.size ();
}
BigInteger::operator int () const {
  if (words.empty ()) return 0;
  int ret = 0xafffffff;
  if (*this < ret) ret = words[0] & ret;
  if (neg) ret *= -1;
  return ret;
}
BigInteger::operator char () const {
  if (words.empty ()) return 0;
  char ret = 0xaf;
  if (*this < ret) ret = words[0] & ret;
  if (neg) ret *= -1;
  return ret;
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
      if (cmp_word (remaining, res) >= 0) {
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
// returning division result and this has remaining
BigInteger BigInteger::div_mod (const BigInteger b) {
  return BigInteger(div_word(words, b.words), neg ^ b.neg);
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
BigInteger &BigInteger::operator= (const BigInteger &a) {
  words = a.words;
  neg = a.neg;
  return *this;
}
/** safe operator **/
BigInteger &BigInteger::operator-- () {
  neg ^= lowf_word (words, !neg);
  neg = words.size() && neg;
  return *this;
}
BigInteger &BigInteger::operator++ () {
  neg ^= lowf_word (words, neg);
  neg = words.size() && neg;
  return *this;
}
BigInteger &BigInteger::operator+= (const signed b) {
  neg ^= lowf_word (words, neg != (b < 0), (word) abs (b));
  neg = words.size() && neg;
  return *this;
}
BigInteger &BigInteger::operator-= (const signed b) {
  neg ^= lowf_word (words, neg == (b < 0), (word) abs (b));
  neg = words.size() && neg;
  return *this;
}
BigInteger &BigInteger::operator*= (const signed b) {
  words = mul_word(words, (word)abs(b));
  neg = words.size() && (neg ^ (b < 0));
  return *this;
}
BigInteger &BigInteger::operator/= (const signed b) {
  words = div_word(words, (word)abs(b));
  neg = !words.empty() && (neg ^ (b < 0));
  return *this;
}
BigInteger &BigInteger::operator%= (const signed b) {
  div_word(words, (word)abs(b));
  neg &= !words.empty();
  return *this;
}
BigInteger &BigInteger::operator+= (const BigInteger b) {
  neg ^= lowf_word (words, neg != b.neg, b.words);
  neg = words.size() && neg;
  return *this;
}
BigInteger &BigInteger::operator-= (const BigInteger b) {
  neg ^= lowf_word (words, neg == b.neg, b.words);
  neg = words.size() && neg;
  return *this;
}
BigInteger &BigInteger::operator*= (const BigInteger b) {
  words = mul_word(words, b.words);
  neg = words.size() && (neg ^ b.neg);
  return *this;
}
BigInteger &BigInteger::operator/= (const BigInteger b) {
  words = div_word(words, b.words);
  neg = !words.empty() && (neg ^ b.neg);
  return *this;
}
BigInteger &BigInteger::operator%= (const BigInteger b) {
  div_word(words, b.words);
  neg &= !words.empty();
  return *this;
}
BigInteger &BigInteger::operator^= (size_t exponent) {
  BigInteger A(*this);
  if (words.size ()) words.clear ();
  words.push_back (1);
  while (exponent) {
    if (exponent & 1)
      *this *= A;
    A *= A;
    exponent >>= 1;
  }
  return *this;
}
BigInteger &BigInteger::operator<<=(const size_t l) {
  shift_left_word(words, l);
  neg &= !words.empty();
  return *this;
}
BigInteger &BigInteger::operator>>=(const size_t l) {
  shift_right_word(words, l);
  neg &= !words.empty();
  return *this;
}
/** compare operator **/
bool BigInteger::operator== (const signed b) const {
  return !cmp_word (neg, words, b);
}
bool BigInteger::operator!= (const signed b) const {
  return cmp_word (neg, words, b);
}
bool BigInteger::operator<= (const signed b) const {
  return cmp_word (neg, words, b) <= 0;
}
bool BigInteger::operator>= (const signed b) const {
  return cmp_word (neg, words, b) >= 0;
}
bool BigInteger::operator<  (const signed b) const {
  return cmp_word (neg, words, b) < 0;
}
bool BigInteger::operator>  (const signed b) const {
  return cmp_word (neg, words, b) > 0;
}
bool BigInteger::operator== (const BigInteger b) const {
  return !cmp_word(neg, words, b.neg, b.words);
}
bool BigInteger::operator!= (const BigInteger b) const {
  return cmp_word(neg, words, b.neg, b.words);
}
bool BigInteger::operator<= (const BigInteger b) const {
  return cmp_word(neg, words, b.neg, b.words) <= 0;
}
bool BigInteger::operator>= (const BigInteger b) const {
  return cmp_word(neg, words, b.neg, b.words) >= 0;
}
bool BigInteger::operator<  (const BigInteger b) const {
  return cmp_word(neg, words, b.neg, b.words) < 0;
}
bool BigInteger::operator>  (const BigInteger b) const {
  return cmp_word(neg, words, b.neg, b.words) > 0;
}
/** new object generate, operator **/
BigInteger BigInteger::operator- () const {
  BigInteger r(*this);
  r.neg = !r.neg;
  return r;
}
BigInteger BigInteger::operator+ (const signed b) const {
  return BigInteger (*this) += b;
}
BigInteger BigInteger::operator- (const signed b) const {
  return BigInteger (*this) -= b;
}
BigInteger BigInteger::operator* (const signed b) const {
  BigInteger r(mul_word(words, (word)abs(b)), neg ^ (b < 0));
  r.neg = r.neg && r.words.size();
  return r;
}
BigInteger BigInteger::operator/ (const signed b) const {
  BigInteger r(*this);
  r.words = div_word(r.words, (word)abs(b));
  r.neg = !r.words.empty() && (r.neg ^ (b < 0));
  return r;
}
BigInteger BigInteger::operator% (const signed b) const {
  BigInteger r(*this);
  div_word(r.words, (word)abs(b));
  r.neg &= !r.words.empty();
  return r;
}
BigInteger BigInteger::operator+ (const BigInteger b) const {
  return BigInteger (*this) += b;
}
BigInteger BigInteger::operator- (const BigInteger b) const {
  return BigInteger (*this) -= b;
}
BigInteger BigInteger::operator* (const BigInteger b) const {
  BigInteger r(mul_word(words, b.words), neg ^ b.neg);
  r.neg = r.words.size() && r.neg;
  return r;
}
BigInteger BigInteger::operator/ (const BigInteger b) const {
  BigInteger r(*this);
  r.words = div_word(r.words, b.words);
  r.neg = !r.words.empty() && (r.neg ^ b.neg);
  return r;
}
BigInteger BigInteger::operator% (const BigInteger b) const {
  BigInteger r(*this);
  div_word(r.words, b.words);
  r.neg &= !r.words.empty();
  return r;
}
BigInteger BigInteger::operator^ (size_t exponent) const {
  BigInteger r(1);
  BigInteger A = *this;
  while (exponent) {
    if (exponent & 1)
      r *= A;
    A *= A;
    exponent >>= 1;
  }
  return r;
}
BigInteger BigInteger::operator<<(const size_t l) const {
  BigInteger r(*this);
  shift_left_word(r.words, l);
  r.neg &= !r.words.empty();
  return r;
}
BigInteger BigInteger::operator>>(const size_t l) const {
  BigInteger r(*this);
  shift_right_word(r.words, l);
  r.neg &= !r.words.empty();
  return r;
}

std::ostream &operator<< (std::ostream &out, const BigInteger a) {
  word rmr, current;
  wstack A = a.words;
  std::vector<char> texts;
  texts.reserve((A.size() * 10));
  do {
    rmr = 0;
    for (rwit i = A.rbegin (), j = A.rend(); i < j; ++i) {
      current = *i;
      rmr <<= WORD_HALF_BITS;
      rmr |= current >> WORD_HALF_BITS;
      *i = rmr / 10;
      rmr %= 10;
      rmr <<= WORD_HALF_BITS;
      rmr |= current & WORD_HALF_MASK;
      *i <<= WORD_HALF_BITS;
      *i |= rmr / 10;
      rmr %= 10;
    }
    texts.push_back('0' + char (rmr));
    if (A.size () && !A.back ())
      A.pop_back ();
  } while (A.size ());
  if (a.neg) texts.push_back('-');
  reverse(texts.begin(), texts.end());
  texts.push_back(0);
  out << texts.data();
  return out;
}
