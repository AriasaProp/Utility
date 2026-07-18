#include "math/bigInteger.h"
#include "util/console_out.h"
#include "util/profiling.h"
#include "common.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

// duration limit each test
#define TIME 3.1
// undef UPDATE_RATE for info
#define UPDATE_RATE 0.45

// static const iter BIG_TENS = sizeof(word) * 8;

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  bigInteger state[8] = {0};
  int istate[8] = {0}, fd, result = EXIT_FAILURE;
  dstring qstr = NULL;
  bool loop, errflag = false;
  iter i, cnt, max_read, current_read;
#define REPORT_STR 32
#define MAX_DIGITS 1024
  char exc, file_digits[MAX_DIGITS], report_str[REPORT_STR] = {0};
  pr_time counted_time = 0, start_time = 0;
  PRINT_INF("BigInteger Benchmark!\n"
         "  N  |  count |    rate    |  time   |      \n"
         "-----|--------|------------|---------|------\n");
#ifdef UPDATE_RATE
  pr_time pt;
  #define START_CASE pt = start_time = profiling_current_time()
  #define UP_CASE \
    else if (profiling_as_fsec(profiling_time_since(pt)) < UPDATE_RATE) \
      continue; \
    pt = profiling_current_time()
#else
  #define START_CASE start_time = profiling_current_time()
  #define UP_CASE
#endif // UPDATE_RATE
#ifdef TIME
  #define TIMEOUT else if (profiling_as_fsec(profiling_time_since(start_time)) > TIME) \
      loop = false, snprintf(report_str, REPORT_STR, RED"Timeout"RESET)
#else
  #define TIMEOUT
