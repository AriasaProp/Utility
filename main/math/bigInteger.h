/* *****************************************************************************
 * bigInteger.h v0.0.0000
 * 
 * object that stored integer as dynamic array
 * 
 * 
 * 
 * *****************************************************************************/
#ifndef _BIGINTEGER_MATH_INCLUDED_
#define _BIGINTEGER_MATH_INCLUDED_

#include "common.h"

typedef uint word;

typedef struct {
  bool neg;
  word *items;
  ushrt cap, count;
} bigInteger;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// initialization
bigInteger bigInteger_from_cstr(const char*);
bigInteger bigInteger_from_int(const int);
void bigInteger_set_words(bigInteger*, bool, const word*, iter);
void bigInteger_set_cstr(bigInteger*, const char*);
void bigInteger_set_int(bigInteger*, const int);
void bigInteger_set(bigInteger*, const bigInteger);

// helper
bigInteger bigInteger_dup (const bigInteger);
void bigInteger_move(bigInteger*,bigInteger*);
void bigInteger_zero(bigInteger*);
void bigInteger_free(bigInteger*);
// compare 2 bigInteger, which 0 is equal, -1 left smaller, 1 left bigger  
int bigInteger_cmp (const bigInteger, const bigInteger);
// return division result, save reminder on nominator
void bigInteger_div_mod(const bigInteger, const bigInteger, bigInteger*, bigInteger*);

// duplicate operate
bigInteger bigInteger_redc (const bigInteger);
bigInteger bigInteger_incr (const bigInteger);
bigInteger bigInteger_addi (const bigInteger, const int);
bigInteger bigInteger_subi (const bigInteger, const int);
bigInteger bigInteger_muli (const bigInteger, const int);
bigInteger bigInteger_divi (const bigInteger, const int);
word       bigInteger_modi (const bigInteger, const int);
bigInteger bigInteger_powi (const bigInteger, const uint);
bigInteger bigInteger_shfli(const bigInteger, const uint);
bigInteger bigInteger_shfri(const bigInteger, const uint);
bigInteger bigInteger_pow2 (const bigInteger);
bigInteger bigInteger_sqrt (const bigInteger);
bigInteger bigInteger_add  (const bigInteger, const bigInteger);
bigInteger bigInteger_sub  (const bigInteger, const bigInteger);
bigInteger bigInteger_mul  (const bigInteger, const bigInteger);
bigInteger bigInteger_div  (const bigInteger, const bigInteger);
bigInteger bigInteger_mod  (const bigInteger, const bigInteger);
// modification operate no error should be occure
void bigInteger_mredc (bigInteger*);
void bigInteger_mincr (bigInteger*);
void bigInteger_maddi (bigInteger*, const int);
void bigInteger_msubi (bigInteger*, const int);
void bigInteger_mmuli (bigInteger*, const int);
void bigInteger_mdivi (bigInteger*, const int);
void bigInteger_mmodi (bigInteger*, const uint);
void bigInteger_mpowi (bigInteger*, const uint);
void bigInteger_mshfli(bigInteger*, const uint);
void bigInteger_mshfri(bigInteger*, const uint);
void bigInteger_mpow2 (bigInteger*);
void bigInteger_msqrt (bigInteger*);
void bigInteger_madd  (bigInteger*, const bigInteger);
void bigInteger_msub  (bigInteger*, const bigInteger);
void bigInteger_mmul  (bigInteger*, const bigInteger);
void bigInteger_mdiv  (bigInteger*, const bigInteger);
void bigInteger_mmod  (bigInteger*, const bigInteger);
// print out
void bigInteger_append_string(String*, const bigInteger);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _BIGINTEGER_MATH_INCLUDED_
