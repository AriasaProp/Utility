#include "math/bigInteger.h"
#include "util/console_out.h"
#include "util/profiling.h"
#include "common.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

// duration limit each test
#define TIME 5
// undef UPDATE_RATE for info
#define UPDATE_RATE 0.6

void root2_init(bigInteger*);
void e_init(bigInteger*);
void pi_init(bigInteger*);

void root2_ex(bigInteger*,char*);
void e_ex(bigInteger*,char*);
void pi_ex(bigInteger*,char*);

static const iter BIG_TENS = sizeof(word) * 8;

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  bigInteger state[8] = {0};
  int result = EXIT_FAILURE;
  String qstr = NULL;
#define ERR_STR 256
  char err_str[ERR_STR] = {0};
  iter i,cnt = 0;
  pr_time counted_time, start_time;
  PRINT_INF("BigInteger Test!\n");
  printf("  Name  |count|   rate   |    time    |      \n"
         "--------|-----|----------|------------|------\n");
#ifdef UPDATE_RATE
  pr_time pt;
  #define START_CASE pt = start_time = profiling_current_time()
  #define UP_CASE do {\
    if(profiling_as_fsec(profiling_time_since(pt)) < UPDATE_RATE) continue; \
    pt = profiling_current_time(); \
  } while(0)
#else
  #define START_CASE start_time = profiling_current_time()
  #define UP_CASE
#endif // UPDATE_RATE
#ifdef TIME
  #define TIMEOUT if ((profiling_as_fsec(profiling_time_since(start_time)) > TIME) && (snprintf(err_str, ERR_STR, RED"Timeout"RESET) > 0)) break
#else
  #define TIMEOUT
