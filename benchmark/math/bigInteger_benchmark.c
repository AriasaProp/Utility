#include "math/bigInteger.h"
#include "util/console_out.h"
#include "util/profiling.h"
#include "common.h"

#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

// #define E_ONLY
// #define SQRT3_ONLY
// #define SQRT2_ONLY
// #define PI_ONLY

// duration limit each test
#define TIME 3
// undef UPDATE_RATE for info
// #define UPDATE_RATE 0.75
#define CACHE 6

// static const iter BIG_TENS = sizeof(word) * 8;
#if !(defined(PI_ONLY) || defined(SQRT2_ONLY) || defined(SQRT3_ONLY))
static void init_e   (bigInteger*);
static char extract_e(bigInteger*);
#endif
#if !(defined(PI_ONLY) || defined(SQRT3_ONLY) || defined(E_ONLY))
static void init_sqrt2   (bigInteger*);
static char extract_sqrt2(bigInteger*);
#endif
#if !(defined(PI_ONLY) || defined(SQRT2_ONLY) || defined(E_ONLY))
static void init_sqrt3   (bigInteger*);
static char extract_sqrt3(bigInteger*);
#endif
#if !(defined(SQRT2_ONLY) || defined(SQRT3_ONLY) || defined(E_ONLY))
static void init_pi   (bigInteger*);
static char extract_pi(bigInteger*);
#endif

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  bigInteger state[CACHE] = {0};
  int fd, result = EXIT_FAILURE;
  dstring qstr = NULL;
  bool errflag = false;
  iter i, cnt, max_read, current_read;
  for (i = 0; i < CACHE; ++i)
    bigInteger_reserve(state + i, 1);
#define REPORT_STR 32
#define MAX_DIGITS 2*1024
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
#if !(defined(PI_ONLY) || defined(SQRT2_ONLY) || defined(SQRT3_ONLY))
		{.id = "  e  ", .file = "data/math/eDigits.txt" , .init = init_e    , .extract = extract_e    },
#endif
#if !(defined(PI_ONLY) || defined(SQRT2_ONLY) || defined(E_ONLY))
		{.id = "  √3 ", .file = "data/math/√3Digits.txt", .init = init_sqrt3, .extract = extract_sqrt3},
#endif
#if !(defined(PI_ONLY) || defined(SQRT3_ONLY) || defined(E_ONLY))
		{.id = "  √2 ", .file = "data/math/√2Digits.txt", .init = init_sqrt2, .extract = extract_sqrt2},
#endif
#if !(defined(SQRT2_ONLY) || defined(SQRT3_ONLY) || defined(E_ONLY))
		{.id = "  pi ", .file = "data/math/piDigits.txt", .init = init_pi   , .extract = extract_pi   },
#endif
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
#ifdef UPDATE_RATE
		pt = 
#endif // UPDATE_RATE
		start_time = profiling_current_time();
    trsc[i].init(state); 
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
	// if (result == EXIT_FAILURE) {
	// 	for (i = 0; i < 8; ++i) {
	// 		dstring_clean(qstr);
	// 		bigInteger_append_dstring(&qstr, state[i]);
	// 		PRINT_ERR("%01zu: %30s\n", i, qstr);
	//     bigInteger_free(state + i);
	// 	}
	// } else
	cnt = 0;
  dstring_free(&qstr);
	for (i = 0; i < CACHE; ++i) {
	  cnt += state[i].cap;
	  bigInteger_free(state + i);
	}
	
	{
	  char uns[3] = {'B',0,0};
	  float amount = CAST(float)cnt;
	  if (amount > 1024.0f) {
	    amount /= 1024.0f;
	    uns[0] = 'k';
	    uns[1] = 'B';
	  }
	  if (amount > 1024.0f) {
	    amount /= 1024.0f;
	    uns[0] = 'M';
	    uns[1] = 'B';
	  }
	  if (amount > 1024.0f) {
	    amount /= 1024.0f;
	    uns[0] = 'G';
	    uns[1] = 'B';
	  }
	  if (amount > 1024.0f) {
	    amount /= 1024.0f;
	    uns[0] = 'T';
	    uns[1] = 'B';
	  }
    PRINT_INF("total integer cache allocated: %f %s\n", amount, uns);
	}
  return result;
}

