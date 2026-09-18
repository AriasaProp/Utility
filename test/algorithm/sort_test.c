#include "algorithm/sort.h"
#include "util/console_out.h"
#include "array/dstring.h"
#include "common.h"

#include <time.h>

#define STYPE       int
#define STYPE_BYTES sizeof(STYPE)
// some sorting algorithm need more optimization
// it's too slow for builtin qsort
// DATA_SIZE should be greater than 2^7 (randLess doing 6 right shift)
#define DATA_SIZE   8643
#define REPEATE     4
// #define DATA_SIZE   12
#define DATA_BYTES  (DATA_SIZE * STYPE_BYTES)

static int data_compare(const void *a, const void *b) {
  const STYPE A = *CAST(const STYPE*)a;
  const STYPE B = *CAST(const STYPE*)b;
  return (A > B) - (A < B);
}

static void scramble_sorted  (STYPE *data) {
  for (int i = 0; i < DATA_SIZE; ++i)
    data[i] = i;
}
static void scramble_randLess(STYPE *data) {
  int i, j, k, t;
  for (i = 0; i < DATA_SIZE; ++i)
    data[i] = i;
  for (j = CAST(int)imath_sqrt(imath_sqrt(DATA_SIZE)), i = j; i < DATA_SIZE; i += j) {
    k = imath_iabs(rand() % DATA_SIZE);
    t       = data[i];
    data[i] = data[k];
    data[k] = t;
  }
}
static void scramble_randHalf(STYPE *data) {
  int i, k, t;
  for (i = 0; i < DATA_SIZE; ++i)
    data[i] = i;
  for (i = 0; i < DATA_SIZE; i += 2) {
    k = imath_iabs(-2 & (rand() % DATA_SIZE));
    t       = data[i];
    data[i] = data[k];
    data[k] = t;
  }
}
static void scramble_randHead(STYPE *data) {
  int i, j, k, t;
  for (i = 0; i < DATA_SIZE; ++i)
    data[i] = i;
  for (i = 0, j = DATA_SIZE >> 1; i < j; ++i) {
    k = imath_iabs(rand() % j);
    t       = data[i];
    data[i] = data[k];
    data[k] = t;
  }
}
static void scramble_randTail(STYPE *data) {
  int i, j, k, t;
  for (i = 0; i < DATA_SIZE; ++i)
    data[i] = i;
  j = DATA_SIZE >> 1;
  data += j;
  j = DATA_SIZE - j;
  for (i = 0; i < j; ++i) {
    k = imath_iabs(rand() % j);
    t       = data[i];
    data[i] = data[k];
    data[k] = t;
  }
}
static void scramble_similar (STYPE *data) {
  for (iter i = 0; i < DATA_SIZE; ++i)
    data[i] = imath_iabs(rand() % DATA_SIZE) & ~3;
}
static void scramble_randFull(STYPE *data) {
  for (iter i = 0, j = DATA_SIZE << 4; i < DATA_SIZE; ++i)
    data[i] = rand() % j;
}
static void scramble_mount   (STYPE *data) {
  for (int i = 0; i < DATA_SIZE; ++i)
    data[i] = i * (DATA_SIZE - i);
}
static void scramble_valley  (STYPE *data) {
  for (int j = DATA_SIZE >> 1, i = -j; i <= j; ++i)
    data[i] = i * i;
}
static void scramble_flip    (STYPE *data) {
  for (iter i = 0; i < DATA_SIZE; ++i)
    data[i] = DATA_SIZE - i;
}

