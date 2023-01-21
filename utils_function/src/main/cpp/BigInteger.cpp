#include "BigInteger.h"

#include <climits>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdlib>

const size_t WORD_BITS = sizeof(word) * CHAR_BIT;
const size_t WORD_BITS_1 = WORD_BITS - 1;
const word WORD_MASK =(word) - 1;
const size_t WORD_HALF_BITS = sizeof(word) * CHAR_BIT / 2;
const word WORD_HALF_MASK = WORD_MASK >> WORD_HALF_BITS;
const long double LOG2BITS = std::log10(2.99999) * WORD_BITS;

//private function for repeated use
// +1 mean a is greater, -1 mean a is less, 0 mean equal
int compare(const std::vector<word>&a, const std::vector<word>&b) {
  if (&a == &b) return 0;
  size_t as = a.size(), bs = b.size();
  if (as != bs)
    return as>bs ? +1 : -1;
  while (as--)
    if (a[as] != b[as])
      return a[as]>b[as] ? +1 : -1;
  return 0;
}
void add_a_word(std::vector<word>&a, size_t i = 0, word carry = 1) {
  const size_t j = a.size();
  while ((i<j) && carry)
    carry = carry>(a[i++] += carry);
  if (carry)
    a.push_back(carry);
}
// a should be greater or equal than carry
void sub_a_word(std::vector<word>&a, size_t i = 0, word carry = 1) {
  const size_t j = a.size(); 
  while ((i<j) && carry)
    carry = a[i]<(a[i] -= carry), i++;
  while (a.size() &&!a.back())
    a.pop_back();
}
// params b shall be same memory as a
void add_word(std::vector<word>&a, const std::vector<word>&b) {
  size_t i = 0, j = b.size();
  if (a.size()<j)
    a.resize(j, 0);
  word carry = 0, c;
  while (i<j) {
	c = b[i]; //because b can be same memory as a
    carry = carry>(a[i] += carry);
    carry += c>(a[i] += c);
    i++;
  }
  j = a.size();
  while ((i<j) && carry)
    carry =carry>(a[i++] += carry);
  if (carry)
    a.push_back(carry);
}
// params b shall be same memory as a but it always compare before operation
void sub_word(std::vector<word>&a, const std::vector<word>&b) {
  size_t i = 0, j = b.size();
  word carry = 0;
  while (i<j) {
    carry = a[i]<(a[i] -= carry);
    carry += a[i]<(a[i] -= b[i]);
    i++;
  }
  j = a.size();
  while ((i<j) && carry)
    carry = a[i]<(a[i] -= carry), i++;
  while (a.size() &&!a.back())
    a.pop_back();
}

//initialize BigInteger functions

