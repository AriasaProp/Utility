/* *****************************************************************************
 * bigInteger.c v0.0.0000
 * object that stored integer as dynamic array
 * 
 * 
 * 
 * 
 * *****************************************************************************/
#include "math/bigInteger.h"
#include "util/console_out.h"

// private macro
static const iter WORD_BYTES = sizeof (word);
static const iter WORD_BITS = WORD_BYTES * 8;
static const iter WORD_HALF_BITS = WORD_BITS / 2;
static const word WORD_HALF_MASK = ((word)-1) >> WORD_HALF_BITS;

/** private function **/
static word bigInteger__wordadd(word *a, const iter an, const word *b, const iter bn) {
  word c = 0;
  iter i = 0;
  while (i < bn) {
    c = (a[i] += c) < c;
    c += (a[i] += b[i]) < b[i];
    ++i;
  }
  while (c && (i < an))
    c = (a[i++] += c) < c;
  return c;
}
static word bigInteger__wordsub(word *a, const iter an, const word *b, const iter bn) {
  word c = 0, t;
  iter i = 0;
  while (i < bn) {
    t = a[i];
    c = (a[i] -= c) > t;
    t = a[i];
    c += (a[i] -= b[i]) > t;
    ++i;
  }
  while (c && (i < an)) {
    t = a[i];
    c = (a[i++] -= c) > t;
  }
  return c;
}
static void bigInteger__move(bigInteger *a, bigInteger *b) {
  da_copy(a,b);
  a->neg = b->neg;
  bigInteger_free(b);
}
static void bigInteger__shrink(bigInteger *a) {
  while(a->count && !da_last(a)) --a->count;
}
static int bigInteger__cmpa(word *a, word *b, const iter n) {
  //  +1 mean a is greater, -1 mean a is less, 0 mean equal
  int ret = 0;
  for (iter i = n; !ret && i--;)
    ret = (a[i] > b[i]) - (a[i] < b[i]);
  return ret;
}
static int bigInteger__cmp(const bigInteger a, const bigInteger b) {
  //  +1 mean a is greater, -1 mean a is less, 0 mean equal
  int ret = (a.count > b.count) - (a.count < b.count);
  if (!ret) ret = bigInteger__cmpa(a.items, b.items, a.count);
  return ret;
}
static void bigInteger__addition(bigInteger *a, const word c) {
  word r = c;
  if (a->count) r = bigInteger__wordadd(a->items, a->count, &c, 1);
  if (r) da_append (a, r);
} 
static void bigInteger__Addition(bigInteger *a, const bigInteger b) {
  da_atleast(a, MAX(a->count, b.count));
  word r = bigInteger__wordadd(a->items, a->count, b.items, b.count);
  if (r) da_append(a, r);
}
static void bigInteger__subtract(bigInteger *a, const word c) {
  da_atleast(a, 1);
  word r = bigInteger__wordsub(a->items, a->count, &c, 1);
  if (r) {
    da_foreach (word, w, a)
      r &= !(*w = ~(*w) + r);
    ++(*a->items);
    a->neg ^= 1;
  }
  bigInteger__shrink(a);
}
static void bigInteger__Subtract(bigInteger *a, const bigInteger b) {
  da_atleast(a, MAX(a->count, b.count));
  word r = bigInteger__wordsub(a->items, a->count, b.items, b.count);
  if (r) {
    da_foreach (word, w, a)
      r &= !(*w = ~(*w) + r);
    a->neg ^= 1;
  }
  bigInteger__shrink(a);
}
static void bigInteger__multiply(bigInteger *a, const word b) {
  const word bhi = b >> WORD_HALF_BITS, blo = b & WORD_HALF_MASK;
  word ihi, ilo, carry = 0;
  for (iter i = 0; i < a->count; ++i) {
    ihi = a->items[i]>> WORD_HALF_BITS;
    ilo = a->items[i] & WORD_HALF_MASK;
    a->items[i] *= b;
    a->items[i] += carry;
    carry = blo * ilo;
    carry >>= WORD_HALF_BITS;
    carry += blo * ihi;
    carry += bhi * ilo;
    carry >>= WORD_HALF_BITS;
    carry += bhi * ihi;
  }
  if (carry) da_append(a, carry);
  else bigInteger__shrink(a);
}
static bigInteger bigInteger__Multiply(const bigInteger a, const bigInteger b) {
  bigInteger c = {0};
  da_atleast(&c, a.count + b.count + 1);
  word xhi,xlo,yhi,ylo,mt;
  for (iter x = 0,y,z; x < a.count; ++x) {
    xhi = a.items[x]>> WORD_HALF_BITS;
    xlo = a.items[x] & WORD_HALF_MASK;
    for (y = 0; y < b.count; ++y) {
      yhi = b.items[y]>> WORD_HALF_BITS;
      ylo = b.items[y] & WORD_HALF_MASK;
      // muls
      mt = a.items[x] * b.items[y];
      // adds
      for (z = x + y; mt && (z < c.count); ++z)
        mt = (c.items[z] += mt) < mt;
      if (mt) da_append(&c,mt);
      // muls 2
      mt = xlo * ylo;
      mt >>= WORD_HALF_BITS;
      mt += xlo * yhi;
      mt += xhi * ylo;
      mt >>= WORD_HALF_BITS;
      mt += xhi * yhi;
      // adds 2
      for (z = x + y + 1; mt && (z < c.count); ++z)
        mt = (c.items[z] += mt) < mt;
      if (mt) da_append(&c,mt);
    }
  }
  bigInteger__shrink(&c);
  return c;
}
static word bigInteger__division(bigInteger *a, word b) {
  word rmr = 0, c;
  da_rforeach(word, i, a) {
    c = *i;
    rmr <<= WORD_HALF_BITS;
    rmr |= c >> WORD_HALF_BITS;
    *i = rmr / b;
    rmr %= b;
    rmr <<= WORD_HALF_BITS;
    rmr |= c & WORD_HALF_MASK;
    *i <<= WORD_HALF_BITS;
    *i |= rmr / b;
    rmr %= b;
  }
  a->count -= a->count && !da_last(a);
  return rmr;
}
static void bigInteger__shiftleft(bigInteger *a, iter i) {
  iter bit_shift = i % WORD_BITS;
  if (bit_shift) {
    word carry = 0, c;
    da_foreach(word, ia, a) {
      c = *ia;
      *ia <<= bit_shift;
      *ia |= carry;
      carry = c >> (WORD_BITS - bit_shift);
    }
    if (carry) da_append(a, carry);
  }
  iter word_shift = i / WORD_BITS;
  if (word_shift) {
    da_reserve(a, a->count + word_shift);
    util_memmove(a->items + word_shift, a->items, a->count * sizeof(word));
    a->count += word_shift;
  }
}
static void bigInteger__shiftright(bigInteger *a, iter i) {
  iter word_shift = i / WORD_BITS;
  iter bit_shift = i % WORD_BITS;
  if (word_shift) {
    a->count -= word_shift;
    util_memcpy(a->items, a->items + word_shift, a->count * sizeof(word));
  }
  if (bit_shift) {
    word carry = 0, c;
    da_rforeach(word, ia, a) {
      c = *ia;
      *ia >>= bit_shift;
      *ia |= carry;
      carry = c << (WORD_BITS - bit_shift);
    }
    a->count -= a->count && !da_last(a);
  }
}
static bigInteger bigInteger__sqrt(const bigInteger a) {
  bigInteger res = {0};
  bigInteger rem = {0};
  word carry, carry1;
  iter i = 0;
  if (a.count) {
    i = util_bitlead(da_last(&a));
    i += (i & 1);
    i += (a.count - 1) * WORD_BITS;
    da_append(&res, 0);
  }
  while (i) { 
    i -= 2;
    // extract 2 binary from source A
    carry1 = (a.items[i / WORD_BITS] >> (i % WORD_BITS)) & 3;
    // remaining shift left 2 append source A
    da_foreach(word, remi, &rem) {
      carry = *remi;
      *remi <<= 2;
      *remi |= carry1;
      carry1 = carry >> (WORD_BITS - 2);
    }
    if (carry1) da_append(&rem, carry1);
    // result shift 2 left
    carry1 = 1;
    da_foreach(word, resi, &res) {
      carry = *resi;
      *resi <<= 2;
      *resi |= carry1;
      carry1 = carry >> (WORD_BITS - 2);
    }
    if (carry1) da_append(&res, carry1);
    // compare result test with remaining
    if (bigInteger__cmp (rem, res) >= 0) {
      carry = bigInteger__wordsub(rem.items, rem.count, res.items, res.count);
      ASSERT(!carry && "sqrt rem less than res!");
      res.items[0] |= 2;
    }
    // shift 1 right to get pure result
    carry1 = 0;
    da_rforeach(word, resi, &res) {
      carry = *resi;
      *resi >>= 1;
      *resi |= carry1;
      carry1 = carry << (WORD_BITS - 1);
    }
  }
  bigInteger_free(&rem);
  res.count -= (res.count && !da_last(&res));
  return res;
}
static void bigInteger__pow2(bigInteger *a) {
  const iter ccap = a->count * 2 + 1;
  word *c = CAST(word*)util_calloc(WORD_BYTES, ccap);
  word xhi,xlo,yhi,ylo,mt,mt1;
  for (iter x = 0,y,z; x < a->count; ++x) {
    xhi = a->items[x]>> WORD_HALF_BITS;
    xlo = a->items[x] & WORD_HALF_MASK;
    for (y = 0; y < a->count; ++y) {
      yhi = a->items[y]>> WORD_HALF_BITS;
      ylo = a->items[y] & WORD_HALF_MASK;
      // muls
      mt1 = a->items[x] * a->items[y];
      // adds
      z = x + y;
      mt1 = (c[z] += mt1) < mt1;
      // muls 2
      mt = xlo * ylo;
      mt >>= WORD_HALF_BITS;
      mt += xlo * yhi;
      mt += xhi * ylo;
      mt >>= WORD_HALF_BITS;
      mt += xhi * yhi;
      mt += mt1;
      // adds 2
      while (mt && (++z < ccap)) mt = (c[z] += mt) < mt;
      ASSERT(!mt && "pow to 2 generate digit to many!");
    }
  }
  da_atleast(a, ccap);
  util_memcpy(a->items, c, WORD_BYTES * ccap);
  bigInteger__shrink(a);
  util_memfree(c);
}
static void bigInteger__pow(bigInteger *a, iter b) {
  bigInteger r = bigInteger_from_int(1);
  while (b) {
    if (b & 1) {
      bigInteger r1 = bigInteger__Multiply(r, *a);
      bigInteger__move(&r, &r1);
    }
    bigInteger__pow2(a);
    b >>= 1;
  }
  bigInteger__move(a, &r);
}

