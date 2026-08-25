#include "math/bigInteger.h"
#include "util/console_out.h"
#include "common.h"

#define CACHE 4

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  PRINT_INF("BigInteger Test is ");
#define COMMON_TEST 179
#define MAX_RNDI 16
#define RAND_S  CAST(bool)(imath_rand_ubyte()&1)
#define RAND_C  CAST(iter)((imath_rand_uint() % MAX_RNDI) + 1)
#define RAND_W  CAST(word)(imath_rand_uint() + imath_rand_uint())
#define RAND_I  imath_rand_int()
  int result = EXIT_FAILURE, oprB;
  bigInteger state[CACHE] = {0};
  dstring qstr = NULL;
  iter nword, i, cnt = 0;
  word rndT[MAX_RNDI + 1];
  {
  	dstring bstr = NULL;
	  for (cnt = 0; cnt < COMMON_TEST; ++cnt) {
	    dstring_clean(bstr);
	    for (i = RAND_C; i--; )
	    	dstring_append(&bstr, "%zu", RAND_W);
	    bigInteger_set_cstr(state, bstr);
	    dstring_clean(qstr);
	    bigInteger_append_dstring(&qstr, state[0]);
	    if (!dstring_equal(bstr, qstr)) {
	      printf("Setter cstr:\n %s(%zu) != %s(%zu) \n", bstr, dstring_len(bstr), qstr, dstring_len(qstr));
	      goto end; 
	    }
	  }
	  dstring_free(&bstr);
  }
#define CASE(A,B) do {\
  for (cnt = 0; cnt < COMMON_TEST; ++cnt) {\
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ;\
    bigInteger_set_words(state, RAND_S, rndT, nword);\
    bigInteger_set(state + 1, state[0]); \
    oprB = RAND_I;\
    bigInteger_m##A##i(state + 1, oprB);\
    bigInteger_m##B##i(state + 1, oprB);\
    if (bigInteger_cmp(state[0],state[1])) {\
      printf(RED"i[%zu] "#A"_"#B RESET "\n", cnt);\
      printf("i: %50d\n", oprB); \
      for (i = 0; i < 2; ++i) {\
        dstring_clean(qstr);\
        bigInteger_append_dstring(&qstr, state[i]);\
        printf("%zu: %50s\n", i, qstr);\
      }\
      goto end;\
    }\
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ;\
    bigInteger_set_words(state + 0, RAND_S, rndT, nword); \
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ; \
    bigInteger_set_words(state + 1, RAND_S, rndT, nword); \
    bigInteger_set  (state + 2, state[0]); \
    bigInteger_m##A (state + 2, state[1]); \
    bigInteger_m##B (state + 2, state[1]); \
    if (bigInteger_cmp(state[0],state[2])) { \
      printf(RED"big[%zu] "#A"_"#B RESET"\n", cnt); \
      for (i = 0; i < 3; ++i) {\
        dstring_clean(qstr);\
        bigInteger_append_dstring(&qstr, state[i]);\
        printf("%zu: %50s\n", i, qstr);\
      }\
      goto end; \
    }\
  }\
} while(0)
  CASE(add,sub);
  CASE(sub,add);
  CASE(mul,div);
  
  for (cnt = 0; cnt < COMMON_TEST; ++cnt) {
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ;
    bigInteger_set_words(state + 0, false, rndT, nword);
    bigInteger_set  (state + 1, state[0]);
    bigInteger_mpow2(state + 1);
    bigInteger_msqrt(state + 1);
    if (bigInteger_cmp(state[0],state[1])) {
      printf(RED"big[%zu] pow2_sqrt" RESET"\n", cnt);
      for (i = 0; i < 2; ++i) {
        dstring_clean(qstr);
        bigInteger_append_dstring(&qstr, state[i]);
        printf("%zu: %50s\n", i, qstr);
      }
      goto end;
    }
  }
#define CASEM(A,B) do {\
  for (cnt = 0; cnt < COMMON_TEST; ++cnt) { \
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ; \
    bigInteger_set_words(state + 0, false, rndT, nword); \
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ; \
    bigInteger_set_words(state + 1, false, rndT, nword); \
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ; \
    bigInteger_set_words(state + 2, false, rndT, nword); \
    bigInteger_set(state + 3, state[0]); \
    bigInteger_mmul##A (state + 3, state[1], state[2]); \
    bigInteger_m##B (state + 3, state[2]); \
    bigInteger_mdiv (state + 3, state[1]); \
    if (bigInteger_cmp(state[0],state[3])) { \
      printf(RED"big[%zu] mul" #A RESET"\n", cnt); \
      for (i = 0; i < 4; ++i) { \
        dstring_clean(qstr); \
        bigInteger_append_dstring(&qstr, state[i]); \
        printf("%zu: %50s\n", i, qstr); \
      } \
      goto end; \
    } \
    bigInteger_free(state + 3); \
  } \
} while (0)
  CASEM(add, sub);
  CASEM(sub, add);
  
#undef CASEM
#undef CASE
#undef COMMON_TEST
#undef MAX_RNDI
  result = EXIT_SUCCESS;
  printf(GREEN"Success!\n"RESET);
end:
  for (i = 0; i < CACHE; ++i)
    bigInteger_free(state + i);
  dstring_free(&qstr);
  return result;
}