static const struct {
  const char *name;
  bool rep;
  void (*scram)(STYPE *);
} scramble_algo[] = {
  {.name = " sorted " , .rep =false , .scram = scramble_sorted   },
  {.name = "randLess" , .rep = true , .scram = scramble_randLess },
  {.name = "randHalf" , .rep = true , .scram = scramble_randHalf },
  {.name = "randHead" , .rep = true , .scram = scramble_randHead },
  {.name = "randTail" , .rep = true , .scram = scramble_randTail },
  {.name = " similar" , .rep = true , .scram = scramble_similar  },
  {.name = "randFull" , .rep = true , .scram = scramble_randFull },
  {.name = " mount  " , .rep =false , .scram = scramble_mount    },
  {.name = " valley " , .rep =false , .scram = scramble_valley   },
  {.name = "  flip  " , .rep =false , .scram = scramble_flip     },
};
static const struct {
  const char *name;
  void (*srt)(void*, iter, iter, compare_funct);
} sort_algo[] = {
  // { .name = "Stooge"  ,.srt = sort_stooge },
  // { .name = "Cycle"   ,.srt = sort_cycle  },
  // { .name = "Pancke"  ,.srt = sort_panck  },
  // { .name = "Brick"   ,.srt = sort_brick  },
  { .name = " Tim "   ,.srt = sort_tim    },
  // { .name = "Bubble"  ,.srt = sort_bubble },
  // { .name = "Shaker"  ,.srt = sort_shaker },
  // { .name = "Insert"  ,.srt = sort_insert },
  // { .name = "Select"  ,.srt = sort_select },
  // { .name = " Heap "  ,.srt = sort_heap   },
  // { .name = "Merge"   ,.srt = sort_merge  },
  // { .name = "Shell"   ,.srt = sort_shell  },
  // { .name = "Intro"   ,.srt = sort_intro  },
  // { .name = "Extra"   ,.srt = sort_extra  },
  // { .name = "Quick"   ,.srt = sort_quick  },
};

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
	srand(time(0));
	PRINT_INF("Sorting Test for %d data is ", DATA_SIZE);
	// randomize $(STYPE) data
  STYPE
    *data = CAST(STYPE*)malloc(DATA_BYTES * 3),
    *sort_data = data + DATA_SIZE,
    *sorted_data = sort_data + DATA_SIZE;
	if (!data) {
	  printf(RED "fail to allocate memory, for %lu bytes" RESET "\n", DATA_BYTES * 2);
	  return EXIT_FAILURE;
	}
  int result = EXIT_FAILURE;
	iter i, j, k, r, rend;
	printf("Start .... ");
  for (i = 0; i < STACK_ARR_LEN(scramble_algo); ++i) {
    for (r = 0, rend = !scramble_algo[i].rep + scramble_algo[i].rep * REPEATE; r < rend; ++r) {
  	  // scramble
      scramble_algo[i].scram(data);
    	memcpy(sorted_data, data, DATA_BYTES);
    	qsort(sorted_data, DATA_SIZE, STYPE_BYTES, data_compare);
    	memcpy(sort_data, data, DATA_BYTES);
	    for (j = 0; j < STACK_ARR_LEN(sort_algo); ++j) {
      	// start
        printf("\rScrambling %9s, x%01zu, %6s Sort", scramble_algo[i].name, r, sort_algo[j].name);
      	fflush(stdout);
      	sort_algo[j].srt(sort_data, DATA_SIZE, STYPE_BYTES, data_compare);
      	// proof sorted
        if (memcmp(sort_data, sorted_data, DATA_BYTES)) {
    	    printf(RED "Failure" RESET " doing sort with %s Sort to sort %s data!\n", sort_algo[j].name, scramble_algo[i].name);
  	      printf("data:");
    	    for (k = 0; k < DATA_SIZE; ++k) {
    	      printf("%d,",data[k]);
    	    }
  	      printf("\n");
  	      printf("fail:");
    	    for (k = 0; k < DATA_SIZE; ++k) {
    	      printf("%d,",sort_data[k]);
    	    }
  	      printf("\n");
  	      printf("crrc:");
    	    for (k = 0; k < DATA_SIZE; ++k) {
    	      printf("%d,",sorted_data[k]);
    	    }
  	      printf("\n");
          goto end;
        }
  	  }
	  }
  }
  printf("\r" GREEN "Success" RESET "\n");
  result = EXIT_SUCCESS;
end:
	free(data);
  return result;
}