// initialization
bigInteger bigInteger_from_cstr(const char *c) {
  bigInteger ret = {0};
  // read sign
  if (*c == '-') ret.neg |= 1, c++;
  else ret.neg &= 0;
  // read digits
  word carry, tmp, tp;
  while (*c) {
    carry = *c - '0';
    if (carry >= 10) break;
    da_foreach(word, cur, &ret) {
      tmp = (*cur >> (WORD_BITS - 3));
      tmp+= (*cur >> (WORD_BITS - 1));
      tp = *cur << 1;
      *cur <<= 3;
      tmp+= (*cur += tp) < tp;
      tmp+= (*cur += carry) < carry;
      carry = tmp;
    }
    if (carry) da_append(&ret, carry);
    c++;
  }
  return ret;
}
bigInteger bigInteger_from_int(const int in) {
  bigInteger ret = {.neg = (in < 0), .items = NULL, .cap = 0, .count = 0};
  da_append(&ret, CAST(word)imath_iabs (in));
  return ret;
}
void bigInteger_set_cstr(bigInteger *a, const char *c) {
  da_clean(a);
  // read sign
  if (*c == '-') a->neg |= 1, c++;
  else a->neg &= 0;
  // read digits
  word carry, tmp, tp;
  while (*c) {
    carry = *c - '0';
    if (carry >= 10) break;
    da_foreach(word, cur, a) {
      tmp = (*cur >> (WORD_BITS - 3));
      tmp+= (*cur >> (WORD_BITS - 1));
      tp = *cur << 1;
      *cur <<= 3;
      tmp+= (*cur += tp) < tp;
      tmp+= (*cur += carry) < carry;
      carry = tmp;
    }
    if (carry) da_append(a, carry);
    c++;
  }
}
void bigInteger_set_int(bigInteger *a, const int in) {
  a->neg = (in < 0);
  da_clean(a);
  da_append(a, CAST(word)imath_iabs (in));
}
void bigInteger_set(bigInteger *a, const bigInteger b) {
  da_copy(a,&b);
  a->neg = b.neg;
}
// helper
bigInteger bigInteger_dup(const bigInteger x) {
  word *d = CAST(word*)util_malloc(WORD_BYTES * x.count);
  util_memcpy(d, x.items, x.count * WORD_BYTES);
  return (bigInteger){.neg = x.neg, .items = d, .cap = x.count, .count = x.count};
}
void bigInteger_free(bigInteger *x) {
  da_free(x);
}
// compare 2 bigInteger, which 0 is equal, -1 left smaller, 1 left bigger  
int bigInteger_cmp(const bigInteger a, const bigInteger b) {
  int ret = b.neg - a.neg;
  if (!ret) ret = bigInteger__cmp(a, b);
  if (a.neg) ret *= -1;
  return ret;
}
// return division result, save reminder on nominator
bigInteger bigInteger_div_mmod(bigInteger *a, const bigInteger b) {
  bigInteger res = {0};
  word c, c1;
  if (b.count && (bigInteger__cmp(*a, b) >= 0)) {
    res.neg = a->neg ^ b.neg;
    iter s = (a->count - b.count) * WORD_BITS - util_clz(da_last(a)) + util_clz(da_last(&b));
    bigInteger d = bigInteger_dup(b);
    bigInteger__shiftleft(&d, s);
    for (;;) {
      // check remainder bigger than divider
      c1 = bigInteger__cmp(*a, d) >= 0;
      if (c1) {
        c = bigInteger__wordsub(a->items, a->count, d.items, d.count);
        ASSERT(!c && "bigInteger divider (subtraction) shouldn't bigger than reminder!");
        bigInteger__shrink(a);
      }
      // shift left 1 bits result
      da_foreach(word, ir, &res) {
        c = *ir;
        *ir <<= 1;
        *ir |= c1;
        c1 = c >> (WORD_BITS - 1);
      }
      if (c1) da_append(&res, c1);
      if (!a->count || !s) break;
      // shift right 1 bits divider
      c1 = 0;
      da_rforeach(word, id, &d) {
        c = *id;
        *id >>= 1;
        *id |= c1;
        c1 = c << (WORD_BITS - 1);
      }
      d.count -= !da_last(&d);
      --s;
    }
    bigInteger__shiftleft(&res, s);
    bigInteger_free(&d);
    res.count -= res.count && !da_last(&res);
  }
  return res;
}
// duplicate operate
bigInteger bigInteger_redc(const bigInteger a) {
  bigInteger r = bigInteger_dup(a);
  bigInteger_mredc(&r);
  return r;
}
bigInteger bigInteger_incr(const bigInteger a) {
  bigInteger r = bigInteger_dup(a);
  bigInteger_mincr(&r);
  return r;
}
bigInteger bigInteger_addi(const bigInteger a, const int c) {
  bigInteger r = bigInteger_dup(a);
  if (r.neg ^ (c < 0)) bigInteger__subtract(&r, CAST(word)imath_iabs(c));
  else bigInteger__addition(&r, CAST(word)imath_iabs(c));
  return r;
}
bigInteger bigInteger_subi(const bigInteger a, const int c) {
  bigInteger r = bigInteger_dup(a);
  if (r.neg & (c < 0)) bigInteger__subtract(&r, CAST(word)imath_iabs(c));
  else bigInteger__addition(&r, CAST(word)imath_iabs(c));
  return r;
}
bigInteger bigInteger_muli(const bigInteger a, const int c) {
  bigInteger r = bigInteger_dup(a);
  bigInteger_mmuli(&r, CAST(word)imath_iabs(c));
  return r;
}
bigInteger bigInteger_divi(const bigInteger a, const int c) {
  bigInteger r = bigInteger_dup(a);
  UNUSED(bigInteger__division(&r, CAST(word)imath_iabs(c)));
  r.neg ^= (c < 0);
  return r;
}
bigInteger bigInteger_powi(const bigInteger a, const uint b) {
  bigInteger r = bigInteger_dup(a);
  bigInteger__pow(&r,b);
  r.neg &= (b & 1);
  return r;
}
bigInteger bigInteger_shfli(const bigInteger a, const uint i) {
  bigInteger r = bigInteger_dup(a);
  bigInteger__shiftleft(&r, i);
  return r;
}
bigInteger bigInteger_shfri(const bigInteger a, const uint i) {
  bigInteger r = bigInteger_dup(a);
  bigInteger__shiftright(&r, i);
  return r;
}
bigInteger bigInteger_sqrt(const bigInteger a) {
  return bigInteger__sqrt(a);
}
bigInteger bigInteger_add(const bigInteger a, const bigInteger b) {
  bigInteger r = bigInteger_dup(a);
  if (r.neg ^ b.neg) bigInteger__Subtract(&r, b);
  else bigInteger__Addition(&r, b);
  return r;
}
bigInteger bigInteger_sub(const bigInteger a, const bigInteger b) {
  bigInteger r = bigInteger_dup(a);
  if (r.neg ^ b.neg) bigInteger__Addition(&r, b);
  else bigInteger__Subtract(&r, b);
  return r;
}
bigInteger bigInteger_mul(const bigInteger a, const bigInteger b) {
  bigInteger r = bigInteger__Multiply(a, b);
  r.neg = a.neg ^ b.neg;
  return r;
}
bigInteger bigInteger_div(const bigInteger a, const bigInteger b) {
  bigInteger rem = bigInteger_dup(a);
  bigInteger r = bigInteger_div_mmod(&rem, b);
  bigInteger_free(&rem);
  return r;
}
bigInteger bigInteger_mod(const bigInteger a, const bigInteger b) {
  bigInteger r = bigInteger_dup(a);
  bigInteger_mmod(&r, b);
  return r;
}
// modification operate no error should be occure
void bigInteger_mredc(bigInteger *a) {
  // just do a single bit
  word *i = a->items, *j = i + a->count;
  word carry = 1;
  if (a->neg) { // do addition
    while ((i < j) && (carry = !(++(*(i++))))) ;
    if (carry) da_append (a, 1);
  } else { // do subtraction
    while ((i < j) && (carry = !(*(i++))--)) ;
    if (carry) da_append (a, 1);
    a->neg |= 1;
    bigInteger__shrink(a);
  }
}
void bigInteger_mincr(bigInteger *a) {
  // just do a single bit
  word *i = a->items, *j = i + a->count;
  word carry = 1;
  if (a->neg) { // do subtraction
    while ((i < j) && (carry = !((*(i++))--))) ;
    if (carry) da_append (a, 1);
    a->neg &= 0;
    bigInteger__shrink(a);
  } else { // do addition
    while ((i < j) && (carry = !(++(*(i++))))) ;
    if (carry) da_append (a, 1);
  }
}
void bigInteger_maddi(bigInteger *a, const int c) {
  if (a->neg ^ (c < 0)) bigInteger__subtract(a, CAST(word)imath_iabs(c));
  else bigInteger__addition(a, CAST(word)imath_iabs(c));
}
void bigInteger_msubi(bigInteger *a, const int c) {
  if (a->neg ^ (c < 0)) bigInteger__addition(a, CAST(word)imath_iabs(c));
  else bigInteger__subtract(a, CAST(word)imath_iabs(c));
}
void bigInteger_mmuli(bigInteger *a, const int c) {
  bigInteger__multiply(a, c);
  a->neg ^= (c < 0);
} 
void bigInteger_mdivi(bigInteger *a, const int c) {
  UNUSED(bigInteger__division(a, CAST(word)imath_iabs(c)));
  a->neg ^= (c < 0);
}
void bigInteger_mmodi(bigInteger *a, const uint c) {
  word r = bigInteger__division(a, CAST(word)imath_iabs(c));
  bigInteger_set_int(a, r);
}
void bigInteger_mpowi(bigInteger *a, const uint b) {
  bigInteger__pow(a, b);
  a->neg &= (b & 1);
}
void bigInteger_mshfli(bigInteger *a, const uint i) {
  bigInteger__shiftleft(a, i);
}
void bigInteger_mshfri(bigInteger *a, const uint i) {
  bigInteger__shiftright(a, i);
}
void bigInteger_msqrt(bigInteger *a) {
  bigInteger r = bigInteger__sqrt(*a);
  bigInteger__move(a, &r);
}
void bigInteger_madd(bigInteger *a, const bigInteger b) {
  if (a->neg ^ b.neg) bigInteger__Subtract(a, b);
  else bigInteger__Addition(a,b);
}
void bigInteger_msub(bigInteger *a, const bigInteger b) {
  if (a->neg ^ b.neg) bigInteger__Addition(a,b);
  else bigInteger__Subtract(a, b);
}
void bigInteger_mmul(bigInteger *a, const bigInteger b) {
  bigInteger r = bigInteger__Multiply(*a, b);
  r.neg = a->neg ^ b.neg;
  bigInteger__move(a, &r);
}
void bigInteger_mdiv(bigInteger *a, const bigInteger b) {
  bigInteger res = bigInteger_div_mmod(a, b);
  bigInteger__move(a, &res);
}
void bigInteger_mmod(bigInteger *a, const bigInteger b) {
  if (bigInteger__cmp(*a, b) < 0) return;
  word c = 0, c1, c2;
  bigInteger d = bigInteger_dup(b);
  d.neg = 0;
  c = CAST(iter)(imath_log2(CAST(float)(da_last(a) + 1)) - imath_log2(CAST(float)da_last(&d)));
  // shift divider to left
  if (c) {
    c1 = 0;
    da_foreach(word, id, &d) {
      c2 = *id;
      *id <<= c;
      *id |= c1;
      c1 = c2 >> (WORD_BITS - c);
    }
    if (c1) da_append(&d, c1);
  }
  iter s = a->count - d.count;
  // shift divider to left each word
  da_reserve(&d, d.count + s);
  util_memmove(d.items + s, d.items, d.count * sizeof(word));
  d.count += s;
  // turn s into bits
  s *= WORD_BITS;
  s += c;
  do {
    // check remainder bigger than divider
    if (bigInteger__cmp(*a, d) >= 0) {
      // substract
      c = bigInteger__wordsub(a->items, a->count, d.items, d.count);
      ASSERT(!c && "bigInteger divider (subtraction) never bigger than remainder!");
      bigInteger__shrink(a);
    }
    c1 = 0;
    da_rforeach(word, id, &d) {
      c = *id;
      *id >>= 1;
      *id |= c1;
      c1 = c << (WORD_BITS - 1);
    }
    bigInteger__shrink(&d);
  } while (s--);
  bigInteger_free(&d);
  bigInteger__shrink(a);
}

// print out
void bigInteger_append_string(String *str, const bigInteger a) {
  word rmr, current;
  iter ac = a.count;
  iter string_old = string_len(*str);
  string_reserve(str, string_old + ac * 10);
  word *aw = CAST(word*)util_malloc(WORD_BYTES * ac);
  util_memcpy(aw, a.items, WORD_BYTES * ac);
  do {
    rmr = 0;
    for(word *i = aw + ac; i > aw; ){
      --i;
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
    string_append_char(str, '0' + CAST(char)rmr);
    ac -= ac && !aw[ac - 1];
  } while (ac);
  if (a.neg) string_append_char(str, '-');
  util_memflip(*str + string_old, string_len(*str) - string_old);
  util_memfree(aw);
}

