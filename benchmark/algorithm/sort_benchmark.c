#include "algorithm/sort.h"
#include "util/profiling.h"
#include "util/console_out.h"
#include "common.h"
#include "array/dstring.h"

#include <math.h> // nanl
#include <time.h>
#include <fcntl.h>

#define STYPE        int
#define STYPE_BYTES  sizeof(STYPE)

// some sorting algorithm need more optimization
// it's too slow for builtin qsort
#define REPEATE      9
// #define DATA_SIZE    300
#define DATA_SIZE    11003
// #define DATA_SIZE    12074
#define DATA_BYTES   (DATA_SIZE * STYPE_BYTES)


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
  {.name = "similar " , .rep = true , .scram = scramble_similar  },
  {.name = "randFull" , .rep = true , .scram = scramble_randFull },
  {.name = " mount  " , .rep =false , .scram = scramble_mount    },
  {.name = " valley " , .rep =false , .scram = scramble_valley   },
  {.name = "  flip  " , .rep =false , .scram = scramble_flip     },
};
static const struct {
  const char *name;
  void (*srt)(void*, iter, iter, compare_funct);
} sort_algo[] = {
  { .name = "quick "  ,.srt = qsort       },
#if DATA_SIZE < 2143
  { .name = "Stooge"  ,.srt = sort_stooge },
  { .name = " Cycle"  ,.srt = sort_cycle  },
  { .name = "Bubble"  ,.srt = sort_bubble },
  { .name = "Shaker"  ,.srt = sort_shaker },
#endif
#if (DATA_SIZE > 2143) && (DATA_SIZE < 6747)
  { .name = "Pancke"  ,.srt = sort_panck  },
  { .name = "Brick "  ,.srt = sort_brick  },
  { .name = "Select"  ,.srt = sort_select },
  { .name = " Heap "  ,.srt = sort_heap   },
#endif
#if (DATA_SIZE > 6747) && (DATA_SIZE < 11249)
  { .name = " Tim  "  ,.srt = sort_tim    },
  { .name = "Insert"  ,.srt = sort_insert },
  { .name = "Intro "  ,.srt = sort_intro  },
  { .name = "Quick "  ,.srt = sort_quick  },
#endif
#if DATA_SIZE > 11249
  { .name = "Merge "  ,.srt = sort_merge  },
  { .name = "Extra "  ,.srt = sort_extra  },
  { .name = "Shell "  ,.srt = sort_shell  },
#endif
};

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  srand(time(0));
  // File write
  int fout = open("data/algorithm/sort_report.txt", O_WRONLY | O_TRUNC | O_DIRECT | O_CREAT, S_IRWXU | S_IRWXO | S_IRWXG);
  if (fout < 0) {
	  PRINT_ERR("fail to open file");
	  return EXIT_FAILURE;
  }
	// randomize $(STYPE) data
  STYPE
    *data = CAST(STYPE*)malloc(DATA_BYTES * 3),
    *sort_data = data + DATA_SIZE,
    *sorted_data = sort_data + DATA_SIZE;
	if (!data) {
	  close(fout);
	  PRINT_ERR("fail to allocate trial memory, for %lu bytes \n", DATA_BYTES * 2);
	  return EXIT_FAILURE;
	}
  iter i, j, k, r;
  iter nT = 0;
  pr_time pm;
  ldouble timef;
  dprintf(fout, " Sorter ");
	for (i = 0; i < STACK_ARR_LEN(sort_algo); ++i)
    dprintf(fout, " %7s", sort_algo[i].name);
  dprintf(fout,"\n");
  // close(fout);
  printf("Starting ................................");
	for (i = 0; i < STACK_ARR_LEN(scramble_algo); ++i) {
    dprintf(fout,"%8s", scramble_algo[i].name);
    for (j = 0; j < STACK_ARR_LEN(sort_algo); ++j) {
	    timef = 0.0;
      for (r = 0, k = !scramble_algo[i].rep + scramble_algo[i].rep * REPEATE; r < k; ++r) {
	      printf("\rSort %8s data with %6s Sort in %01zu times  ",scramble_algo[i].name, sort_algo[j].name, r);
	      fflush(stdout);
    	  // scramble
        scramble_algo[i].scram(data);
    	  // duplicate
    	  if (j) {
          memcpy(sorted_data, data, DATA_BYTES);
          qsort(sorted_data, DATA_SIZE, STYPE_BYTES, data_compare);
    	  }
        memcpy(sort_data, data, DATA_BYTES);
        // sort with timing
        pm = profiling_current_time();
        sort_algo[j].srt(sort_data, DATA_SIZE, STYPE_BYTES, data_compare);
        pm = profiling_time_since(pm);
        if (j && memcmp(sorted_data, sort_data, DATA_BYTES))
	        break;
        else
          timef += CAST(ldouble)pm / 1000000.0;
	    }
      if (r < k)
        dprintf(fout, "   nan  "), ++nT;
      else
        dprintf(fout, " %.1Le", timef / CAST(ldouble)k);
	  }
    dprintf(fout,"\n");
	}
	close(fout);
	printf("\r");
  PRINT_INF("Done! with %zu error\n", nT);
	free(data);
  return EXIT_SUCCESS;
}



