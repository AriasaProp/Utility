#include "math/bigInteger.h"
#include "util/console_out.h"
#include "util/profiling.h"
#include "common.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

// duration limit each test
#define TIME 3
// undef UPDATE_RATE for info
#define UPDATE_RATE 0.6

static void root2_init(bigInteger*);
static void e_init(bigInteger*);
static void pi_init(bigInteger*);

static void root2_ex(bigInteger*,char*);
static void e_ex(bigInteger*,char*);
static void pi_ex(bigInteger*,char*);

static const iter BIG_TENS = sizeof(word) * 8;

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  bigInteger state[8] = {0};
  dstring qstr = NULL;
  int result = EXIT_FAILURE;
  bool loop, errflag = false;
#define ERR_STR 256
  char err_str[ERR_STR] = {0};
  iter i,cnt = 0;
  pr_time counted_time = 0, start_time = 0;
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
  #define TIMEOUT \
    else if (profiling_as_fsec(profiling_time_since(start_time)) > TIME) \
      loop = false, snprintf(err_str, ERR_STR, RED"Timeout"RESET)
#else
  #define TIMEOUT
#endif // TIME 
  char exc;
  void *file_digits;
  const char *file_digits_current, *file_digits_end;
  struct stat bstat;
  
#define CASE(N,F,I,G) do {\
  printf( N "| 00000 |    000.000 |      0s |"); \
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
    cnt = 0, loop = true; \
    snprintf(err_str, ERR_STR," ----- "); \
    file_digits_current = (const char*)file_digits; \
    file_digits_end = file_digits_current + bstat.st_size; \
    I (state); \
    START_CASE; \
    while (loop) {\
      G (state,&exc); \
      if ((*file_digits_current - exc) != '0') {\
        snprintf(err_str, ERR_STR, RED "x%d √%c" RESET, (int)exc, *file_digits_current); \
        loop = false, errflag = true; \
      } else {\
        ++cnt; \
        if (++file_digits_current >= file_digits_end) {\
          snprintf(err_str, ERR_STR, "EOF"); \
          loop = false, errflag = true; \
        } TIMEOUT; \
          UP_CASE; \
      }\
      fflush(stdout); \
      counted_time = profiling_time_since(start_time); \
      dstring_clean(qstr); \
      profiling_append_as_time(&qstr, counted_time); \
      printf("\r" N "| %05zu | %010.2e | %7s | %s", cnt, CAST(double)cnt / profiling_as_dsec(counted_time), qstr, err_str); \
    }\
    munmap(file_digits, bstat.st_size); \
  }\
  printf("\n"); \
  if (errflag) goto end; \
} while(0)
    CASE(" √2  ", "√2Digits", root2_init, root2_ex);
    CASE(" e   ",  "eDigits",     e_init,     e_ex);
    CASE(" pi  ", "piDigits",    pi_init,    pi_ex);
#undef CASE
  result = EXIT_SUCCESS;
end:
  for (i = 0; i < 8; ++i)
    bigInteger_free(state + i);
  dstring_free(&qstr);
  return result;
}
static void e_init(bigInteger *s) {
  bigInteger_set_int(s    , 9864101);
  bigInteger_set_int(s + 1, 3628800);
  bigInteger_set_int(s + 2, 11);
  bigInteger_set_int(s + 3, 1);
}
static void e_ex(bigInteger *s, char *r) {
  for(;;) {
    bigInteger_set   (s + 5, s[1]);
    bigInteger_mmul  (s + 5, s[2]);
    bigInteger_set   (s + 4, s[3]);
    bigInteger_mshfli(s + 4, BIG_TENS);
    if (bigInteger_cmp(s[5], s[4]) > 0) break;
    bigInteger_mmul (s    , s[2]);
    bigInteger_madd (s    , s[3]);
    bigInteger_mmul (s + 1, s[2]);
    bigInteger_mincr(s + 2);
  }
  bigInteger_div_mod(s[0], s[1], s + 4, s + 5);
  bigInteger_set(s, s[5]);
  *r = s[4].count ? (char)s[4].items[0] : 0;
  bigInteger_mmuli(s    , 10);
  bigInteger_mmuli(s + 3, 10);
}