//Constructors
BigInteger::BigInteger() {}
BigInteger::BigInteger(const BigInteger &a): neg(a.neg), words(a.words) {}
BigInteger::BigInteger(const signed &i): neg(i<0) {
  unsigned u = abs(i);
  while (u) {
    words.push_back(u);
    size_t i = WORD_BITS;
    while (i--)
      u >>= 1;
  }
}
// in constructor should begin with sign or number, add '\0' in last world to close number
BigInteger::BigInteger(const char *c): neg(false) {
  // read sign
  if ( *c == '-')
    neg = true, c++;
  // read digits
  word carry, tmp, a_hi, a_lo, tp;
  while ( *c) {
    carry = *c - '0';
    if (carry>9)
      throw ("BigInteger should initialize with number from 0 to 9.");
    for (word &cur: words) {
      tmp = cur *10;
      carry =(tmp += carry)<carry;
      a_hi = cur >> WORD_HALF_BITS;
      a_lo = cur &WORD_HALF_MASK;
      tp =((a_lo *10) >> WORD_HALF_BITS) + a_hi *10;
      carry +=(tp >> WORD_HALF_BITS) + ((tp &WORD_HALF_MASK) >> WORD_HALF_BITS);
      cur = tmp;
    }
    if (carry)
      words.push_back(carry);
    c++;
  }
}
BigInteger::BigInteger(const std::vector<word>&v, bool neg = false): neg(neg), words(v) {}
// Destructors
BigInteger::~BigInteger() {
  words.clear();
}
//environment count
size_t BigInteger::tot() const {
  return words.size() * WORD_BITS;
}
char *BigInteger::to_chars() const {
  std::vector<char>text;
  std::vector<word>A = words;
  text.reserve(LOG2BITS *A.size() + 2);
  word remainder, current;
  size_t i;
  while ((i = A.size())) {
    remainder = 0;
    while(i--) {
    	word &cur = A[i];
      current = cur;
      remainder <<= WORD_HALF_BITS;
      remainder |= current >> WORD_HALF_BITS;
      cur <<= WORD_HALF_BITS;
      cur |= remainder / 10;
      remainder %= 10;
      remainder <<= WORD_HALF_BITS;
      remainder |= current & WORD_HALF_MASK;
      cur <<= WORD_HALF_BITS;
      cur |= remainder / 10;
      remainder %= 10;
    }
    text.push_back('0' + char(remainder));
    while (A.size() && !A.back())
      A.pop_back();
  }
  if (neg)
    text.push_back('-');
  std::reverse(text.begin(), text.end());
  text.push_back('\0');
  char *result = new char[text.size()];
  std::copy(text.begin(), text.end(), result);
  return result;
}
double BigInteger::to_double() const {
  if (!words.size())
    return 0.0;
  double d = 0.0, base = std::pow(2.0, WORD_BITS);
  for (size_t i = words.size(); i--;)
    d = d *base + words[i];
  return neg ? -d : d;
}
bool BigInteger::can_convert_to_int(int *result) const {
  if (words.size()) {
    if (words.size()>1 || words[0] >=(WORD_MASK >> 1))
      return false;
    *result = words[0];
    if (neg)
      *
      result = -( *result);
  } else
    *
    result = 0;
  return true;
}
//math operational
BigInteger BigInteger::sqrt() const {
  BigInteger result;
  size_t n = words.size();
  if (n) {
    n =(n - 1) *WORD_BITS;
    word carry = words.back();
    while (carry)
      n++, carry >>= 1;
    if (n &1)
      n++;
    std::vector<word>remaining(1, 0), &res = result.words;
    res.push_back(0);
    size_t i, j;
    const size_t l_shift = WORD_BITS - 2;
    const word setLast = ~word(1);
    while (n) {
      n -= 2;
      //remaining shift left 2
      carry = remaining.back() >> l_shift;
      for (i = remaining.size() - 1; i; i--)
        remaining[i] =(remaining[i] << 2) | (remaining[i - 1] >> l_shift);
      remaining[i] <<= 2;
      remaining[i] |=(words[n / WORD_BITS] >> (n % WORD_BITS)) &word(3);
      if (carry)
        remaining.push_back(carry);
      //result shift 1 left with test
      carry = res.back() >> WORD_BITS_1;
      for (i = res.size() - 1; i; i--)
        res[i] =(res[i] << 1) | (res[i - 1] >> WORD_BITS_1);
      res[i] =(res[i] << 1) | 1;
      if (carry)
        res.push_back(carry);
      // compare result test with remaining
      if (compare(remaining, res) >= 0) {
        carry = 0;
        for (i = 0, j = res.size(); i<j; i++) {
          carry = remaining[i]<(remaining[i] -= carry);
          carry += remaining[i]<(remaining[i] -= res[i]);
        }
        for (j = remaining.size(); (i<j) && carry; i++)
          carry = remaining[i]<(remaining[i] -= carry);
        res[0] |= 2;
      }
      res[0] &= setLast;
    }
    // shift 1 right to get pure result
    for (i = 0, j = res.size() - 1; i<j; i++)
      res[i] =(res[i + 1] << WORD_BITS_1) | (res[i] >> 1);
    res.back() >>= 1;
    while (res.size() &&!res.back())
      res.pop_back();
  }
  return result;
}
//re-initialize
BigInteger &BigInteger::operator=(const signed &a) {
  if (words.size())
    words.clear();
  neg =(a<0);
  unsigned u = abs(a);
  size_t i;
  while (u) {
    words.push_back(u);
    i = WORD_BITS;
    while (i--)
      u >>= 1;
  }
  return *this;
}
BigInteger &BigInteger::operator=(const BigInteger &a) {
  words = a.words;
  neg = a.neg;
  return *this;
}
//safe operator
BigInteger &BigInteger::operator--() {
  if (neg)
    add_a_word(words);
  else
    sub_a_word(words);
  return *this;
}
BigInteger &BigInteger::operator++() {
  if (neg)
    sub_a_word(words);
  else
    add_a_word(words);
  return *this;
}
BigInteger &BigInteger::operator+=(const s_word &b) {
  const word B = word(abs(b));
  if (!words.size()) {
    neg = std::signbit(b);
    words.push_back(B);
  } else if (neg == std::signbit(b))
    add_a_word(words, 0, B);
  else {
    if ((words.size()>1) || (words[0]>B))
      sub_a_word(words, 0, B);
    else if (words[0]<B) {
      neg = !neg;
      words[0] = B - words[0];
    } else {
      neg = false;
      words.clear();
    }
  }
  return *this;
}
BigInteger &BigInteger::operator+=(const BigInteger &b) {
  if (neg == b.neg) {
	add_word(words, b.words);
  } else {
    int cmp = compare(words, b.words);
    if (cmp<0) {
      std::vector<word>t = words;
      neg = b.neg;
      words = b.words;
      sub_word(words, t);
    } else if (cmp>0)
      sub_word(words, b.words);
    else {
      neg = false;
      words.clear();
    }
  }
  return *this;
}
BigInteger &BigInteger::operator-=(const s_word &b) {
  const word B = word(abs(b));
  if (!words.size()) {
    neg = !std::signbit(b);
    words.push_back(B);
  } else if (neg != std::signbit(b))
    add_a_word(words, 0, B);
  else {
    if ((words.size()>1) || (words[0]>B))
      sub_a_word(words, 0, B);
    else if (words[0]<B) {
      neg = !neg;
      words[0] = B - words[0];
    } else {
      neg = false;
      words.clear();
    }
  }
  return *this;
}
BigInteger &BigInteger::operator-=(const BigInteger &b) {
  if (neg != b.neg)
    add_word(words, b.words);
  else {
    int cmp = compare(words, b.words);
    if (cmp<0) {
      neg = !neg;
      std::vector<word>t = words;
      words = b.words;
      sub_word(words, t);
    } else if (cmp>0) {
      sub_word(words, b.words);
    } else {
      neg = false;
      words.clear();
    }
  }
  return *this;
}
BigInteger &BigInteger::operator*=(const s_word &b) {
  if (b != 0) {
    std::vector<word>&r = words;
    const size_t j = r.size();
    if (j) {
      neg ^= std::signbit(b);
      const word B = word(abs(b));
      word A[j];
      std::copy(r.begin(), r.end(), A);
      const word b_hi = B >> WORD_HALF_BITS;
      const word b_lo = B &WORD_HALF_MASK;
      word a_hi, a_lo, carry = 0, carry0;
      for (size_t i = 0; i<j; i++) {
        word &wA = A[i];
        a_hi = wA >> WORD_HALF_BITS;
        a_lo = wA &WORD_HALF_MASK;
        r[i] = wA * B;
        carry =(r[i] += carry)<carry;
        carry0 =(a_lo *b_lo) >> WORD_HALF_BITS;
        carry0 += a_hi *b_lo;
        carry += carry0 >> WORD_HALF_BITS;
        carry +=(a_lo *b_hi + (carry0 &WORD_HALF_MASK)) >> WORD_HALF_BITS;
        carry += a_hi *b_hi;
      }
      if (carry)
        r.push_back(carry);
    }
  } else {
    neg = false;
    words.clear();
  }
  return *this;
}
BigInteger &BigInteger::operator*=(const BigInteger &b) {
  const size_t nb = b.words.size();
  if (!nb) {
    neg = false;
    words.clear();
  }
  std::vector<word>&a = words;
  const size_t na = a.size();
  if (na) {
    neg ^= b.neg;
    word A[na], B[nb];
    std::copy(a.begin(), a.end(), A);
    std::copy(b.words.begin(), b.words.end(), B);
    a.clear();
    a.resize(na + nb, 0);
    word a_hi, b_hi, a_lo, b_lo, carry, carry0;
    size_t ia, ib, i, j;
    for (ia = 0; ia<na; ia++) {
      const word &Ar = A[ia];
      a_hi = Ar >> WORD_HALF_BITS;
      a_lo = Ar &WORD_HALF_MASK;
      carry = 0;
      for (ib = 0; ib<nb; ib++) {
        i = ia + ib;
        word &r = a[i];
        carry =(r += carry)<carry;
        const word &Br = B[ib];
        b_hi = Br >> WORD_HALF_BITS;
        b_lo = Br &WORD_HALF_MASK;
        //part 1
        carry0 = Ar *Br;
        carry +=(r += carry0)<carry0;
        //part 2
        carry0 =(a_lo *b_lo) >> WORD_HALF_BITS;
        carry0 += a_hi *b_lo;
        //TODO:  this may overflow, if you find error in multiplication check this
        carry += carry0 >> WORD_HALF_BITS;
        carry +=(a_lo *b_hi + (carry0 &WORD_HALF_MASK)) >> WORD_HALF_BITS;
        carry += a_hi *b_hi;
      }
      j = a.size();
      while (((++i)<j) &&carry)
        carry =(a[i] += carry)<carry;
      if (carry)
        a.push_back(carry);
    }
    while (a.size() &&!a.back())
      a.pop_back();
  }
  return *this;
}
BigInteger &BigInteger::operator/=(const s_word &b) {
  if (b == 0)
    throw ("Undefined number cause / 0 !");
  std::vector<word>rem = words, div{word(abs(b))};
  words.clear();
  if (compare(rem, div) >= 0) {
    //shifting count
    size_t i = div.size();
    size_t j =(rem.size() - i) * WORD_BITS;
    word carry0 = rem.back();
    while (carry0)
      j++, carry0 >>= 1;
    carry0 = div.back();
    while (carry0)
      j--, carry0 >>= 1;
    size_t n = j % WORD_BITS;
    if (n) {
      const size_t l_shift = WORD_BITS - n;
      carry0 = div.back() >> l_shift;
      while (--i)
        div[i] =(div[i] << n) | (div[i - 1] >> l_shift);
      div[i] <<= n;
      if (carry0)
        div.push_back(carry0);
    }
    n = j / WORD_BITS;
    if (n)
      div.insert(div.begin(), n, 0);
    words.resize(n + 1, 0);
    do {
      if (compare(rem, div) >= 0) {
        words[j / WORD_BITS] |= word(1) << (j % WORD_BITS);
        sub_word(rem, div);
        if (!rem.size()) break;
      }
      //reverse shift one by one
      for (i = 0, n = div.size() - 1; i<n; i++)
        div[i] =(div[i] >> 1) | (div[i + 1] << WORD_BITS_1);
      div[i] >>= 1;
      while (!div.back())
        div.pop_back();
    } while (j--);
    while (words.size() &&!words.back())
      words.pop_back();
    neg ^= std::signbit(b);
  } else
    neg = false;
  // return result
  return *this;
}
BigInteger &BigInteger::operator/=(const BigInteger &b) {
  if (!b.words.size())
    throw ("Undefined number cause / 0 !");
  std::vector<word>rem = words, div = b.words;
  words.clear();
  if (compare(rem, div) >= 0) {
    //shifting count
    size_t i = div.size();
    size_t j =(rem.size() - i) *WORD_BITS;
    word carry0 = rem.back();
    while (carry0)
      j++, carry0 >>= 1;
    carry0 = div.back();
    while (carry0)
      j--, carry0 >>= 1;
    size_t n = j % WORD_BITS;
    if (n) {
      const size_t l_shift = WORD_BITS - n;
      carry0 = div.back() >> l_shift;
      while (--i)
        div[i] =(div[i] << n) | (div[i - 1] >> l_shift);
      div[i] <<= n;
      if (carry0)
        div.push_back(carry0);
    }
    n = j / WORD_BITS;
    if (n)
      div.insert(div.begin(), n, 0);
    words.resize(n + 1, 0);
    do {
      if (compare(rem, div) >= 0) {
        words[j / WORD_BITS] |= word(1) << (j % WORD_BITS);
        sub_word(rem, div);
        if (!rem.size())
          break;
      }
      //reverse shift one by one
      for (i = 0, n = div.size() - 1; i<n; i++)
        div[i] =(div[i] >> 1) | (div[i + 1] << WORD_BITS_1);
      div[i] >>= 1;
      if (!div.back())
        div.pop_back();
    } while (j--);
    while (words.size() &&!words.back())
      words.pop_back();
    neg ^= b.neg;
  } else {
    neg = false;
  }
  return *this;
}
BigInteger &BigInteger::operator%=(const s_word &b) {
  if (b != 0) {
    std::vector<word>&rem = words, div {word(abs(b))};
    int cmp = compare(rem, div);
    if (cmp >= 0) {
      //shifting count
      size_t i = div.size();
      size_t j =(rem.size() - i) *WORD_BITS;
      word carry0 = rem.back();
      while (carry0)
        j++, carry0 >>= 1;
      carry0 = div.back();
      while (carry0)
        j--, carry0 >>= 1;
      size_t n = j % WORD_BITS;
      if (n) {
        const size_t l_shift = WORD_BITS - n;
        carry0 = div.back() >> l_shift;
        while (--i)
          div[i] =(div[i] << n) | (div[i - 1] >> l_shift);
        div[i] <<= n;
        if (carry0)
          div.push_back(carry0);
      }
      n = j / WORD_BITS;
      if (n)
        div.insert(div.begin(), n, 0);
      do {
        cmp = compare(rem, div);
        if (cmp>0)
          sub_word(rem, div);
        else if (cmp == 0) {
          neg = false;
          rem.clear();
          break;
        }
        //reverse shift one by one
        for (i = 0, n = div.size() - 1; i<n; i++)
          div[i] =(div[i] >> 1) | (div[i + 1] << WORD_BITS_1);
        div[i] >>= 1;
        if (!div.back())
          div.pop_back();
      } while (j--);
    }
  }
  return *this;
}
BigInteger &BigInteger::operator%=(const BigInteger &b) {
  if (b.words.size()) {
    std::vector<word>&rem = words, div = b.words;
    int cmp = compare(rem, div);
    if (cmp >= 0) {
      //shifting count
      size_t i = div.size();
      size_t j =(rem.size() - i) *WORD_BITS;
      word carry0 = rem.back();
      while (carry0)
        j++, carry0 >>= 1;
      carry0 = div.back();
      while (carry0)
        j--, carry0 >>= 1;
      size_t n = j % WORD_BITS;
      if (n) {
        const size_t l_shift = WORD_BITS - n;
        carry0 = div.back() >> l_shift;
        while (--i)
          div[i] =(div[i] << n) | (div[i - 1] >> l_shift);
        div[i] <<= n;
        if (carry0)
          div.push_back(carry0);
      }
      n = j / WORD_BITS;
      if (n)
        div.insert(div.begin(), n, 0);
      do {
        cmp = compare(rem, div);
        if (cmp>0)
          sub_word(rem, div);
        else if (cmp == 0) {
          neg = false;
          rem.clear();
          break;
        }
        //reverse shift one by one
        for (i = 0, n = div.size() - 1; i<n; i++)
          div[i] =(div[i] >> 1) | (div[i + 1] << WORD_BITS_1);
        div[i] >>= 1;
        if (!div.back())
          div.pop_back();
      } while (j--);
    }
  }
  return *this;
}
BigInteger &BigInteger::operator^=(size_t exponent) {
  size_t na = words.size();
  if (na) {
    std::vector<word>p = words, &R = words;
    R.clear();
    R.push_back(1);
    word A[na *exponent], B[na *exponent];
    neg = neg &(exponent &1);
    word a_hi, a_lo, b_hi, b_lo, carry, carry0;
    size_t ia, ib, i = 0, j, nb;
    while (exponent) {
      nb = p.size();
      if (exponent &1) {
        na = R.size();
        std::copy(R.begin(), R.end(), A);
        std::copy(p.begin(), p.end(), B);
        R.clear();
        R.resize(na + nb, 0);
        for (ia = 0; ia<na; ia++) {
          const word &Ar = A[ia];
          a_hi = Ar >> WORD_HALF_BITS;
          a_lo = Ar &WORD_HALF_MASK;
          carry = 0;
          for (ib = 0; ib<nb; ib++) {
            i = ia + ib;
            word &r = R[i];
            carry =(r += carry)<carry;
            const word &Br = B[ib];
            b_hi = Br >> WORD_HALF_BITS;
            b_lo = Br &WORD_HALF_MASK;
            // part 1
            carry0 = Ar *Br;
            carry +=(r += carry0)<carry0;
            //part 2
            carry0 = a_lo *b_lo;
            carry0 >>= WORD_HALF_BITS;
            carry0 += a_hi *b_lo;
            //TODO:  this may overflow, if you find error in multiplication check this
            carry +=(carry0 >> WORD_HALF_BITS) + ((a_lo *b_hi + (carry0 &WORD_HALF_MASK)) >> WORD_HALF_BITS) + a_hi *b_hi;
          }
          j = R.size();
          while (((++i)<j) &&carry)
            carry =(R[i] += carry)<carry;
          if (carry)
            R.push_back(carry);
        }
        while (R.size() &&!R.back())
          R.pop_back();
      }
      exponent >>= 1;
      std::copy(p.begin(), p.end(), A);
      p.clear();
      p.resize(nb *2, 0);
      for (ia = 0; ia<nb; ia++) {
        const word &Ar = A[ia];
        a_hi = Ar >> WORD_HALF_BITS;
        a_lo = Ar &WORD_HALF_MASK;
        carry = 0;
        for (ib = 0; ib<nb; ib++) {
          i = ia + ib;
          word &r = p[i];
          carry =(r += carry)<carry;
          const word &Br = A[ib];
          b_hi = Br >> WORD_HALF_BITS;
          b_lo = Br &WORD_HALF_MASK;
          // part 1
          carry0 = Ar *Br;
          carry +=(r += carry0)<carry0;
          //part 2
          carry0 = a_lo *b_lo;
          carry0 >>= WORD_HALF_BITS;
          carry0 += a_hi *b_lo;
          //TODO:  this may overflow, if you find error in multiplication check this
          carry +=(carry0 >> WORD_HALF_BITS) + ((a_lo *b_hi + (carry0 &WORD_HALF_MASK)) >> WORD_HALF_BITS) + a_hi *b_hi;
        }
        j = p.size();
        while (((++i)<j) &&carry)
          carry =(p[i] += carry)<carry;
        if (carry)
          p.push_back(carry);
      }
      while (p.size() &&!p.back())
        p.pop_back();
    }
  } else if (!exponent)
    throw ("Undefined result!");
  return *this;
}
BigInteger &BigInteger::operator>>=(size_t n_bits) {
  if (n_bits && words.size()) {
    size_t j = n_bits / WORD_BITS;
    if (j<words.size()) {
      std::vector<word>::iterator carried = words.begin();
      words.erase(carried, carried + j);
      n_bits %= WORD_BITS;
      if (n_bits) {
        std::vector<word>::iterator endCarried = words.end() - 1;
        const size_t r_shift = WORD_BITS - n_bits;
        *carried >>= n_bits;
        while (carried != endCarried) {
          *carried |= *(carried + 1) << r_shift;
          carried++;
          *carried >>= n_bits;
        }
        if ( *endCarried == 0)
          words.pop_back();
      }
    } else {
      neg = false;
      words.clear();
    }
  }
  return *this;
}
BigInteger &BigInteger::operator<<=(size_t bits) {
  if (bits) {
    size_t n = bits % WORD_BITS;
    if (n) {
      const size_t l_shift = WORD_BITS - n;
      std::vector<word>::reverse_iterator carried = words.rbegin();
      std::vector<word>::reverse_iterator endCarried = words.rend() - 1;
      word lo = *carried >> l_shift;
      while (carried != endCarried) {
        *carried <<= n;
        *carried |= *(carried + 1) >> l_shift;
        carried++;
      }
      *carried <<= n;
      if (lo)
        words.push_back(lo);
    }
    n = bits / WORD_BITS;
    if (n)
      words.insert(words.begin(), n, 0);
  }
  return *this;
}
//compare operator
bool BigInteger::operator==(const BigInteger &b) const {
  if (neg != b.neg)
    return false;
  size_t i = words.size(), j = b.words.size();
  if (i != j)
    return false;
  while (i--)
    if (words[i] != b.words[i])
      return false;
  return true;
}
bool BigInteger::operator!=(const BigInteger &b) const {
  if (neg != b.neg)
    return true;
  size_t i = words.size(), j = b.words.size();
  if (i != j)
    return true;
  while (i--)
    if (words[i] != b.words[i])
      return true;
  return false;
}
bool BigInteger::operator<=(const BigInteger &b) const {
  if (neg != b.neg)
    return neg;
  size_t i = words.size(), j = b.words.size();
  if (i != j)
    return (i<j) ^ neg;
  while (i--)
    if (words[i] != b.words[i])
      return (words[i]<b.words[i]) ^ neg;
  return true;
}
bool BigInteger::operator>=(const BigInteger &b) const {
  if (neg != b.neg)
    return b.neg;
  size_t i = words.size(), j = b.words.size();
  if (i != j)
    return (i>j) ^ neg;
  while (i--)
    if (words[i] != b.words[i])
      return (words[i]>b.words[i]) ^ neg;
  return true;
}
bool BigInteger::operator<(const BigInteger &b) const {
  if (neg != b.neg)
    return neg;
  size_t i = words.size(), j = b.words.size();
  if (i != j)
    return (i<j) ^ neg;
  while (i--)
    if (words[i] != b.words[i])
      return (words[i]<b.words[i]) ^ neg;
  return false;
}
bool BigInteger::operator>(const BigInteger &b) const {
  if (neg != b.neg)
    return b.neg;
  size_t i = words.size(), j = b.words.size();
  if (i != j)
    return (i>j) ^ neg;
  while (i--)
    if (words[i] != b.words[i])
      return (words[i]>b.words[i]) ^ neg;
  return false;
}
//unsafe operator
BigInteger BigInteger::operator+(const s_word &b) const {
  return BigInteger(*this)+= b;
}
BigInteger BigInteger::operator+(const BigInteger &b) const {
  return BigInteger(*this)+= b;
}
BigInteger BigInteger::operator-(const s_word &b) const {
  return BigInteger(*this)-= b;
}
BigInteger BigInteger::operator-(const BigInteger &b) const {
  return BigInteger(*this)-= b;
}
BigInteger BigInteger::operator*(const s_word &b) const {
  return BigInteger(*this)*= b;
}
BigInteger BigInteger::operator*(const BigInteger &b) const {
  return BigInteger(*this)*= b;
}
BigInteger BigInteger::operator/(const s_word &b) const {
  return BigInteger(*this)/= b;
}
BigInteger BigInteger::operator/(const BigInteger &b) const {
  return BigInteger(*this)/= b;
}
BigInteger BigInteger::operator%(const s_word &b) const {
  return BigInteger(*this)%= b;
}
BigInteger BigInteger::operator%(const BigInteger &b) const {
  return BigInteger(*this)%= b;
}
BigInteger BigInteger::operator^(size_t exponent) const {
  return BigInteger(*this)^= exponent;
}
BigInteger BigInteger::operator-() const {
  return BigInteger(words, !neg);
}
BigInteger BigInteger::operator>> (size_t n_bits) const {
  return BigInteger(*this)>>= n_bits;
}
BigInteger BigInteger::operator<< (size_t n_bits) const {
  return BigInteger(*this)<<= n_bits;
}

std::ostream &operator<<(std::ostream &out, const BigInteger &num) {
  std::vector<char> text;
  std::vector<word> A = num.words;
  text.reserve(LOG2BITS * A.size() + 1);
  word remainder, current;
  size_t i;
  while ((i = A.size())) {
    remainder = 0;
    while(i--) {
    	word &cur = A[i];
      current = cur;
      remainder <<= WORD_HALF_BITS;
      remainder |= current >> WORD_HALF_BITS;
      cur = remainder / 10;
      remainder %= 10;
      remainder <<= WORD_HALF_BITS;
      remainder |= current & WORD_HALF_MASK;
      cur <<= WORD_HALF_BITS;
      cur |= remainder / 10;
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