#if !(defined(PI_ONLY) || defined(SQRT2_ONLY) || defined(SQRT3_ONLY))
/*
 *        1
 * e =>  -----
 *        n!
 * 
 * pre 9
 * a =  986410
 * b =  362880
 * c =  1
 * n =  9
 */
void init_e(bigInteger *state) {
  bigInteger_set_int(state    , 986410);
  bigInteger_set_int(state + 1, 362880);
  bigInteger_set_int(state + 2, 1);
  bigInteger_set_int(state + 3, 9);
}
char extract_e(bigInteger *state) {
  // e term has exponentially small than base
  // need less term for every digits
  while (state[1].count <= state[2].count) {
    bigInteger_mincr(state + 3);
    bigInteger_mmuladd(state + 0, state[3], state[2]);
    bigInteger_mmul(state + 1, state[3]);
  }
  // extract
  bigInteger_set(state + 4, state[0]);
  bigInteger_div_mod(state + 4, state[1], state + 0);
  // 10 base
  bigInteger_mmuli (state + 0, 10);
  bigInteger_mmuli (state + 2, 10);
  // simplify (not faster at all, but reduce memory usage :/ )
  // bigInteger_mgcd(state + 5, state + 0, 3);
  // bigInteger_mdiv(state + 0, state[5]);
  // bigInteger_mdiv(state + 1, state[5]);
  // bigInteger_mdiv(state + 2, state[5]);
	return state[4].count ? (char)state[4].items[0] : 0;
}
#endif

#if !(defined(PI_ONLY) || defined(SQRT3_ONLY) || defined(E_ONLY))
/*
 *  (4 + 3)     3 * 5 * (12 + 7)     3 * 5 * 7 * 9 * (20 + 11)
 * --------- + ------------------ + --------------------------
 *   2 * 4       2 * 4 * 8 * 12      2 * 4 * 8 * 12 * 16 * 20
 *
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
 * n = 1
 * m = 0
 * 32 -> 33
 * a =  23605555
 * b =  16777216
 * c =  109395
 * n =  17
 * m =  32
 */
void init_sqrt2(bigInteger *state) {
  bigInteger_set_int(state    , 23605555); // a
  bigInteger_set_int(state + 1, 16777216); // b
  bigInteger_set_int(state + 2, 109395); // c
  bigInteger_set_int(state + 3, 17); // n
  bigInteger_set_int(state + 4, 32); // m
}
char extract_sqrt2(bigInteger *state) {
  // √2 term has low dexponential grow precision
  // need more term for digit generated
  do {
	  bigInteger_maddi(state + 3, 2);
	  bigInteger_maddi(state + 4, 4);
	  bigInteger_mmul(state + 2, state[3]);
	  bigInteger_mmuladd(state, state[4], state[2]);
	  bigInteger_mmul(state + 1, state[4]);
  } while (state[1].count <= state[2].count);
  // extract
	bigInteger_set(state + 5, state[0]);
	bigInteger_div_mod(state + 5, state[1], state);
  // 10 base
	bigInteger_mmuli (state + 0, 5);
  bigInteger_mshfri(state + 1, 1);
  bigInteger_mmuli (state + 2, 5);
  // simplify
  // bigInteger_mgcd(state + 6, state   , 3);
  // bigInteger_mdiv(state + 0, state[6]);
  // bigInteger_mdiv(state + 1, state[6]);
  // bigInteger_mdiv(state + 2, state[6]);
  return state[5].count ? state[5].items[0] : 0;
}
#endif

#if !(defined(PI_ONLY) || defined(SQRT2_ONLY) || defined(E_ONLY))
/*
 *  3     3 * 1     3 * 1 * -1     3 * 1 * -1 * -3     3 * 1 * -1 * -3 * -5
 * --- + ------- + ------------ + ----------------- + ----------------------
 *  2     2 * 6     2 * 6 * 12     2 * 6 * 12 * 18     2 * 6 * 12 * 18 * 24
 * 
 *  7       -1         -1 * -3         -1 * -3 * -5         -1 * -3 * -5 * -7
 * --- + -------- + ------------- + ------------------ + -----------------------
 *  4     4 * 12     4 * 12 * 18     4 * 12 * 18 * 24     4 * 12 * 18 * 24 * 30
 * 
 *    83         -1 * -3         -1 * -3 * -5         -1 * -3 * -5 * -7
 * -------- + ------------- + ------------------ + -----------------------
 *  4 * 12     4 * 12 * 18     4 * 12 * 18 * 24     4 * 12 * 18 * 24 * 30
 * 
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
 * n => 3 - 2
 * m => 0 + 6
 *
 * 0 -> 1
 * a = 83
 * b = 48
 * c = 1
 * n = 1
 * m = 12
 *
 * a =  82749985
 * b =  47775744
 * c =  143
 * n =  13
 * m =  48
 */