#endif // TIME
#define CASE(N,F,I,G) do {\
  cnt = 0; \
  printf( N "| 000000 |    000.000 |      0s |"); \
  fd = open("data/math/" F ".txt", O_RDONLY | S_IRUSR);\
  if (fd < 0) {\
    printf( RED " file %s error!" RESET ".\n", F); \
    errflag = true; \
  } else {\
    printf(" start "); \
  	max_read = read(fd, file_digits, MAX_DIGITS), current_read = 0; \
    loop = true; \
    snprintf(report_str, REPORT_STR," ----- "); \
    (I); \
    START_CASE; \
    do {\
      (G); \
      if ((file_digits[current_read] - exc) != '0') {\
        snprintf(report_str, REPORT_STR, RED "x%d √%c" RESET, exc, file_digits[current_read]); \
        loop = false, errflag = true; \
      } else if (++cnt, ++current_read, max_read <= current_read) {\
      	current_read = 0;\
      	if ((max_read = read(fd, file_digits, MAX_DIGITS)) <= current_read) {\
	        snprintf(report_str, REPORT_STR, "EOF"); \
	        loop = false, errflag = true; \
      	}\
      } TIMEOUT; \
        UP_CASE; \
      fflush(stdout); \
      counted_time = profiling_time_since(start_time); \
      dstring_clean(qstr); \
      profiling_append_as_time(&qstr, counted_time); \
      printf("\r" N "| %06zu | %010.2e | %7s | %s", cnt, CAST(double)cnt / profiling_as_dsec(counted_time), qstr, report_str); \
    } while (loop); \
  	close(fd); \
  }\
  printf("\n"); \
  if (errflag) goto end; \
} while(0)
    CASE(" e   ", "eDigits", ({
      // created pre-calcl 100 iteration
      /*
      bigInteger_set_int (state, 1);
      bigInteger_set_int (state + 1, 1);
      for (i = 1; i < 100; ++i) {
        bigInteger_mmuli(state, i);
        bigInteger_mincr(state);
        bigInteger_mmuli(state + 1, i);
      }
      dstring_clean(qstr);
      bigInteger_append_dstring(&qstr, state[0]);
      printf("a: %s\n", qstr);
      dstring_clean(qstr);
      bigInteger_append_dstring(&qstr, state[1]);
      printf("b: %s\n", qstr);
      */
      bigInteger_set_cstr(state    , "253686955560127297415270748212280220445147578566298142232775185987449253908386446518940485425152049793267407732328003493609513499849694176709764490323163992001");
      bigInteger_set_cstr(state + 1,  "93326215443944152681699238856266700490715968264381621468592963895217599993229915608941463976156518286253697920827223758251185210916864000000000000000000000000");
      bigInteger_set_int (state + 2, 1);
      istate[0] = 101;
    }), ({
      bigInteger_mmuli(state    ,istate[0]);
      bigInteger_madd (state    , state[2]);
      bigInteger_mmuli(state + 1,istate[0]);
      ++istate[0];
      bigInteger_div_mod(state[0], state[1], state + 3, state + 4);
      bigInteger_set(state, state[4]);
      exc = state[3].count ? (char)state[3].items[0] : 0;
      bigInteger_mmuli(state    , 5);
      bigInteger_mshfri(state + 1, 1);
      bigInteger_mmuli(state + 2, 5);
    }));
    CASE(" √2  ", "√2Digits", ({
      // created pre-calcl 50 iteration
      /*
        bigInteger_set_int(state    , 1);
        bigInteger_set_int(state + 1, 2);
        bigInteger_set_int(state + 2, 1);
        for (i = 4, j = 3; i <= 100; i += 4, j += 2) {
          bigInteger_mmuli(state + 2, j);
          bigInteger_mmuli(state    , i);
          bigInteger_madd (state    , state[4]);
          bigInteger_mmuli(state + 1, i);
        }
        printf("i: %zu\n", i);
        printf("j: %zu\n", j);
        dstring_clean(qstr);
        bigInteger_append_dstring(&qstr, state[0]);
        printf("a: %s\n", qstr);
        dstring_clean(qstr);
        bigInteger_append_dstring(&qstr, state[1]);
        printf("b: %s\n", qstr);
        dstring_clean(qstr);
        bigInteger_append_dstring(&qstr, state[2]);
        printf("c: %s\n", qstr);
      */
      // 3 step
      bigInteger_set_int(state    , 71);
      bigInteger_set_int(state + 1, 64);
      bigInteger_set_int(state + 2, 12);
      bigInteger_set_int(state + 3, 7);
      bigInteger_set_int(state + 4, 15);
    }), ({
      for(;;) {
        bigInteger_set   (state + 6, state[4]);
        bigInteger_mmul  (state + 6, state[3]); 
        bigInteger_mshfli(state + 6, sizeof(word) * 4);
        bigInteger_set (state + 5, state[1]);
        bigInteger_mmul(state + 5, state[2]); 
        if (bigInteger_cmp(state[5], state[6]) > 0) break;
        bigInteger_mmul (state + 4, state[3]);
        bigInteger_mmul (state    , state[2]);
        bigInteger_madd (state    , state[4]);
        bigInteger_mmul (state + 1, state[2]);
        bigInteger_maddi(state + 2, 4);
        bigInteger_maddi(state + 3, 2);
//rep
      }
      bigInteger_div_mod(state[0], state[1], state + 5, state + 6);
      bigInteger_set(state, state[6]);
      exc = state[5].count ? (char) state[5].items[0] : 0;
      bigInteger_mmuli (state    , 5);
      bigInteger_mmuli (state + 4, 5);
      bigInteger_mshfri(state + 1, 1); 
    }));
    CASE(" pi  ", "piDigits", ({
      bigInteger_set_int(state    , 1);
      bigInteger_set_int(state + 1, 6);
      bigInteger_set_int(state + 2, 3);
      bigInteger_set_int(state + 3, 2);
      bigInteger_set_int(state + 4, 5);
      bigInteger_set_int(state + 5, 3);
    }), ({
      for(;;) {  
        bigInteger_set     (state + 6, state[0]);
        bigInteger_mshfli  (state + 6, 2);
        bigInteger_madd    (state + 6, state[1]);
        bigInteger_set     (state + 7, state[5]);
        bigInteger_mincr   (state + 7);
        bigInteger_mmul    (state + 7, state[2]);
        if (bigInteger_cmp(state[6], state[7]) <= 0) break;
        bigInteger_mmul  (state + 2, state[4]);
        bigInteger_set   (state + 5, state[3]);
        bigInteger_mmuli (state + 5, 7);
        bigInteger_maddi (state + 5, 2);
        bigInteger_mmul  (state + 5, state[0]);
        bigInteger_set   (state + 6, state[1]);
        bigInteger_mmul  (state + 6, state[4]);
        bigInteger_madd  (state + 5, state[6]);
        bigInteger_mdiv  (state + 5, state[2]);
        bigInteger_set   (state + 6, state[0]);
        bigInteger_mshfli(state + 6, 1);
        bigInteger_madd  (state + 1, state[6]);
        bigInteger_mmul  (state + 1, state[4]);
        bigInteger_mmul  (state    , state[3]);
        bigInteger_mincr (state + 3);
        bigInteger_maddi (state + 4, 2);
      }
      exc = state[5].count ? (char) state[5].items[0] : 0;
      bigInteger_mmuli(state, 10);
      bigInteger_mmul (state + 5, state[2]);
      bigInteger_msub (state + 1, state[5]);
      bigInteger_mmuli(state + 1, 10);
      bigInteger_set  (state + 5, state[0]);
      bigInteger_mmuli(state + 5, 3);
      bigInteger_madd (state + 5, state[1]);
      bigInteger_mdiv (state + 5, state[2]);
    }));
#undef CASE
  result = EXIT_SUCCESS;
end:
  for (i = 0; i < 8; ++i)
    bigInteger_free(state + i);
  dstring_free(&qstr);
  return result;
}








