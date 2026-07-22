/* *****************************************************************************
 * bigInteger.c v0.0.0000
 * object that stored integer as dynamic array
 * 
 * 
 * 
 * 
 * *****************************************************************************/
#include "math/bigInteger.h"
#include "array/darray.h"

// private macro
#define WORD_BYTES     (sizeof (word))
#define WORD_BITS      (WORD_BYTES << 3)
#define WORD_HALF_BITS (WORD_BITS >> 1)
#define WORD_HALF_MASK (((word)-1) >> WORD_HALF_BITS)

/** private function **/
static word bigInteger__wordadd(word *a, const iter an, const word *b, const iter bn) {
  word c = 0;
  iter i = 0;
  while (i < bn) {
    c = (a[i] += c) < c;
    c+= (a[i] += b[i]) < b[i];
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
    c+= (a[i] -= b[i]) > t;
    ++i;
  }
  while (c && (i < an)) {
    t = a[i];
    c = (a[i++] -= c) > t;
  }
  return c;
}
static void bigInteger__shrink(bigInteger *a) {
  while(a->count && !darray_last(a)) --(a->count);
  if (!a->count) a->neg &= 0;
}
static int bigInteger__cmpa(const word *a, const word *b, iter i) {
  //  +1 mean a is greater, -1 mean a is less, 0 mean equal
  int ret = 0;
  while (!ret && i--)
    ret = (a[i] > b[i]) - (a[i] < b[i]);
  return ret;
}
static int bigInteger__cmp(const bigInteger a, const bigInteger b) {
  // handle back zero that break compare
  iter alen = a.count, blen = b.count;
  while (alen && !a.items[alen - 1]) --alen;
  while (blen && !b.items[blen - 1]) --blen;
  //  +1 mean a is greater, -1 mean a is less, 0 mean equal
  int ret = (alen > blen) - (alen < blen);
  if (!ret) ret = bigInteger__cmpa(a.items, b.items, alen);
  return ret;
}
static void bigInteger__addition(bigInteger *a, const word c) {
  word r = c;
  if (a->count) r = bigInteger__wordadd(a->items, a->count, &c, 1);
  if (r) darray_append (a, r);
} 
static void bigInteger__Addition(bigInteger *a, const bigInteger b) {
  darray_atleast(a, MAX(a->count, b.count));
  word r = bigInteger__wordadd(a->items, a->count, b.items, b.count);
  if (r) darray_append(a, r);
}
static void bigInteger__subtract(bigInteger *a, const word c) {
  darray_atleast(a, 1);
  word r = bigInteger__wordsub(a->items, a->count, &c, 1);
  if (r) {
    darray_foreach (word, w, a)
      r &= (*w = ~(*w) + r);
    a->neg ^= 1;
  }
  bigInteger__shrink(a);
}
static void bigInteger__Subtract(bigInteger *a, const bigInteger b) {
  darray_atleast(a, MAX(a->count, b.count));
  word r = bigInteger__wordsub(a->items, a->count, b.items, b.count);
  if (r) {
    darray_foreach (word, w, a)
      r &= !(*w = ~(*w) + r);
    a->neg ^= 1;
  }
  bigInteger__shrink(a);
}
static void bigInteger__multiply(bigInteger *a, const int B) {
	const word b = CAST(word)imath_iabs(B);
  const word yhi = b >> WORD_HALF_BITS, ylo = b & WORD_HALF_MASK;
  a->neg ^= B < 0;
  word xhi, xlo, carry[3] = {0}, temp;
  darray_foreach(word, i, a) {
    xhi = *i>> WORD_HALF_BITS;
    xlo = *i & WORD_HALF_MASK;
    *i *= b;
    carry[0] = (*i += carry[0]) < carry[0];
    carry[2] = (ylo * xlo) >> WORD_HALF_BITS;
    temp = ylo * xhi;
    carry[1] = (carry[2] += temp) < temp;
    temp = yhi * xlo;
    carry[1] += (carry[2] += temp) < temp;
    carry[2] >>= WORD_HALF_BITS;
    carry[1] <<= WORD_HALF_BITS;
    carry[0] += carry[2];
    carry[0] += carry[1];
    temp = yhi * xhi;
    carry[0] += temp;
  }
  if (carry[0]) darray_append(a, carry[0]);
  else bigInteger__shrink(a);
}
/*
 *  a*b - d
 *  a*b - (B - D)
 *  a*b - B + D
 * Borrow
 * Degate
 *
 */
