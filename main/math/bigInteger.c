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
/*
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
*/
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
  if (!a->count) a->neg = false;
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
static void bigInteger__abit(bigInteger *a, bool sub) {
  // just do a single bit
  word *i = a->items, *j = i + a->count;
  const bool sgn = a->neg ^ sub;
  bool carry = true;
  while (i < j) if (
  	( sgn && !(carry = !((*(i++))--))) ||
  	(!sgn && !(carry = !(++(*(i++)))))
  ) break;
  if (carry) {
  	darray_append (a, carry);
  	a->neg ^= sgn;
  }
  bigInteger__shrink(a);
}
/*
 * a + b   =>      a    - b
 * a + b   => A - (~a + 1) - b
 * a + b   => -(~a + 1) - b  + A
 * a + b   =>-((~a + 1) + b) + A
 * a + b   =>     -c         + A
 * a + b   =>-(A - (~c + 1)  + A
 * a + b   => ~c + 1
 *
 *
 */
static void bigInteger__sumi(bigInteger *a, const int B, const bool sub) {
	darray_atleast(a, 1);
	word b = CAST(word)imath_iabs(B);
	iter i;
	bool borrow = a->neg ^ (B < 0) ^ sub, c;
	if (borrow)
  	for (i = 0, c = true; i < a->count; ++i)
      c &= !(a->items[i] = ~a->items[i] + c);
  for (i = 0; b && (i < a->count); ++i)
    b = (a->items[i] += b) < b;
  if (borrow) {
  	if (b) a->neg ^= true;
  	else for (i = 0, c = true; i < a->count; ++i)
	  	c &= !(a->items[i] = ~a->items[i] + c);
  } else {
  	if (b) darray_append(a, b);
	  else bigInteger__shrink(a);
  }
}
static void bigInteger__Sum (bigInteger *a, const bigInteger b, const bool sub) {
  darray_atleast(a, MAX(1, b.count));
  iter i;
	bool borrow = a->neg ^ b.neg ^ sub, c;
	if (borrow)
  	for (i = 0, c = true; i < a->count; ++i)
      c &= !(a->items[i] = ~a->items[i] + c);
  i = 0;
  while (i < b.count) {
    c&=!(a->items[i] += c);
    c|= (a->items[i] += b.items[i]) < b.items[i];
    ++i;
  }
  while (c && (i < a->count))
    c&=!(a->items[i++] += c);

  if (c) {
  	if (borrow) a->neg ^= true;
  	else darray_append(a, c);
  } else {
  	if (borrow) for (i = 0, c = true; i < a->count; ++i)
	  	c&=!(a->items[i] = ~a->items[i] + c);
	  else bigInteger__shrink(a);
  }
}

/*
 *  a*b - d
 *  a*b - (B - D)
 *  a*b - B + D
 * Borrow
 * Degate
 *
 */