#endif // TIME
  {
#define MAX_RNDI 4
#define RAND_S  CAST(int)(imath_rand_ubyte()&1)
#define RAND_C  CAST(iter)((imath_rand_uint() % MAX_RNDI) + 1)
#define RAND_W  CAST(word)(imath_rand_uint() + imath_rand_uint())
#define RAND_I  imath_rand_int()
    word rndT[MAX_RNDI + 1] = {0};
    iter nword;
    int oprB;
#define COMMON_TEST 1080
#define IFERROR do {\
  if (cnt >= COMMON_TEST) break;\
  printf("i: %50d\n", oprB);\
  string_clean(qstr);\
  bigInteger_append_string(&qstr, state[0]);\
  printf("0: %50s\n", qstr);\
  string_clean(qstr);\
  bigInteger_append_string(&qstr, state[1]);\
  printf("1: %50s\n", qstr);\
  string_clean(qstr);\
  bigInteger_append_string(&qstr, state[2]);\
  printf("2: %50s\n", qstr);\
  string_clean(qstr);\
  bigInteger_append_string(&qstr, state[4]);\
  printf("4: %50s\n", qstr);\
  goto end; \
} while(0)
#define CASEW(A,B) do {\
  printf(" "#A"_"#B"|00000|   000.000|          0s| starting "); \
  START_CASE;\
  cnt = 0;\
  do {\
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ;\
    bigInteger_set_words(state + 0, RAND_S, rndT, nword); \
    state[3] = bigInteger_##A (state[0]); \
    bigInteger_move(state + 1, state + 3); \
    state[3] = bigInteger_##B (state[1]); \
    bigInteger_move(state + 2, state + 3); \
    if (bigInteger_cmp(state[0],state[2])) { \
      snprintf(err_str, ERR_STR, RED"big "#A"_"#B RESET); \
      break; \
    }\
    TIMEOUT; \
    if (++cnt > COMMON_TEST) {\
      snprintf(err_str, ERR_STR, GREEN"Done!"RESET); \
      break; \
    }\
    UP_CASE; \
    fflush(stdout); \
    counted_time = profiling_time_since(start_time); \
    string_clean(qstr);\
    profiling_append_as_time2(&qstr, counted_time); \
    printf("\r"#A#B"|%05zu|%010.2e| %11s| --- ", cnt, CAST(double)cnt / profiling_as_dsec(counted_time), qstr); \
  } while (1); \
  fflush(stdout); \
  counted_time = profiling_time_since(start_time); \
  string_clean(qstr);\
  profiling_append_as_time2(&qstr, counted_time); \
  printf("\r"#A#B"|%05zu|%010.2e| %11s| %s \n", cnt, CAST(double)cnt / profiling_as_dsec(counted_time), qstr, err_str); \
  IFERROR;\
} while(0)
#define CASE(A,B) do {\
  printf(" "#A"_"#B"|00000|   000.000|          0s| starting "); \
  START_CASE;\
  cnt = 0;\
  do {\
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ;\
    bigInteger_set_words(state, (RAND_S && nword), rndT, nword);\
    oprB = RAND_I;\
    state[3] = bigInteger_##A##i(state[0], oprB);\
    bigInteger_move(state + 1, state + 3);\
    state[3] = bigInteger_##B##i(state[1], oprB);\
    bigInteger_move(state + 2, state + 3);\
    if (bigInteger_cmp(state[0],state[2])) {\
      snprintf(err_str, ERR_STR, RED"i "#A"_"#B RESET);\
      break;\
    }\
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ;\
    bigInteger_set_words(state + 0, 0, rndT, nword); \
    for (nword = RAND_C,i = 0; i < nword; rndT[i++] = RAND_W) ; \
    bigInteger_set_words(state + 1, 0, rndT, nword); \
    state[3] = bigInteger_##A (state[0], state[1]); \
    bigInteger_move(state + 2, state + 3); \
    state[3] = bigInteger_##B (state[2], state[1]); \
    bigInteger_move(state + 4, state + 3); \
    if (bigInteger_cmp(state[0],state[4])) { \
      snprintf(err_str, ERR_STR, RED"big "#A"_"#B RESET); \
      break; \
    }\
    TIMEOUT; \
    if (++cnt > COMMON_TEST) {\
      snprintf(err_str, ERR_STR, GREEN"Done!"RESET); \
      break; \
    }\
    UP_CASE; \
    fflush(stdout); \
    counted_time = profiling_time_since(start_time); \
    string_clean(qstr);\
    profiling_append_as_time2(&qstr, counted_time); \
    printf("\r "#A"_"#B"|%05zu|%010.2e| %11s| --- ", cnt, CAST(double)cnt / profiling_as_dsec(counted_time), qstr); \
  } while (1); \
  fflush(stdout); \
  counted_time = profiling_time_since(start_time); \
  string_clean(qstr);\
  profiling_append_as_time2(&qstr, counted_time); \
  printf("\r "#A"_"#B"|%05zu|%010.2e| %11s| %s \n", cnt, CAST(double)cnt / profiling_as_dsec(counted_time), qstr, err_str); \
  IFERROR;\
} while(0)
    CASE(add,sub);
    CASE(sub,add);
    TODO("Division doesn't give correct result!");
    CASE(mul,div);
    CASEW(pow2,sqrt);
#undef IFERROR
#undef CASEW
#undef CASE
#undef COMMON_TEST
#undef MAX_RNDI
  }
  {
    // trancendental extraction test
    int fd;
    char exc;
    void *file_digits;
    const char *file_digits_current, *file_digits_end;
    struct stat bstat;
    
#define CASE(N,F,I,G) do {\
  do {\
    printf("%8s|00000|   000.000|        0s| starting ", N); \
    fd = open(F, O_RDONLY | S_IRUSR);\
    if ((fd < 0)||(fstat(fd, &bstat) < 0)||!(file_digits = mmap(NULL, bstat.st_size, PROT_READ, MAP_PRIVATE, fd, 0))) {\
      snprintf(err_str, ERR_STR, "%s " RED " error!" RESET ".\n", F);\
      close(fd);\
      break;\
    }\
    file_digits_current = (const char*)file_digits;\
    file_digits_end = file_digits_current + bstat.st_size;\
    close(fd);\
    I (state);\
    START_CASE;\
    do {\
      G (state,&exc);\
      if ((*file_digits_current - exc) != '0') {\
        snprintf(err_str, ERR_STR, RED "x%d √%c" RESET, (int)exc, *file_digits_current);\
        break;\
      }\
      ++cnt;\
      if (++file_digits_current >= file_digits_end) {\
        snprintf(err_str, ERR_STR, "EOF");\
        break;\
      }\
      TIMEOUT;\
      UP_CASE; \
      fflush(stdout); \
      counted_time = profiling_time_since(start_time); \
      string_clean(qstr);\
      profiling_append_as_time2(&qstr, counted_time); \
      printf("\r%8s|%05zu|%010.2e| %11s| --- ", N, cnt, CAST(double)cnt / profiling_as_dsec(counted_time), qstr); \
    } while (1);\
    munmap(file_digits, bstat.st_size);\
  } while(0); \
  fflush(stdout); \
  counted_time = profiling_time_since(start_time); \
  string_clean(qstr);\
  profiling_append_as_time2(&qstr, counted_time); \
  printf("\r%8s|%05zu|%010.2e| %11s| %s ", N, cnt, CAST(double)cnt / profiling_as_dsec(counted_time), qstr, err_str); \
} while(0)
    CASE( "e",  "data/math/eDigits.txt",     e_init,     e_ex);
    CASE("pi", "data/math/piDigits.txt",    pi_init,    pi_ex);
    CASE("√2", "data/math/√2Digits.txt", root2_init, root2_ex);
#undef CASE
  }
  if (*err_str) goto end;
  result = EXIT_SUCCESS;
end:
  for (i = 0; i < 8; ++i)
    bigInteger_free(state + i);
  string_free(&qstr);
  return result;
}
void e_init(bigInteger *s) {
  bigInteger_set_int(s    , 9864101);
  bigInteger_set_int(s + 1, 3628800);
  bigInteger_set_int(s + 2, 11);
  bigInteger_set_int(s + 3, 1);
}
void e_ex(bigInteger *s, char *r) {
  for(;;) {
    bigInteger_set   (s + 5, s[1]);
    bigInteger_mmul  (s + 5, s[2]);
    bigInteger_set   (s + 6, s[3]);
    bigInteger_mshfli(s + 6, BIG_TENS);
    if (bigInteger_cmp(s[5], s[6]) <= 0) break;
    bigInteger_mmul (s, s[2]);
    bigInteger_madd (s, s[3]);
    bigInteger_mmul (s + 1,s[2]);
    bigInteger_mincr(s + 2);
  }
  s[4] = bigInteger_div_mmod(s, s[1]);
  *r = s[4].count ? (char)s[4].items[0] : 0;
  bigInteger_free(s + 4);
  bigInteger_mmuli(s, 10);
  bigInteger_mmuli(s + 3, 10);
}

void root2_init(bigInteger *s) {
  bigInteger_set_int(s    , 1);
  bigInteger_set_int(s + 1, 2);
  bigInteger_set_int(s + 2, 4);
  bigInteger_set_int(s + 3, 3);
  bigInteger_set_int(s + 4, 1);
}
void root2_ex(bigInteger *s, char *r) {
  for(;;) {
    bigInteger_set   (s + 6, s[4]);
    bigInteger_mmul  (s + 6, s[3]); 
    bigInteger_mshfli(s + 6, BIG_TENS);
    bigInteger_set (s + 7, s[1]);
    bigInteger_mmul(s + 7, s[2]); 
    if (bigInteger_cmp(s[7], s[6]) <= 0) break;
    bigInteger_mmul(s + 4, s[3]);
    bigInteger_mmul(  s  , s[2]);
    bigInteger_madd(  s  , s[4]);
    bigInteger_mmul(s + 1, s[2]);
    bigInteger_maddi(s + 2, 4);
    bigInteger_maddi(s + 3, 2);
  }
  s[5] = bigInteger_div_mmod(s, s[1]);
  *r = s[5].count ? (char) s[5].items[0] : 0;
  bigInteger_free(s + 5);
  bigInteger_mmuli(  s  , 5);
  bigInteger_mmuli(s + 4, 5);
  bigInteger_mshfri(s + 1, 1); 
}

void pi_init(bigInteger *s) {
  bigInteger_set_int(s + 0, 1);
  bigInteger_set_int(s + 1, 6);
  bigInteger_set_int(s + 2, 3);
  bigInteger_set_int(s + 3, 2);
  bigInteger_set_int(s + 4, 5);
  bigInteger_set_int(s + 5, 3);
}
void pi_ex(bigInteger *s, char *r) {
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
    bigInteger_mmul  (s + 0, s[3]);
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









