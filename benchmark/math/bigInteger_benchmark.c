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
#define UPDATE_RATE 0.8

// static const iter BIG_TENS = sizeof(word) * 8;
static const iter DIGITS_CLAMP = 100000;

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  bigInteger state[8] = {0};
  dstring qstr = NULL;
  int result = EXIT_FAILURE;
  bool loop, errflag = false;
#define ERR_STR 256
  char err_str[ERR_STR] = {0};
  iter i;
  pr_time counted_time = 0, start_time = 0;
  
  // bigInteger_set_int(state    , 1);
  // bigInteger_set_int(state + 1, 2);
  // bigInteger_set_int(state + 2, 1);
  // for (i = 4, j = 3; i <= 100; i += 4, j += 2) {
  //   bigInteger_mmuli(state + 2, j);
  //   bigInteger_mmuli(state    , i);
  //   bigInteger_madd (state    , state[4]);
  //   bigInteger_mmuli(state + 1, i);
  // }
  // printf("i: %zu\n", i);
  // printf("j: %zu\n", j);
  // dstring_clean(qstr);
  // bigInteger_append_dstring(&qstr, state[0]);
  // printf("a: %s\n", qstr);
  // dstring_clean(qstr);
  // bigInteger_append_dstring(&qstr, state[1]);
  // printf("b: %s\n", qstr);
  // dstring_clean(qstr);
  // bigInteger_append_dstring(&qstr, state[2]);
  // printf("c: %s\n", qstr);
  PRINT_INF("BigInteger Benchmark!\n"
         "  N  | count |    rate    |  time   |      \n"
         "-----|-------|------------|---------|------\n");
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
      loop = false, snprintf(err_str, ERR_STR, RED"Timeout"RESET)
#else
  #define TIMEOUT
#endif // TIME 
#define CASE(N,F,I,G) do {\
  char exc; \
  iter cnt = 0; \
  printf( N "| 00000 |    000.000 |      0s |"); \
  struct stat bstat; \
  void *file_digits; \
  { \
    int fd = open("data/math/" F ".txt", O_RDONLY | S_IRUSR);\
    if ((fd < 0)||(fstat(fd, &bstat) < 0)||!(file_digits = mmap(NULL, bstat.st_size, PROT_READ, MAP_PRIVATE, fd, 0))) {\
      printf( RED " file %s error!" RESET ".\n", F); \
      errflag = true; \
    } else {\
      printf(" start "); \
    }\
    close(fd); \
  } \
  if (!errflag) {\
    loop = true; \
    snprintf(err_str, ERR_STR," ----- "); \
    const char *file_digits_char = (const char*)file_digits; \
    (I); \
    START_CASE; \
    do {\
      (G); \
      if ((file_digits_char[cnt] - exc) != '0') {\
        snprintf(err_str, ERR_STR, RED "x%d √%c" RESET, exc, file_digits_char[cnt]); \
        loop = false, errflag = true; \
      } else if (!(++cnt < DIGITS_CLAMP)) {\
        snprintf(err_str, ERR_STR, "EOF"); \
        loop = false, errflag = true; \
      } TIMEOUT; \
        UP_CASE; \
      fflush(stdout); \
      counted_time = profiling_time_since(start_time); \
      dstring_clean(qstr); \
      profiling_append_as_time(&qstr, counted_time); \
      printf("\r" N "| %05zu | %010.2e | %7s | %s", cnt, CAST(double)cnt / profiling_as_dsec(counted_time), qstr, err_str); \
    } while (loop); \
    munmap(file_digits, bstat.st_size); \
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
      bigInteger_set_int (state + 2, 101);
      bigInteger_set_int (state + 3, 1);
    }), ({
      bigInteger_mmul (state    , state[2]);
      bigInteger_madd (state    , state[3]);
      bigInteger_mmul (state + 1, state[2]);
      bigInteger_mincr(state + 2);
      bigInteger_div_mod(state[0], state[1], state + 4, state + 5);
      bigInteger_set(state, state[5]);
      exc = state[4].count ? (char)state[4].items[0] : 0;
      bigInteger_mmuli(state    , 5);
      bigInteger_mshfri(state + 1, 1);
      bigInteger_mmuli(state + 3, 5);
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