static bigInteger bigInteger__MultiplyAdd(const bigInteger a, const bigInteger b,const bigInteger d) {
  bigInteger c = bigInteger_dup(d);
  darray_atleast(&c, a.count + b.count + 1);
  c.neg = a.neg ^ b.neg;
  bool borrow = d.neg ^ c.neg;
  word xhi,xlo,yhi,ylo,carry[3] = {0},temp;
  if (borrow) { // borrow
  	carry[0] = 1;
  	darray_foreach(word, ic, &c)
  		carry[0] &= !(*ic = ~*ic + carry[0]);
  }
  for (iter x = 0,y,z; x < a.count; ++x) {
    xhi = a.items[x]>> WORD_HALF_BITS;
    xlo = a.items[x] & WORD_HALF_MASK;
    for (y = 0; y < b.count; ++y) {
      yhi = b.items[y]>> WORD_HALF_BITS;
      ylo = b.items[y] & WORD_HALF_MASK;
      z = x + y;
      carry[0] = (c.items[z] += carry[0]) < carry[0];
      temp = a.items[x] * b.items[y];
      carry[0]+= (c.items[z] += temp) < temp;
      carry[2] = (ylo * xlo) >> WORD_HALF_BITS;
      temp = ylo * xhi;
      carry[1] = (carry[2] += temp) < temp;
      temp = yhi * xlo;
      carry[1]+= (carry[2] += temp) < temp;
      carry[2]>>= WORD_HALF_BITS;
      carry[1]<<= WORD_HALF_BITS;
      
      carry[0] += carry[2];
      carry[0] += carry[1];
      temp = yhi * xhi;
      carry[0] += temp;
    }
    for (z = x + y; carry[0] && (z < c.count); ++z)
      carry[0] = (c.items[z] += carry[0]) < carry[0];
    if (carry[0]) {
    	if (borrow) borrow = false;
    	else darray_append(&c, carry[0]);
  	}
  }
  if (borrow) { // borrow
  	carry[0] = 1;
  	darray_foreach(word, ic, &c)
  		carry[0] &= (*ic = ~*ic + carry[0]);
  	c.neg ^= true;
  }
  bigInteger__shrink(&c);
  return c;
}
static word bigInteger__division(bigInteger *a, const int B) {
	const word b = CAST(word)imath_iabs(B);
  word rem[2] = {0};
  word d, e;
  iter i, j;
  darray_rforeach(word, ia, a) {
    for (j = WORD_BITS; j--; ) {
      e = (*ia >> j) & 1;
      for(i = 0;i < 2; ++i) {
        d = rem[i];
        rem[i] <<= 1;
        rem[i] |= e;
        e = d >> (WORD_BITS - 1);
      }
      ASSERT(!e && "bigInteger__division overflow remainder");
      *ia <<= 1;
      if (rem[1] || rem[0] >= b) {
        rem[1] -= (rem[0] -= b) > b;
        *ia |= 1;
      }
    }
  }
  a->neg ^= B < 0;
  ASSERT(!rem[1] && "bigInteger, word division overflow");
  bigInteger__shrink(a);
  return rem[0];
}
// initialization
bigInteger bigInteger_from_cstr(const char *c) {
  bigInteger ret = {0};
  // read sign
  if (*c == '-') ret.neg |= 1, c++;
  else ret.neg &= 0;
  // read digits
  word carry[3] = {0}, temp;
  while (*c) {
    carry[0] = *c - '0';
    if (carry[0] >= 10) break;
    darray_foreach(word, i, &ret) {
      carry[2] = *i & WORD_HALF_MASK;
      carry[1] = *i>> WORD_HALF_BITS;
      *i *= 10;
      carry[0] = (*i += carry[0]) < carry[0];
      carry[2] = (10 * carry[2]) >> WORD_HALF_BITS;
      temp = 10 * carry[1];
      carry[1] = (carry[2] += temp) < temp;
      carry[2] >>= WORD_HALF_BITS;
      carry[1] <<= WORD_HALF_BITS;
      carry[0] += carry[2];
      carry[0] += carry[1];
    }
    if (carry[0]) darray_append(&ret, carry[0]);
    c++;
  }
  return ret;
}
bigInteger bigInteger_from_int(const int in) {
  bigInteger ret = {.neg = (in < 0)};
  darray_append(&ret, (CAST(word)imath_iabs (in)));
  return ret;
}
void bigInteger_set_words(bigInteger *a, bool neg, const word *w, iter i) {
  darray_clean(a);
  darray_appends(a, w, i);
  bigInteger__shrink(a);
  a->neg = neg;
}
void bigInteger_set_cstr(bigInteger *a, const char *c) {
  darray_clean(a);
  // read sign
  if (*c == '-') a->neg |= 1, c++;
  else a->neg &= 0;
  // read digits
  word carry[3] = {0}, temp;
  while (*c) {
    carry[0] = *c - '0';
    if (carry[0] >= 10) break;
    darray_foreach(word, i, a) {
      carry[2] = *i & WORD_HALF_MASK;
      carry[1] = *i>> WORD_HALF_BITS;
      *i *= 10;
      carry[0] = (*i += carry[0]) < carry[0];
      carry[2] = (10 * carry[2]) >> WORD_HALF_BITS;
      temp = 10 * carry[1];
      carry[1] = (carry[2] += temp) < temp;
      carry[2] >>= WORD_HALF_BITS;
      carry[1] <<= WORD_HALF_BITS;
      carry[0] += carry[2];
      carry[0] += carry[1];
    }
    if (carry[0]) darray_append(a, carry[0]);
    c++;
  }
}
void bigInteger_set_int(bigInteger *a, const int in) {
  a->neg = (in < 0);
  darray_clean(a);
  darray_append(a, CAST(word)imath_iabs (in));
}
void bigInteger_set(bigInteger *a, const bigInteger b) {
  darray_copy(a,&b);
  a->neg = b.neg;
}
// helper
inline bigInteger bigInteger_dup(const bigInteger x) {
  bigInteger r = {.neg = x.neg};
  darray_copy(&r,&x);
  bigInteger__shrink(&r);
  return r;
}
void bigInteger_move(bigInteger *a, bigInteger *b) {
  darray_copy(a,b);
  a->neg = b->neg;
  bigInteger_free(b);
}
inline void bigInteger_zero(bigInteger *x) {
  darray_clean(x);
  x->neg = 0;
}
inline void bigInteger_free(bigInteger *x) {
  darray_free(x);
}
// compare 2 bigInteger, which 0 is equal, -1 left smaller, 1 left bigger  
int bigInteger_cmp(const bigInteger a, const bigInteger b) {
  int ret = b.neg - a.neg;
  if (!ret) ret = bigInteger__cmp(a, b);
  if (a.neg) ret *= -1;
  return ret;
}
// return division result, save reminder on nominator
void bigInteger_div_mod(const bigInteger a, const bigInteger b, bigInteger *res, bigInteger *REM) {
  word c, c1;
  iter i, j;
  bigInteger *rem;
  if (res) {
    bigInteger_zero(res);
    res->neg = a.neg ^ b.neg;
  }
	if (!b.count || bigInteger__cmp(a, b) < 0) {
		if (REM) bigInteger_set(REM, a);
	  return;
	} else if (REM) {
  	bigInteger_zero((rem = REM));
  } else {
  	rem = CAST(bigInteger*)util_calloc(1, sizeof(bigInteger));
  }
  i = b.count - 1;
  j = a.count - i;
	darray_appends(rem, a.items + j, i);
	while (j--) {
		for (i = WORD_BITS; i--;) {
	    c1 = (a.items[j] >> i) & 1;
	    darray_foreach(word, irem, rem) {
	      c = *irem;
	      *irem <<= 1;
	      *irem |= c1;
	      c1 = (c >> (WORD_BITS - 1)) & 1;
	    }
	    if (c1) darray_append(rem, c1);
	    c1 = (bigInteger__cmp(*rem, b) >= 0);
	    if (c1) bigInteger__wordsub(rem->items, rem->count, b.items, b.count);
	    if (!res) continue;
	    darray_foreach(word, ires, res) {
	      c = *ires;
	      *ires <<= 1;
	      *ires |= c1;
	      c1 = c >> (WORD_BITS - 1);
	    }
	    if (c1) darray_append(res, c1);
	  }
	}
  if (!REM) bigInteger_free(rem);
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
  const word w = CAST(word)imath_iabs(c);
  if (r.neg ^ (c < 0)) bigInteger__subtract(&r, w);
  else bigInteger__addition(&r, w);
  return r;
}
bigInteger bigInteger_subi(const bigInteger a, const int c) {
  bigInteger r = bigInteger_dup(a);
  const word w = CAST(word)imath_iabs(c);
  if (r.neg ^ (c < 0)) bigInteger__addition(&r, w);
  else bigInteger__subtract(&r, w);
  return r;
}
bigInteger bigInteger_muli(const bigInteger a, const int c) {
  bigInteger r = bigInteger_dup(a);
  bigInteger__multiply(&r, c);
  return r;
}
bigInteger bigInteger_divi(const bigInteger a, const int c) {
  bigInteger r = bigInteger_dup(a);
  bigInteger__division(&r, c);
  return r;
}
word bigInteger_modi(const bigInteger a, const int c) {
  bigInteger r = bigInteger_dup(a);
  word m = bigInteger__division(&r, c);
  bigInteger_free(&r);
  return m;
}
bigInteger bigInteger_powi(const bigInteger a, const uint b) {
  bigInteger r = bigInteger_dup(a);
  bigInteger_mpowi(&r,b);
  return r;
}
bigInteger bigInteger_shfli(const bigInteger a, const uint i) {
  bigInteger r = bigInteger_dup(a);
  bigInteger_mshfli(&r, i);
  return r;
}
bigInteger bigInteger_shfri(const bigInteger a, const uint i) {
  bigInteger r = bigInteger_dup(a);
  bigInteger_mshfri(&r, i);
  return r;
}
bigInteger bigInteger_pow2(const bigInteger a) {
  bigInteger c = {0};
  darray_atleast(&c, a.count * 2 + 1);
  word xhi,xlo,yhi,ylo,carry[4] = {0},temp;
  for (iter x = 0,y,z; x < a.count; ++x) {
    xhi = a.items[x]>> WORD_HALF_BITS;
    xlo = a.items[x] & WORD_HALF_MASK;
    for (y = 0; y < a.count; ++y) {
      yhi = a.items[y]>> WORD_HALF_BITS;
      ylo = a.items[y] & WORD_HALF_MASK;
      // muls
      temp = a.items[x] * a.items[y];
      // adds
      z = x + y;
      carry[0] = (c.items[z] += carry[0]) < carry[0];
      carry[0] += (c.items[z] += temp) < temp;
      carry[1] = (carry[0] += carry[1]) < carry[1];
      
      carry[2] = (xlo * ylo) >> WORD_HALF_BITS;
      temp = xlo * yhi;
      carry[3] = (carry[2] += temp) < temp;
      temp = xhi * ylo;
      carry[3] += (carry[2] += temp) < temp;
      carry[2] >>= WORD_HALF_BITS;
      carry[3] <<= WORD_HALF_BITS;
      
      carry[1] += (carry[0] += carry[2]) < carry[2];
      carry[1] += (carry[0] += carry[3]) < carry[3];
      
      temp = xhi * yhi;
      carry[1] += (carry[0] += temp) < temp;
    }
    for (z = x + y; carry[0] && (z < c.count); ++z) {
      carry[0] = (c.items[z] += carry[0]) < carry[0];
    }
    if (carry[0]) darray_append(&c, carry[0]);
    for (z = x + y + 1; carry[1] && (z < c.count); ++z) {
      carry[1] = (c.items[z] += carry[1]) < carry[1];
    }
    if (carry[1]) darray_append(&c, carry[1]);
  }
  c.neg &= 0;
  bigInteger__shrink(&c);
  return c;
}
bigInteger bigInteger_sqrt(const bigInteger a) {
  bigInteger res = {0};
  bigInteger rem = {0};
  word carry, carry1;
  iter i = 0;
  if (a.count) {
    i = util_bitlead(darray_last(&a));
    i += (i & 1);
    i += (a.count - 1) * WORD_BITS;
    darray_append(&res, 0);
  }
  while (i) { 
    i -= 2;
    // extract 2 binary from source A
    carry1 = (a.items[i / WORD_BITS] >> (i % WORD_BITS)) & 3;
    // remaining shift left 2 append source A
    darray_foreach(word, remi, &rem) {
      carry = *remi;
      *remi <<= 2;
      *remi |= carry1;
      carry1 = carry >> (WORD_BITS - 2);
    }
    if (carry1) darray_append(&rem, carry1);
    // result shift 2 left
    carry1 = 1;
    darray_foreach(word, resi, &res) {
      carry = *resi;
      *resi <<= 2;
      *resi |= carry1;
      carry1 = carry >> (WORD_BITS - 2);
    }
    if (carry1) darray_append(&res, carry1);
    // compare result test with remaining
    if (bigInteger__cmp (rem, res) >= 0) {
      carry = bigInteger__wordsub(rem.items, rem.count, res.items, res.count);
      ASSERT(!carry && "sqrt rem less than res!");
      res.items[0] |= 2;
    }
    // shift 1 right to get pure result
    carry1 = 0;
    darray_rforeach(word, resi, &res) {
      carry = *resi;
      *resi >>= 1;
      *resi |= carry1;
      carry1 = carry << (WORD_BITS - 1);
    }
  }
  bigInteger_free(&rem);
  bigInteger__shrink(&res);
  return res;
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
  bigInteger c = {0};
  darray_atleast(&c, a.count + b.count + 1);
  word xhi,xlo,yhi,ylo,carry[3] = {0},temp;
  for (iter x = 0,y,z; x < a.count; ++x) {
    xhi = a.items[x]>> WORD_HALF_BITS;
    xlo = a.items[x] & WORD_HALF_MASK;
    for (y = 0; y < b.count; ++y) {
      yhi = b.items[y]>> WORD_HALF_BITS;
      ylo = b.items[y] & WORD_HALF_MASK;
      z = x + y;
      carry[0] = (c.items[z] += carry[0]) < carry[0];
      temp = a.items[x] * b.items[y];
      carry[0]+= (c.items[z] += temp) < temp;
      carry[2] = (ylo * xlo) >> WORD_HALF_BITS;
      temp = ylo * xhi;
      carry[1] = (carry[2] += temp) < temp;
      temp = yhi * xlo;
      carry[1]+= (carry[2] += temp) < temp;
      carry[2]>>= WORD_HALF_BITS;
      carry[1]<<= WORD_HALF_BITS;
      
      carry[0] += carry[2];
      carry[0] += carry[1];
      temp = yhi * xhi;
      carry[0] += temp;
    }
    for (z = x + y; carry[0] && (z < c.count); ++z)
      carry[0] = (c.items[z] += carry[0]) < carry[0];
    if (carry[0]) darray_append(&c, carry[0]);
  }
  c.neg = a.neg ^ b.neg;
  bigInteger__shrink(&c);
  return c;
}
bigInteger bigInteger_muladd(const bigInteger a, const bigInteger b, const bigInteger c) {
	return bigInteger__MultiplyAdd(a,b,c);
}
bigInteger bigInteger_div(const bigInteger a, const bigInteger b) {
  bigInteger res = {0};
  bigInteger_div_mod(a, b, &res, NULL);
  return res;
}
bigInteger bigInteger_mod(const bigInteger a, const bigInteger b) {
  bigInteger rem = {0};
  bigInteger_div_mod(a, b, NULL, &rem);
  return rem;
}
// modification operate no error should be occure
void bigInteger_mredc(bigInteger *a) {
  // just do a single bit
  word *i = a->items, *j = i + a->count;
  word carry = 1;
  if (a->neg) { // do addition
    while ((i < j) && (carry = !(++(*(i++))))) ;
    if (carry) darray_append (a, 1);
  } else { // do subtraction
    while ((i < j) && (carry = !(*(i++))--)) ;
    if (carry) darray_append (a, 1);
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
    if (carry) darray_append (a, 1);
    a->neg &= 0;
    bigInteger__shrink(a);
  } else { // do addition
    while ((i < j) && (carry = !(++(*(i++))))) ;
    if (carry) darray_append (a, 1);
  }
}
void bigInteger_maddi(bigInteger *a, const int c) {
  word w = CAST(word)imath_iabs(c);
  if (a->neg ^ (c < 0)) bigInteger__subtract(a, w);
  else bigInteger__addition(a, w);
}
void bigInteger_msubi(bigInteger *a, const int c) {
  word w = CAST(word)imath_iabs(c);
  if (a->neg ^ (c < 0)) bigInteger__addition(a, w);
  else bigInteger__subtract(a, w);
}
void bigInteger_mmuli(bigInteger *a, const int c) {
  bigInteger__multiply(a, c);
} 
void bigInteger_mdivi(bigInteger *a, const int c) {
  bigInteger__division(a, c);
}
void bigInteger_mmodi(bigInteger *a, const int c) {
  word r = bigInteger__division(a, c);
  bigInteger_set_int(a, CAST(int)r * -1 * (c < 0));
}
void bigInteger_mpowi(bigInteger *a, uint b) {
  bigInteger r = bigInteger_from_int(1);
  while (b) {
    if (b & 1) bigInteger_mmul(&r, *a);
    bigInteger_mpow2(a);
    b >>= 1;
  }
  bigInteger_move(a, &r);
}
void bigInteger_mshfli(bigInteger *a, const uint i) {
  iter bit_shift = i % WORD_BITS;
  if (bit_shift) {
    word carry = 0, c;
    darray_foreach(word, ia, a) {
      c = *ia;
      *ia <<= bit_shift;
      *ia |= carry;
      carry = c >> (WORD_BITS - bit_shift);
    }
    if (carry) darray_append(a, carry);
  }
  iter word_shift = i / WORD_BITS;
  if (word_shift) {
    darray_reserve(a, a->count + word_shift);
    util_memmove(a->items + word_shift, a->items, a->count * sizeof(word));
    a->count += word_shift;
  }
}
void bigInteger_mshfri(bigInteger *a, const uint i) {
  iter word_shift = i / WORD_BITS;
  if (word_shift) {
    a->count -= word_shift;
    util_memcpy(a->items, a->items + word_shift, a->count * sizeof(word));
  }
  iter bit_shift = i % WORD_BITS;
  if (bit_shift) {
    word carry = 0, c;
    darray_rforeach(word, ia, a) {
      c = *ia;
      *ia >>= bit_shift;
      *ia |= carry;
      carry = c << (WORD_BITS - bit_shift);
    }
    bigInteger__shrink(a);
  }
}
void bigInteger_mpow2(bigInteger *a) {
  bigInteger r = bigInteger_pow2(*a);
  bigInteger_move(a, &r);
}
void bigInteger_msqrt(bigInteger *a) {
  bigInteger r = bigInteger_sqrt(*a);
  bigInteger_move(a, &r);
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
  bigInteger r = bigInteger_mul(*a, b);
  bigInteger_move(a, &r);
}
void bigInteger_mmuladd(bigInteger *a, const bigInteger b, const bigInteger c) {
  bigInteger r = bigInteger_muladd(*a, b, c);
  bigInteger_move(a, &r);
}
void bigInteger_mdiv(bigInteger *a, const bigInteger b) {
  bigInteger res = {0};
  bigInteger_div_mod(*a, b, &res, NULL);
  bigInteger_move(a, &res);
}
void bigInteger_mmod(bigInteger *a, const bigInteger b) {
  bigInteger rem = {0};
  bigInteger_div_mod(*a, b, NULL, &rem);
  bigInteger_move(a, &rem);
}

// print out
void bigInteger_append_dstring(dstring *str, const bigInteger a) {
  word rmr, current;
  iter ac = a.count;
  if (!ac) {
    dstring_append_char(str, '0');
    return;
  }
  iter dstring_old = dstring_len(*str);
  dstring_reserve(str, dstring_old + ac * 10);
  iter bytes = WORD_BYTES * ac;
  word *aw = CAST(word*)util_malloc(bytes);
  util_memcpy(aw, a.items, bytes);
  do {
    rmr = 0;
    for(word *i = aw + ac; (i--) > aw; ){
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
    dstring_append_char(str, '0' + CAST(char)rmr);
    ac -= (ac && !aw[ac - 1]);
  } while (ac);
  util_memfree(aw);
  if (a.neg) dstring_append_char(str, '-');
  util_memflip(*str + dstring_old, dstring_len(*str) - dstring_old);
  // dstring_append_char(str, 0);
}
