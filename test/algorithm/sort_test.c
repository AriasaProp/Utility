#include "algorithm/sort.h"
#include "util/profiling.h"
#include "util/console_out.h"
#include "common.h"

#include <stdio.h>
#include <unistd.h>

#define STYPE       float
#define DATA_RANDOM imath_rand_float ()
// some sorting algorithm need more optimization
// it's too slow for builtin qsort
#define DATA_SIZE   800
#define DATA_BYTES  (DATA_SIZE * sizeof(STYPE))

typedef void (*sort_funct)(void*, iter, iter, compare_funct);
static int data_compare (const void*,const void*);

String main_str = NULL;
int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  int proofen = 0;
	iter i, j;
	// randomize $(STYPE) data
	printf("Sorting Test! %d data \n", DATA_SIZE);
  void *temp_data = malloc(DATA_BYTES * 2);
	if (!temp_data) {
	  printf("fail to allocate trial memory, for %lu bytes \n", DATA_BYTES * 2);
	  return EXIT_FAILURE;
	}
  STYPE *data_r = (CAST(STYPE*) temp_data) + DATA_SIZE;
  // data_r[0] = 3.0f;
  // data_r[1] = -2.0f;
  // data_r[2] = -7.0f;
  // data_r[3] = 1.0f;
  // data_r[4] = 4.0f;
  // data_r[5] = 13.0f;
	for (i = 0; i < DATA_SIZE; ++i)
		data_r[i] = DATA_RANDOM;
// 	for (j = 0; j < DATA_SIZE; ++j)
//       printf(" %.1f -",(CAST(float*)data_r)[j]);
//     printf("\n");
	printf(
		"|     Name     |     time     | \n"
		"|--------------|--------------| \n"
	);
  const struct {
    const char *name; sort_funct srt;
  } sort_algo[] = {
  	{ .name = "Quick{b}", .srt = qsort,      },
#if 1
#if DATA_SIZE < 220
  	{ .name = "Stooge"  , .srt = sort_stooge, },
#endif
#if DATA_SIZE < 1280
  	{ .name = "Gnome"   , .srt = sort_gnome, },
  	{ .name = "Quick"   , .srt = sort_quick,      },
  	{ .name = "Shaker"  , .srt = sort_shaker, },
  	{ .name = "Intro"   , .srt = sort_intro,},
  	{ .name = "Brick"   , .srt = sort_brick, },
  	{ .name = "Bubble"  , .srt = sort_bubble,},
#endif
#if DATA_SIZE < 4700
  	{ .name = "Pancke"  , .srt = sort_panck,},
  	{ .name = "Select"  , .srt = sort_select,},
  	{ .name = "Heap"    , .srt = sort_heap,  },
#endif
#if DATA_SIZE < 7380
  	{ .name = "Insert"  , .srt = sort_insert,},
#endif
  	{ .name = "Shell"   , .srt = sort_shell, },
  	{ .name = "Merge"   , .srt = sort_merge, },
#endif
  };
  pr_time c_time;
	for (i = 0; i < STACK_ARR_LEN(sort_algo); ++i) {
  	// start
  	util_memcpy(temp_data, data_r, DATA_BYTES);
  	// profiling 
  	c_time = profiling_current_time();
  	sort_algo[i].srt(temp_data, DATA_SIZE, sizeof(STYPE), data_compare);
  	c_time = profiling_time_since(c_time);
  	// proof sorted
  	STYPE *ret = CAST(STYPE*) temp_data;
  	for (j = 1, proofen = 1; proofen && (j < DATA_SIZE); ++j) proofen &= (ret[j - 1] <= ret[j]);
	  // result log
	  string_clean(main_str);
	  profiling_append_as_time2(&main_str, c_time);
	  fflush(stdout);
	  printf("\r| %12s | %12s | %s \n", sort_algo[i].name, main_str, (proofen?(GREEN "√" RESET):(RED "x" RESET)));
  }
	string_free(&main_str);
	util_memfree (temp_data);
  return EXIT_SUCCESS;
}
static int data_compare(const void* a, const void* b) {
  static pr_time dt = 0;
  if (!dt || profiling_as_fsec(profiling_time_since(dt)) > 0.5f) {
    dt = profiling_current_time();
    string_clean(main_str);
    profiling_append_as_time2(&main_str, dt);
    fflush(stdout);
    printf("\r|  %23s  |", main_str);
  }
  const STYPE A = *(const STYPE*)a;
  const STYPE B = *(const STYPE*)b;
  return (A > B) - (A < B);
}

