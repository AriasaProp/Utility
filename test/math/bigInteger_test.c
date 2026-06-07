#include "math/bigInteger.h"
#include "common.h"
#include "util/console_out.h"
#include "util/profiling.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

// duration limit each test
#define TIME 2
// undef UPDATE_RATE for info
#define UPDATE_RATE 0.6

void root2_init(bigInteger*);
void e_init(bigInteger*);
void pi_init(bigInteger*);

void root2_ex(bigInteger*,char*);
void e_ex(bigInteger*,char*);
void pi_ex(bigInteger*,char*);

static void print_out(const char *sp, const char *lb, iter cnt, double tm, const char *ts, const char *reason) {
  fflush(stdout);
  printf("\r%s%s| %06zu | %08.1f | %6s | %s ", sp, lb, cnt, CAST(double)cnt / tm, ts, reason);
}

static const iter BIG_TENS = sizeof(word) * 8;

bigInteger state[8];
int statei[8];
int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  int result = EXIT_SUCCESS;
  String qstr = NULL;
  char text_buffer[1024] = {0};
  iter i,cnt = 0;
    // time markdown
  pr_time counted_time, start_time;
#ifdef UPDATE_RATE
  pr_time pt;
#endif
    ;
  {
    FILE *file = fopen("data/math/bigIntegerTest.txt", "r");
    if (!file) { PRINT_ERR("bigIntegerTest.txt file was not found!"); goto end; }
    printf(" Test |  total | spd(/ms) |  time  | reason \n"
           "------|--------|----------|--------|--------\n");
  // basic operation test
#ifdef UPDATE_RATE
    pt = 
#endif
    start_time = profiling_current_time();
basic_start:
      // start basic test
#define OUT_BASE(S) do {\
  counted_time = profiling_time_since(start_time); \
  string_clean(qstr);\
  profiling_append_as_time(&qstr, counted_time);\
  print_out(" ","Basic",cnt, profiling_as_dms(counted_time), qstr, (S));\
} while(0)
#define OUT_INF(S) do {\
  OUT_BASE(S);\
  goto basic_end;\
} while(0)
#define OUT_ERR(S) do {\
  OUT_BASE(S);\
  goto basic_err;\
} while(0)
#define EXTRACT(X) do {\
  if (!fscanf(file, " %s", text_buffer)) \
    OUT_ERR("[" #X "] fmt!");\
  bigInteger_set_cstr(&state[(X)],text_buffer); \
} while (0)
#define CASE3(L, X) case L: { \
  EXTRACT(0); EXTRACT(1); EXTRACT(2);\
  if (!fscanf(file, " #%[^\n]", text_buffer)) \
    OUT_ERR("comment fmt!");\
  UNUSED(fgetc(file)); \
  bigInteger_##X(&state[0], state[1]);\
  if (bigInteger_cmp(state[0], state[2])) \
    OUT_ERR(text_buffer);\
} break
#define CASE1(L, X) case L: {\
  EXTRACT(0); EXTRACT(1); \
  if (!fscanf(file, " #%[^\n]", text_buffer)) \
    OUT_ERR("comment fmt!");\
  UNUSED(fgetc(file)); \
  bigInteger_##X(&state[0]);\
  if (bigInteger_cmp(state[0], state[1])) \
    OUT_ERR(text_buffer);\
} break
  	switch (fgetc(file)) {
  	  CASE3('A',madd);
  	  CASE3('B',msub);
  	  CASE3('C',mmul);
  	  CASE3('D',mdiv);
  	  CASE3('G',mmod);
  	  case 'E': {
        EXTRACT(0);
        if (!fscanf(file, " %d", &statei[0]))
          OUT_ERR("format wrong!");
        EXTRACT(1);
        if (!fscanf(file, " #%[^\n]", text_buffer))
          OUT_ERR("format wrong!");
        UNUSED(fgetc(file));
        bigInteger_mpowi(&state[0], statei[0]);
        if (bigInteger_cmp(state[0], state[1]))
          OUT_ERR(text_buffer);
      } break;
  	  CASE1('F', msqrt);
  		case 'H': {
  		  EXTRACT(0); EXTRACT(1); EXTRACT(2); EXTRACT(3);
  	    if (!fscanf(file, " #%[^\n]", text_buffer))
          OUT_ERR("format wrong!");
        UNUSED(fgetc(file));
  	    bigInteger_free(&state[4]);
  	    state[4] = bigInteger_div_mmod(&state[0], state[1]);
        if (bigInteger_cmp(state[4], state[2]) || bigInteger_cmp(state[0], state[3]))
          OUT_ERR(text_buffer);
  		} break;
  		case EOF: OUT_INF("eof");
  		default: OUT_ERR("invalid test");
  	}
    cnt++;