void init_sqrt3(bigInteger *state) {
  bigInteger_set_int(state    , 82749985); // a
  bigInteger_set_int(state + 1, 47775744); // b
  bigInteger_set_int(state + 2, 143); // c
  bigInteger_set_int(state + 3, 13); // n
  bigInteger_set_int(state + 4, 48); // m
}
char extract_sqrt3(bigInteger *state) {
  // √3 term has low dexponential grow precision
  // need more term for digit generated
  do {
	  bigInteger_maddi  (state + 3, 2);
	  bigInteger_maddi  (state + 4, 6);
	  bigInteger_mmul   (state + 2, state[3]);
	  bigInteger_mmuladd(state + 0, state[4], state[2]);
	  bigInteger_mmul   (state + 1, state[4]);
	  bigInteger_maddi  (state + 3, 2);
	  bigInteger_maddi  (state + 4, 6);
	  bigInteger_mmul   (state + 2, state[3]);
	  bigInteger_mmulsub(state + 0, state[4], state[2]);
	  bigInteger_mmul   (state + 1, state[4]);
  } while (state[1].count <= state[2].count);
  // extract
	bigInteger_set    (state + 5, state[0]);
	bigInteger_div_mod(state + 5, state[1], state);
  // 10 base
	bigInteger_mmuli (state + 0, 5);
  bigInteger_mshfri(state + 1, 1);
  bigInteger_mmuli (state + 2, 5);
  // simplify
  // bigInteger_mgcd(state + 6, state   , 3);
  // bigInteger_mdiv(state + 0, state[6]);
  // bigInteger_mdiv(state + 1, state[6]);
  // bigInteger_mdiv(state + 2, state[6]);
  return state[5].count ? state[5].items[0] : 0;
}
#endif

#if !(defined(SQRT2_ONLY) || defined(SQRT3_ONLY) || defined(E_ONLY))
/*
 *      2(n!)
 * £ ----------
 *    (2n + 1)!!
 *
 *  a * m      c * n
 * -------- + -------
 *  b * m      b * m
 *
 *  2     2     4      12
 * --- + --- + ---- + -----
 *  1     3     15     105
 *
 * 
 * 0
 * a =  2
 * b =  1
 * c =  2
 * n =  0
 * m =  1
 * pre 10
 * a =  45701632
 * b =  14549535
 * c =  7680
 * n =  10
 * m =  21
 *
 */
void init_pi(bigInteger *state) {
  bigInteger_set_int(state    , 45701632);
  bigInteger_set_int(state + 1, 14549535);
  bigInteger_set_int(state + 2, 7680);
  bigInteger_set_int(state + 3, 10);
  bigInteger_set_int(state + 4, 21);
}
char extract_pi(bigInteger *state) {
  // π term has dexponential grow precision
  // need more term for digit generated
  do {
    bigInteger_mincr(state + 3);										 // ++n
    bigInteger_mmul(state + 2, state[3]);					 // c *= n
    bigInteger_maddi(state + 4, 2);								   // m += 2
    bigInteger_mmuladd(state, state[4], state[2]); // a = a * m + c
    bigInteger_mmul(state + 1, state[4]);					 // b *= m
  } while (state[1].count <= state[2].count);
  // extract
  bigInteger_set    (state + 5, state[0]);
  bigInteger_div_mod(state + 5, state[1], state);
  // 10 base
  bigInteger_mmuli(state    , 10);					 // a *= 10
  bigInteger_mmuli(state + 2, 10);					 // c *= 10
  // simplify
  // bigInteger_mgcd(state + 6, state   , 3);
  // bigInteger_mdiv(state + 0, state[6]);
  // bigInteger_mdiv(state + 1, state[6]);
  // bigInteger_mdiv(state + 2, state[6]);
	return state[5].count ? (char) state[5].items[0] : 0;
}
#endif
/*
 * π = 4*arctan(1)
 * π = 8*arctan(1/(1+√2))
 *
 *
 *
 */






