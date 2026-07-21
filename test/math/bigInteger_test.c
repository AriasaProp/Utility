#include "math/bigInteger.h"
#include "util/console_out.h"
#include "common.h"

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
#define COMMON_TEST 6080
#define MAX_RNDI 16
#define RAND_S  CAST(bool)(imath_rand_ubyte()&1)
#define RAND_C  CAST(iter)((imath_rand_uint() % MAX_RNDI) + 1)
#define RAND_W  CAST(word)(imath_rand_uint() + imath_rand_uint())
#define RAND_I  imath_rand_int()
  int result = EXIT_FAILURE, oprB;
  bigInteger state[5] = {0};
  dstring qstr = NULL;
  iter nword, i, cnt = 0;
  word rndT[MAX_RNDI + 1];
  PRINT_INF("BigInteger Test! ");
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
	      PRINT_ERR("Setter cstr: \n should %s but get %s \n", bstr, qstr);
	      goto end; 
	    }
	  }
	  dstring_free(&bstr);
  }
#define CASE(A,B) do {\
  for (cnt = 0; cnt < COMMON_TEST; ++cnt) {\
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ;\
    bigInteger_set_words(state, !!(RAND_S && nword), rndT, nword);\
    oprB = RAND_I;\
    state[4] = bigInteger_##A##i(state[0], oprB);\
    bigInteger_move(state + 1, state + 4);\
    state[4] = bigInteger_##B##i(state[1], oprB);\
    bigInteger_move(state + 2, state + 4);\
    if (bigInteger_cmp(state[0],state[2])) {\
      printf(RED"i[%zu] "#A"_"#B RESET "\n", cnt);\
      printf("i: %50d\n", oprB); \
      for (i = 0; i < 3; ++i) {\
        dstring_clean(qstr);\
        bigInteger_append_dstring(&qstr, state[i]);\
        printf("%zu: %50s\n", i, qstr);\
      }\
      goto end;\
    }\
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ;\
    bigInteger_set_words(state + 0, false, rndT, nword); \
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ; \
    bigInteger_set_words(state + 1, false, rndT, nword); \
    state[4] = bigInteger_##A (state[0], state[1]); \
    bigInteger_move(state + 2, state + 4); \
    state[4] = bigInteger_##B (state[2], state[1]); \
    bigInteger_move(state + 3, state + 4); \
    if (bigInteger_cmp(state[0],state[3])) { \
      printf(RED"big[%zu] "#A"_"#B RESET"\n", cnt); \
      for (i = 0; i < 4; ++i) {\
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
#define CASEW(A,B) do {\
  for (cnt = 0; cnt < COMMON_TEST; ++cnt) {\
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ;\
    bigInteger_set_words(state + 0, false, rndT, nword); \
    state[3] = bigInteger_##A (state[0]); \
    bigInteger_move(state + 1, state + 3); \
    state[3] = bigInteger_##B (state[1]); \
    bigInteger_move(state + 2, state + 3); \
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
  CASEW(pow2,sqrt);
#undef CASEW
#undef CASE
#undef COMMON_TEST
#undef MAX_RNDI
  result = EXIT_SUCCESS;
  printf("Done!\n");
end:
  for (i = 0; i < 5; ++i)
    bigInteger_free(state + i);
  dstring_free(&qstr);
  return result;
}