#ifdef TIME
    if (profiling_as_fsec(counted_time) >= TIME)
      OUT_INF("timeout");
#endif // TIME
#ifdef UPDATE_RATE
    if (profiling_as_fsec(profiling_time_since(pt)) > UPDATE_RATE) {
#endif // UPDATE_RATE
      OUT_BASE("-----");
#ifdef UPDATE_RATE
      pt = profiling_current_time();
    }
#endif // UPDATE_RATE
    goto basic_start;
basic_err:
    result = EXIT_FAILURE;
basic_end:
    string_clean(qstr);
    bigInteger_append_string(&qstr, state[0]);
    printf("\n [0] %s ", qstr);
    string_clean(qstr);
    bigInteger_append_string(&qstr, state[2]);
    printf("\n [2] %s \n", qstr);
    fclose(file);
#undef CASE1
#undef CASE3
#undef EXTRACT
#undef OUT_ERR
#undef OUT_INF
#undef OUT_BASE
    if (result == EXIT_FAILURE) {
      printf("Failure end!\n");
      goto end;
    }
  }
  {
  // trancendental extraction test
    struct {
      const char *lb, *f;
      void (*init)(bigInteger*);
      void (*ex)(bigInteger*,char*);
    } df[] = {
      {
        .lb = " e",
        .f = "data/math/eDigits.txt",
        .init = e_init,
        .ex = e_ex,
      },
      {
        .lb = "pi",
        .f = "data/math/piDigits.txt",
        .init = pi_init,
        .ex = pi_ex,
      },
      {
        .lb = " √2",
        .f = "data/math/√2Digits.txt",
        .init = root2_init,
        .ex = root2_ex,
      },
    };
    char exc;
    void *file_digits;
    const char *file_digits_current, *file_digits_end;
    struct stat bstat;
    
    for (i = 0; (result == EXIT_SUCCESS) && (i < STACK_ARR_LEN(df)); ++i) {
      printf("ex %3s|", df[i].lb);
      { // file test existance
        int fd = open(df[i].f, O_RDONLY | S_IRUSR);
        if ((fd < 0)||(fstat(fd, &bstat) < 0)||!(file_digits = mmap(NULL, bstat.st_size, PROT_READ, MAP_PRIVATE, fd, 0))) {
          printf("%s " RED " error!" RESET ".\n", df[i].f);
          close(fd);
          continue;
        }
        file_digits_current = (const char*)file_digits;
        file_digits_end = file_digits_current + bstat.st_size;
        close(fd);
      }
      // starting 
      df[i].init(state);
#ifdef UPDATE_RATE
      pt = 
#endif
      start_time = profiling_current_time();
#define OUT_BASE(S) do {\
  counted_time = profiling_time_since(start_time); \
  string_clean(qstr);\
  profiling_append_as_time(&qstr, counted_time);\
  print_out("ex ", df[i].lb, cnt, profiling_as_dms(counted_time), qstr, S);\
} while(0)
#define OUT_ERR(S) do {\
  OUT_BASE(S);\
  goto extr_err;\
} while(0)
#define OUT_INF(S) do {\
  OUT_BASE(S);\
  goto extr_end;\
} while(0)
    // loop
extr_start:
      df[i].ex(state,&exc);
      // check result
      if ((*file_digits_current - exc) != '0') {
        sprintf(text_buffer, RED "x%d √%c" RESET, (int)exc, *file_digits_current);
        OUT_ERR(text_buffer);
      }
      ++cnt;
      if (++file_digits_current >= file_digits_end)
        OUT_INF(RED "eof" RESET);
#ifdef TIME
      if (profiling_as_fsec(profiling_time_since(start_time)) > TIME)
        OUT_INF(RED "timeout" RESET);
#endif
#ifdef UPDATE_RATE
      if (profiling_as_fsec(profiling_time_since(pt)) > UPDATE_RATE) {
#endif
        OUT_BASE("----");
#ifdef UPDATE_RATE
        pt = profiling_current_time();
      }
#endif
      goto extr_start;
#undef OUT_ERR
#undef OUT_INF
#undef OUT_BASE
extr_err:
      result = EXIT_FAILURE;
extr_end:
      printf("\n");
      munmap(file_digits, bstat.st_size);
    }
  }
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









