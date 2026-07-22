#include "math/bigInteger.h"
#include "util/console_out.h"
#include "util/profiling.h"
#include "common.h"

#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

// duration limit each test
#define TIME 0.98
// undef UPDATE_RATE for info
#define UPDATE_RATE 0.1

// static const iter BIG_TENS = sizeof(word) * 8;
static void init_e    (bigInteger*);
static void init_sqrt2(bigInteger*);
static void init_pi   (bigInteger*);
static char extract_e    (bigInteger*);
static char extract_sqrt2(bigInteger*);
static char extract_pi   (bigInteger*);

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  bigInteger state[8] = {0};
  int fd, result = EXIT_FAILURE;
  dstring qstr = NULL;
  bool errflag = false;
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
#endif // UPDATE_RATE
	struct {
		const char *id;
		const char *file;
		void (*init)(bigInteger*);
		char (*extract)(bigInteger*);
	} trsc[] = {
		{.id = "  e  ", .file = "data/math/eDigits.txt" , .init = init_e    , .extract = extract_e    },
		{.id = "  √2 ", .file = "data/math/√2Digits.txt", .init = init_sqrt2, .extract = extract_sqrt2},
		{.id = "  pi ", .file = "data/math/piDigits.txt", .init = init_pi   , .extract = extract_pi   },
	};
	for (i = 0; !errflag && (i < STACK_ARR_LEN(trsc)); ++i) {
	  cnt = 0; 
	  printf("%s| 000000 |    000.000 |      0s |", trsc[i].id); 
	  fd = open(trsc[i].file, O_RDONLY | S_IRUSR);
	  if (fd < 0) {
	    printf( RED " file %s error!" RESET ".\n", trsc[i].file); 
	    errflag = true;
	    continue;
	  }
    printf(" start "); 
  	max_read = read(fd, file_digits, MAX_DIGITS); 
  	current_read = 0; 
    snprintf(report_str, REPORT_STR," ----- "); 
    trsc[i].init(state); 
#ifdef UPDATE_RATE
		pt = 
#endif // UPDATE_RATE
		start_time = profiling_current_time();
    for (bool loop = true; loop;) {
    	exc = trsc[i].extract(state); 
      if ((file_digits[current_read] - exc) != '0') {
        snprintf(report_str, REPORT_STR, RED "x%d √%c" RESET, exc, file_digits[current_read]); 
        loop = false, errflag = true; 
      } else if (++cnt, max_read <= ++current_read) {
      	current_read = 0;
      	if ((max_read = read(fd, file_digits, MAX_DIGITS)) <= current_read) {
	        snprintf(report_str, REPORT_STR, "EOF"); 
	        loop = false, errflag = true; 
      	}
      }
#ifdef TIME
	  	else if (profiling_as_fsec(profiling_time_since(start_time)) > TIME) 
	      loop = false, snprintf(report_str, REPORT_STR, RED"Timeout"RESET);
#endif // TIME
#ifdef UPDATE_RATE
	    else if (profiling_as_fsec(profiling_time_since(pt)) < UPDATE_RATE) 
	      continue; 
	    pt = profiling_current_time();
#endif // UPDATE_RATE
      fflush(stdout); 
      counted_time = profiling_time_since(start_time); 
      dstring_clean(qstr); 
      profiling_append_as_time(&qstr, counted_time); 
      printf("\r%s| %06zu | %010.2e | %7s | %s", trsc[i].id, cnt, CAST(double)cnt / profiling_as_dsec(counted_time), qstr, report_str); 
    }
  	close(fd);
    printf("\n");
  	if (errflag) goto end; 
	}
  result = EXIT_SUCCESS;
end:
	if (result == EXIT_FAILURE) {
		for (i = 0; i < 8; ++i) {
			dstring_clean(qstr);
			bigInteger_append_dstring(&qstr, state[i]);
			PRINT_ERR("%01zu: %30s\n", i, qstr);
		}
	}
  for (i = 0; i < 8; ++i)
    bigInteger_free(state + i);
  dstring_free(&qstr);
  return result;
}


