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

using wit = wstack::iterator;
using cwit = wstack::const_iterator;
using rwit = wstack::reverse_iterator;
using crwit = wstack::const_reverse_iterator;

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
  size_t bit = i % WORD_BITS;
  size_t word_shift = i / WORD_BITS;
  
  if (bit) {
    size_t rbit = WORD_BITS - bit;
    word carry = 0, c;
    for (word &val : a) {
      c = val;
      val <<= bit;
      val |= carry;
      carry = c >> rbit;
    }
    if (carry) a.push_back(carry);
  }

  if (word_shift) {
    size_t old_size = a.size();
    a.resize(old_size + word_shift, 0);
    for (size_t idx = old_size; idx-- > 0; ) {
      a[idx + word_shift] = a[idx];
    }
    std::fill(a.begin(), a.begin() + word_shift, 0);
  }
}
static void shift_right_word(wstack &a, size_t i) {
  size_t word_shift = i / WORD_BITS;
  
  if (word_shift >= a.size()) {
    a.clear();
    return;
  }
  
  if (word_shift) {
    for (size_t idx = 0; idx + word_shift < a.size(); ++idx) {
      a[idx] = a[idx + word_shift];
    }
    a.resize(a.size() - word_shift);
  }

  size_t bit = i % WORD_BITS;
  if (bit && a.size()) {
    size_t rbit = WORD_BITS - bit;
    word carry = 0, c;
    for (size_t idx = a.size(); idx-- > 0; ) {
      c = a[idx];
      a[idx] = (c >> bit) | carry;
      carry = c << rbit;
    }
    if (!a.empty() && a.back() == 0) a.pop_back();
  }
}
static wstack div_word(wstack &a, wstack b) {
  size_t s = a.size(), nb = b.size();
  wstack c;
  if (s && nb && (s >= nb)) {
    s = (s - nb) * WORD_BITS;
    word carry;
    for (carry = a.back(); carry; carry >>= 1) ++s;
    for (carry = b.back(); carry; carry >>= 1) if (!(s--)) return c;
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

/*
static wstack div_word(wstack &a, word b) {
  size_t s = a.size();
  wstack c;
  if (s) {
    word_together wt;
    s = (s - nb) * WORD_BITS;
    word rmr = 0;
    size_t r = 2;
    for (rwit i = a.rbegin (), j = a.rend(); i < j; ++i) {
      wt.a = *i;
      for ( r--; ) {
        rmr <<= WORD_HALF_BITS;
        rmr |= wt.s[r];
        *i = rmr / 10;
        rmr %= 10;
      }
    }
    if (A.size () && !A.back ())
      A.pop_back ();
  }
  return c;
}
*/
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
BigInteger BigInteger::pow (word exponent) const {
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
  words = div_word(words, wstack{(word)abs(b)});
  neg = !words.empty() && (neg ^ (b < 0));
  return *this;
}
BigInteger &BigInteger::operator%= (const signed b) {
  div_word(words, wstack{(word)abs(b)});
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
/** compare operator **/
bool operator== (const BigInteger &a, const signed b) {
  return !cmp_word (a.neg, a.words, b);
}
bool operator!= (const BigInteger &a, const signed b) {
  return cmp_word (a.neg, a.words, b);
}
bool operator<= (const BigInteger &a, const signed b) {
  return cmp_word (a.neg, a.words, b) <= 0;
}
bool operator>= (const BigInteger &a, const signed b) {
  return cmp_word (a.neg, a.words, b) >= 0;
}
bool operator<  (const BigInteger &a, const signed b) {
  return cmp_word (a.neg, a.words, b) < 0;
}
bool operator>  (const BigInteger &a, const signed b) {
  return cmp_word (a.neg, a.words, b) > 0;
}
bool operator== (const BigInteger &a, const BigInteger b) {
  return !cmp_word(a.neg, a.words, b.neg, b.words);
}
bool operator!= (const BigInteger &a, const BigInteger b) {
  return cmp_word(a.neg, a.words, b.neg, b.words);
}
bool operator<= (const BigInteger &a, const BigInteger b) {
  return cmp_word(a.neg, a.words, b.neg, b.words) <= 0;
}
bool operator>= (const BigInteger &a, const BigInteger b) {
  return cmp_word(a.neg, a.words, b.neg, b.words) >= 0;
}
bool operator<  (const BigInteger &a, const BigInteger b) {
  return cmp_word(a.neg, a.words, b.neg, b.words) < 0;
}
bool operator>  (const BigInteger &a, const BigInteger b) {
  return cmp_word(a.neg, a.words, b.neg, b.words) > 0;
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
  r.words = div_word(r.words, wstack{(word)abs(b)});
  r.neg = !r.words.empty() && (r.neg ^ (b < 0));
  return r;
}
BigInteger BigInteger::operator% (const signed b) const {
  BigInteger r(*this);
  div_word(r.words, wstack{(word)abs(b)});
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
  std::reverse(texts.begin(), texts.end());
  texts.push_back(0);
  out << texts.data();
  return out;
}