static bigInteger bigInteger__MultiplySum(const bigInteger a, const bigInteger b,const bigInteger d, const bool sub) {
  bigInteger c = bigInteger_dup(d);
  darray_atleast(&c, a.count + b.count);
  c.neg = a.neg ^ b.neg;
  iter x,y,z;
  bool borrow = d.neg ^ c.neg ^ sub;
  word xhi,xlo,yhi,ylo,carry[3] = {0},temp;
  if (borrow) // borrow
  	for (x = 0, carry[0] = 1; x < c.count; ++x)
  		carry[0]&=!(c.items[x] = ~c.items[x] + carry[0]);
  for (x = 0; x < a.count; ++x) {
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
  if (borrow) for (x = 0,carry[0] = 1; x < c.count; ++x)
    carry[0]&=!(c.items[x] = ~c.items[x] + carry[0]);
	c.neg ^= borrow;
  bigInteger__shrink(&c);
  return c;
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
  x->neg = false;
}
inline void bigInteger_free(bigInteger *x) {
  darray_free(x);
}
// compare 2 bigInteger, which 0 is equal, -1 left smaller, 1 left bigger  
int bigInteger_cmp(const bigInteger a, const bigInteger b) {
  int ret = b.neg - a.neg;
  if (!ret) ret = bigInteger__cmp(a, b) * (1 - 2 * a.neg);
  return ret;
}
static bool word__isPrime (word a) {
	if (a < 2) return false;
	if (a == 2) return true;
	if (!(a & 1)) return false;
	word end = (a >> (util_bitlead(a) / 2)) & 1; // need approx sqrt integer
	for (word i = 3; i < end; i += 2) {
		if (a == i) return true;
		if (!(a % i)) return false;
	}
	return true;
}
void bigInteger_property (const bigInteger a, int *p) {
	if (!p || !*p) return;
	int res = 0;
	if (a.count) {
		// is odd
		if (*p & (BigInteger_Odd | BigInteger_Prime)) {
			res |= BigInteger_Odd * (a.items[0] & 1);
		}
		// is prime
		if (*p & BigInteger_Prime) {
			if (a.count == 1) {
				res |= BigInteger_Prime * word__isPrime(a.items[0]);
			} else if (res & BigInteger_Odd) { // filter even number
				bigInteger end = bigInteger_sqrt(a);
				end.items[0] |= 1;
				bigInteger curr = bigInteger_from_int(3), mod = {0};
				while (bigInteger_cmp(end, curr) > 0) {
					bigInteger_set (&mod, a);
					bigInteger_mmod(&mod, curr);
					bigInteger__shrink(&mod);
					if (!mod.count) break;
					bigInteger_maddi(&curr, 2);
				}
				res |= (mod.count != 0) * BigInteger_Prime;
				bigInteger_free(&end);
				bigInteger_free(&mod);
				bigInteger_free(&curr);
			}
		}
	}
	*p = res;
}
// return division result, save reminder on nominator
void bigInteger_div_mod(bigInteger *a, const bigInteger b, bigInteger *rem) {
	bigInteger__shrink(a);
  word c, c1, d;
  iter i, j, k;
  iter blen = b.count;
  while (blen && !b.items[blen-1]) --blen;
	bigInteger_zero(rem);
	i = MIN(a->count, MAX(1, blen) - 1);
	j = a->count - i;
	darray_appends(rem, a->items + j, i);
	a->count = j;
	while (j--) {
		d = a->items[j];
		for (i = WORD_BITS; i--;) {
	    c1 = (d >> i) & 1;
	    for (k = 0; k < rem->count; ++k) {
	      c = rem->items[k];
	      rem->items[k] <<= 1;
	      rem->items[k] |= c1;
	      c1 = (c >> (WORD_BITS - 1)) & 1;
	    }
	    if (c1) darray_append(rem, c1);
	    a->items[j] <<= 1;
	    /*
	    if (rem->count < blen || (rem->count == blen && bigInteger__cmpa(rem->items, b.items, blen) < 0)) continue;
	    c = bigInteger__wordsub(rem->items, rem->count, b.items, blen);
	    */
	    if (rem->count < blen) continue;
	    if (rem->count == blen) {
			  for (k = blen; k-- && (rem->items[k] == b.items[k]); ) ;
			  if (k < blen && rem->items[k] < b.items[k]) continue;
	    }
	    c = 0, k = 0;
		  while (k < blen) {
		    c1 = rem->items[k];
		    c = (rem->items[k] -= c) > c1;
		    c1 = rem->items[k];
		    c+= (rem->items[k] -= b.items[k]) > c1;
		    ++k;
		  }
		  while (c && (k < rem->count)) {
		    c1 = rem->items[k];
		    c = (rem->items[k] -= c) > c1;
		    ++k;
		  }
	    bigInteger__shrink(rem);
		  ASSERT(!c && "bigInteger div_mod: subtract result borrow");
	    a->items[j] |= 1;
	  }
	}
	a->neg ^= b.neg;
	bigInteger__shrink(a);
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
  bigInteger_maddi(&r, c);
  return r;
}
bigInteger bigInteger_subi(const bigInteger a, const int c) {
  bigInteger r = bigInteger_dup(a);
  bigInteger_msubi(&r, c);
  return r;
}
bigInteger bigInteger_muli(const bigInteger a, const int c) {
  bigInteger r = bigInteger_dup(a);
  bigInteger_mmuli(&r, c);
  return r;
}
bigInteger bigInteger_divi(const bigInteger a, const int c) {
  bigInteger r = bigInteger_dup(a);
  bigInteger_mdivi(&r, c);
  return r;
}
word bigInteger_modi(const bigInteger a, const int B) {
	const word b = CAST(word)imath_iabs(B);
	word rem[2] = {0};
  for (iter j = a.count, i; j--; ) {
    for (i = WORD_BITS; i--; ) {
      rem[1] <<= 1;
      rem[1] |= rem[0] >> (WORD_BITS - 1);
      rem[0] <<= 1;
      rem[0] |= (a.items[j] >> i) & 1;
      if (!rem[1] && rem[0] < b) continue;
      rem[1] -= rem[0] < b;
      rem[0] -= b;
    }
  }
  ASSERT(!rem[1] && "bigInteger, word modulo overflow");
  return rem[0];
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
  darray_atleast(&c, a.count * 2);
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
  c.neg = false;
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
  bigInteger_madd(&r, b);
  return r;
}
bigInteger bigInteger_sub(const bigInteger a, const bigInteger b) {
  bigInteger r = bigInteger_dup(a);
  bigInteger_msub(&r, b);
  return r;
}
bigInteger bigInteger_mul(const bigInteger a, const bigInteger b) {
  bigInteger c = {0};
  darray_atleast(&c, a.count + b.count);
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
	return bigInteger__MultiplySum(a,b,c, false);
}
bigInteger bigInteger_mulsub(const bigInteger a, const bigInteger b, const bigInteger c) {
	return bigInteger__MultiplySum(a,b,c, true);
}
bigInteger bigInteger_div(const bigInteger a, const bigInteger b) {
  bigInteger res = bigInteger_dup(a);
  bigInteger_mdiv(&res, b);
  return res;
}
bigInteger bigInteger_mod(const bigInteger a, const bigInteger b) {
  word c, c1, d;
  iter i, j, k;
  iter blen = b.count, alen = a.count;
  while (alen && !a.items[alen-1]) --alen;
  while (blen && !b.items[blen-1]) --blen;
  bigInteger rem = {0};
	i = MIN(alen, MAX(1, blen) - 1);
	j = alen - i;
	darray_appends(&rem, a.items + j, i);
	while (j--) {
		d = a.items[j];
		for (i = WORD_BITS; i--;) {
	    c1 = (d >> i) & 1;
	    for (k = 0; k < rem.count; ++k) {
	      c = rem.items[k];
	      rem.items[k] <<= 1;
	      rem.items[k] |= c1;
	      c1 = (c >> (WORD_BITS - 1)) & 1;
	    }
	    if (c1) darray_append(&rem, c1);
	    /*
	    if (rem.count < blen || (rem.count == blen && bigInteger__cmpa(rem.items, b.items, blen) < 0)) continue;
	    c = bigInteger__wordsub(rem.items, rem.count, b.items, blen);
	    */
	    if (rem.count < blen) continue;
	    if (rem.count == blen) {
			  for (k = blen; k-- && (rem.items[k] == b.items[k]); ) ;
			  if (k < blen && rem.items[k] < b.items[k]) continue;
	    }
	    c = 0, k = 0;
		  while (k < blen) {
		    c1 = rem.items[k];
		    c = (rem.items[k] -= c) > c1;
		    c1 = rem.items[k];
		    c+= (rem.items[k] -= b.items[k]) > c1;
		    ++k;
		  }
		  while (c && (k < rem.count)) {
		    c1 = rem.items[k];
		    c = (rem.items[k] -= c) > c1;
		    ++k;
		  }
	    bigInteger__shrink(&rem);
		  ASSERT(!c && "bigInteger div_mod: subtract result borrow");
	  }
	}
	return rem;
}
/* n! = 1 * 2 * 3 * 4 * ..... * n
 * not yet
 * n! = 1 * 3 * 5 * 7 * ..... * last odd n
 *      2 * 4 * 6 * 8 * ..... * last even n
 * n! = 1 * 3 * 5 * 7 * ..... * 
 *      1 * 3 * 5 * 7 * ..... * (last odd n /2)  * 2**(n/2)
 *      1 * 3 * 5 * 7 * ..... * (last odd n /4)  * 2**(n/4)
 *      1 * 3 * 5 * 7 * ..... * (last odd n /8)  * 2**(n/8)
 *      1 * 3 * 5 * 7 * ..... * (last odd n /16)  * 2**(n/16)
 *      1 * 2 * 3 * 4 * ..... * (last odd n /4)  * 2**(n/4)
 *
 *
 *
 */
bigInteger bigInteger_factorial(uint b) {
	bigInteger ret = bigInteger_from_int(1);
  iter shf = 0, j;
	word i, yhi, ylo, xhi, xlo, carry[3], temp;
	while (b) {
	  for (i = 3; i <= b; i += 2) {
	    yhi = i >> WORD_HALF_BITS;
	    ylo = i  & WORD_HALF_MASK;
	    util_memset(carry, 0, 3 * WORD_BYTES);
	    for (j = 0; j < ret.count; ++j) {
	      xhi = ret.items[j] >> WORD_HALF_BITS;
	      xlo = ret.items[j] &  WORD_HALF_MASK;
	      ret.items[j] *= i;
	      carry[0] = (ret.items[j] += carry[0]) < carry[0];
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
	    if (carry[0]) darray_append(&ret, carry[0]);
    }
    b >>= 1;
    shf += b;
	}
  bigInteger_mshfli(&ret, shf);
  bigInteger__shrink(&ret);
	return ret;
}
// modification operate no error should be occure
void bigInteger_mredc(bigInteger *a) {
  bigInteger__abit(a, true);
}
void bigInteger_mincr(bigInteger *a) {
  bigInteger__abit(a, false);
}
void bigInteger_maddi(bigInteger *a, const int c) {
  bigInteger__sumi(a, c, false);
}
void bigInteger_msubi(bigInteger *a, const int c) {
  bigInteger__sumi(a, c, true);
}
void bigInteger_mmuli(bigInteger *a, const int B) {
  a->neg ^= B < 0;
	const word b = CAST(word)imath_iabs(B);
  const word yhi = b >> WORD_HALF_BITS, ylo = b & WORD_HALF_MASK;
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
void bigInteger_mdivi(bigInteger *a, const int B) {
  const word b = CAST(word)imath_iabs(B);
  word rem[2] = {0}, c;
  for (iter j = a->count, i; j--; ) {
  	c = a->items[j];
    for (i = WORD_BITS; i--; ) {
      rem[1] <<= 1;
      rem[1] |= rem[0] >> (WORD_BITS - 1);
      rem[0] <<= 1;
      rem[0] |= (c >> i) & 1;
      a->items[j] <<= 1;
      if (!rem[1] && rem[0] < b) continue;
      rem[1] -= rem[0] < b;
      rem[0] -= b;
      a->items[j] |= 1;
    }
  }
  a->neg ^= B < 0;
  ASSERT(!rem[1] && "bigInteger, word division overflow");
  bigInteger__shrink(a);
}
void bigInteger_mmodi(bigInteger *a, const int c) {
  word r = bigInteger_modi(*a, c);
  bigInteger_set_words(a, a->neg ^ (c < 0), &r, 1);
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
	bigInteger__Sum(a, b, false);
}
void bigInteger_msub(bigInteger *a, const bigInteger b) {
	bigInteger__Sum(a, b, true);
}
void bigInteger_mmul(bigInteger *a, const bigInteger b) {
  bigInteger r = bigInteger_mul(*a, b);
  bigInteger_move(a, &r);
}
void bigInteger_mmuladd(bigInteger *a, const bigInteger b, const bigInteger c) {
  bigInteger r = bigInteger__MultiplySum(*a, b, c, false);
  bigInteger_move(a, &r);
}
void bigInteger_mmulsub(bigInteger *a, const bigInteger b, const bigInteger c) {
  bigInteger r = bigInteger__MultiplySum(*a, b, c, true);
  bigInteger_move(a, &r);
}
void bigInteger_mdiv(bigInteger *a, const bigInteger b) {
  word c, c1, d;
  iter i, j, k;
  iter blen = b.count;
  while (blen && !b.items[blen-1]) --blen;
	bigInteger__shrink(a);
  bigInteger rem = {0};
	i = MIN(a->count, MAX(1, blen) - 1);
	j = a->count - i;
	darray_appends(&rem, a->items + j, i);
	a->count = j;
	while (j--) {
		d = a->items[j];
		for (i = WORD_BITS; i--;) {
	    c1 = (d >> i) & 1;
	    for (k = 0; k < rem.count; ++k) {
	      c = rem.items[k];
	      rem.items[k] <<= 1;
	      rem.items[k] |= c1;
	      c1 = (c >> (WORD_BITS - 1)) & 1;
	    }
	    if (c1) darray_append(&rem, c1);
	    a->items[j] <<= 1;
	    if (rem.count < blen) continue;
	    if (rem.count == blen) {
			  for (k = blen; k-- && (rem.items[k] == b.items[k]); ) ;
			  if (k < blen && rem.items[k] < b.items[k]) continue;
	    }
	    // c = bigInteger__wordsub(rem.items, rem.count, b.items, blen);
	    c = 0, k = 0;
		  while (k < blen) {
		    c1 = rem.items[k];
		    c = (rem.items[k] -= c) > c1;
		    c1 = rem.items[k];
		    c+= (rem.items[k] -= b.items[k]) > c1;
		    ++k;
		  }
		  while (c && (k < rem.count)) {
		    c1 = rem.items[k];
		    c = (rem.items[k] -= c) > c1;
		    ++k;
		  }
		  bigInteger__shrink(&rem);
		  ASSERT(!c && "bigInteger div_mod: subtract result borrow");
	    a->items[j] |= 1;
	  }
	}
	a->neg ^= b.neg;
	bigInteger__shrink(a);
	bigInteger_free(&rem);
}
void bigInteger_mmod(bigInteger *a, const bigInteger b) {
  bigInteger rem = bigInteger_mod(*a, b);
  bigInteger_move(a, &rem);
}

// print out
void bigInteger_append_dstring(dstring *str, const bigInteger a) {
  iter ac = a.count, bytes = WORD_BYTES * ac, i;
  word rmr, current, *aw = CAST(word*)util_malloc(bytes);
  iter dstring_old = dstring_len(*str);
  // predict each word decimal, log10(2) ~> 1/3
  dstring_reserve(str, dstring_old + 1 + ac * WORD_BITS / 3);
  util_memcpy(aw, a.items, bytes);
  do {
    rmr = 0;
    for(i = ac; i--; ) {
      current = aw[i];
      rmr <<= WORD_HALF_BITS;
      rmr |= current >> WORD_HALF_BITS;
      aw[i] = rmr / 10;
      rmr %= 10;
      rmr <<= WORD_HALF_BITS;
      rmr |= current & WORD_HALF_MASK;
      aw[i] <<= WORD_HALF_BITS;
      aw[i] |= rmr / 10;
      rmr %= 10;
    }
    dstring_append_char(str, '0' + CAST(char)rmr);
    ac -= (ac && !aw[ac - 1]);
  } while (ac);
  util_memfree(aw);
  if (a.neg) dstring_append_char(str, '-');
  util_memflip(*str + dstring_old, dstring_len(*str) - dstring_old);
}