/*
 *        1
 * e =>  -----
 *        n!
 * 4
 * 2
 *  2     1     1
 * --- + --- + ---- + ...
 *  1!    2!    3!
 */
void init_e(bigInteger *state) {
  bigInteger_set_int (state    , 2);
  bigInteger_set_int (state + 1, 1);
  bigInteger_set_int (state + 2, 1);
  bigInteger_set_int (state + 3, 2);
}
char extract_e(bigInteger *state) {
  bigInteger_mmul (state + 0, state[3]);
  bigInteger_madd (state + 0, state[2]);
  bigInteger_mmul (state + 1, state[3]);
  bigInteger_mincr(state + 3);
  bigInteger_mmul (state + 0, state[3]);
  bigInteger_madd (state + 0, state[2]);
  bigInteger_mmul (state + 1, state[3]);
  bigInteger_mincr(state + 3);
  bigInteger_div_mod(state[0], state[1], state + 4, state + 5);
  char exc = state[4].count ? (char)state[4].items[0] : 0;
  bigInteger_set   (state + 0, state[5]);
  bigInteger_mmuli (state + 0, 5);
  bigInteger_mshfri(state + 1, 1);
  bigInteger_mmuli (state + 2, 5);
	return exc;
}
/*
 *  a     c * n
 * --- + ------
 *  b     b * m
 *
 *  a * m     c * n
 * ------- + --------
 *  b * m     b * m
 *
 *  am + cn
 * -----------
 *   b * m   
 *
 * n => 3 + 2
 * m => 4 + 4
 *
 * 0 -> 1
 * a = 1
 * b = 2
 * c = 1
 * n = 3
 * m = 4
 * 10 -> 11
 * a = 268341722775 -> 94653165
 * b = 190253629440 -> 67108864
 * c =    654729075 ->   230945
 * n = 21
 * m = 40
 */
void init_sqrt2(bigInteger *state) {
  bigInteger_set_int(state    , 94653165); // a
  bigInteger_set_int(state + 1, 67108864); // b
  bigInteger_set_int(state + 2, 230945); // c
  bigInteger_set_int(state + 3, 21); // n
  bigInteger_set_int(state + 4, 40); // m
}
char extract_sqrt2(bigInteger *state) {
	for (iter i = 0; i < 4; ++i) {
    bigInteger_mmul(state + 2, state[3]);
    bigInteger_mmul(state + 0, state[4]);
    bigInteger_madd(state + 0, state[2]);
    bigInteger_mmul(state + 1, state[4]);
    bigInteger_maddi(state + 3, 2);
    bigInteger_maddi(state + 4, 4);
	}
	bigInteger_div_mod(state[0], state[1], state + 5, state + 6);
	char exc = state[5].count ? state[5].items[0] : 0;
	bigInteger_set(state + 0, state[6]);
  bigInteger_mmuli(state + 0, 5);
  bigInteger_mshfri(state + 1, 1);
  bigInteger_mmuli(state + 2, 5);
  return exc;
}
void init_pi(bigInteger *state) {
  bigInteger_set_int(state    , 1);
  bigInteger_set_int(state + 1, 6);
  bigInteger_set_int(state + 2, 3);
  bigInteger_set_int(state + 3, 2);
  bigInteger_set_int(state + 4, 5);
  bigInteger_set_int(state + 5, 3);
}
char extract_pi(bigInteger *state) {
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
    bigInteger_mmul  (state + 0, state[3]);
    bigInteger_mincr (state + 3);
    bigInteger_maddi (state + 4, 2);
  }
  char exc = state[5].count ? (char) state[5].items[0] : 0;
  bigInteger_mmuli(state + 0, 10);
  bigInteger_mmul (state + 5, state[2]);
  bigInteger_msub (state + 1, state[5]);
  bigInteger_mmuli(state + 1, 10);
  bigInteger_set  (state + 5, state[0]);
  bigInteger_mmuli(state + 5, 3);
  bigInteger_madd (state + 5, state[1]);
  bigInteger_mdiv (state + 5, state[2]);
	return exc;
}