static void root2_init(bigInteger *s) {
  bigInteger_set_int(s    , 1);
  bigInteger_set_int(s + 1, 2);
  bigInteger_set_int(s + 2, 4);
  bigInteger_set_int(s + 3, 3);
  bigInteger_set_int(s + 4, 1);
}
static void root2_ex(bigInteger *s, char *r) {
  for(;;) {
    bigInteger_set   (s + 6, s[4]);
    bigInteger_mmul  (s + 6, s[3]); 
    bigInteger_mshfli(s + 6, BIG_TENS);
    bigInteger_set (s + 5, s[1]);
    bigInteger_mmul(s + 5, s[2]); 
    if (bigInteger_cmp(s[5], s[6]) > 0) break;
    bigInteger_mmul(s + 4, s[3]);
    bigInteger_mmul(  s  , s[2]);
    bigInteger_madd(  s  , s[4]);
    bigInteger_mmul(s + 1, s[2]);
    bigInteger_maddi(s + 2, 4);
    bigInteger_maddi(s + 3, 2);
  }
  bigInteger_div_mod(s[0], s[1], s + 5, s + 6);
  bigInteger_set(s, s[6]);
  *r = s[5].count ? (char) s[5].items[0] : 0;
  bigInteger_mmuli(s, 5);
  bigInteger_mmuli(s + 4, 5);
  bigInteger_mshfri(s + 1, 1); 
}

static void pi_init(bigInteger *s) {
  bigInteger_set_int(s    , 1);
  bigInteger_set_int(s + 1, 6);
  bigInteger_set_int(s + 2, 3);
  bigInteger_set_int(s + 3, 2);
  bigInteger_set_int(s + 4, 5);
  bigInteger_set_int(s + 5, 3);
}
static void pi_ex(bigInteger *s, char *r) {
  for(;;) {  
    bigInteger_set     (s + 6, s[0]);
    bigInteger_mshfli  (s + 6, 2);
    bigInteger_madd    (s + 6, s[1]);
    bigInteger_set     (s + 7, s[5]);
    bigInteger_mincr   (s + 7);
    bigInteger_mmul    (s + 7, s[2]);
    if (bigInteger_cmp(s[6],s[7]) <= 0) break;
    bigInteger_mmul  (s + 2, s[4]);
    bigInteger_set   (s + 5, s[3]);
    bigInteger_mmuli (s + 5, 7);
    bigInteger_maddi (s + 5, 2);
    bigInteger_mmul  (s + 5, s[0]);
    bigInteger_set   (s + 6, s[1]);
    bigInteger_mmul  (s + 6, s[4]);
    bigInteger_madd  (s + 5, s[6]);
    bigInteger_mdiv  (s + 5, s[2]);
    bigInteger_set   (s + 6, s[0]);
    bigInteger_mshfli(s + 6, 1);
    bigInteger_madd  (s + 1, s[6]);
    bigInteger_mmul  (s + 1, s[4]);
    bigInteger_mmul  (s    , s[3]);
    bigInteger_mincr (s + 3);
    bigInteger_maddi (s + 4, 2);
  }
  *r = s[5].count ? (char) s[5].items[0] : 0;
  bigInteger_mmuli(s, 10);
  bigInteger_mmul (s + 5, s[2]);
  bigInteger_msub (s + 1, s[5]);
  bigInteger_mmuli(s + 1, 10);
  bigInteger_set  (s + 5, s[0]);
  bigInteger_mmuli(s + 5, 3);
  bigInteger_madd (s + 5, s[1]);
  bigInteger_mdiv (s + 5, s[2]);
}









