#include "algorithm/sort.h"
#include "util/profiling.h"
#include "util/console_out.h"
#include "common.h"


#define STYPE       float
#define DATA_RANDOM imath_rand_float ()
// some sorting algorithm need more optimization
// it's too slow for builtin qsort
// #define DATA_SIZE   123860
#define DATA_SIZE   120
#define DATA_BYTES  (DATA_SIZE * sizeof(STYPE))

typedef void (*sort_funct)(void*, iter, iter, compare_funct);
static int data_compare (const void*,const void*);

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  String main_str = NULL;
  int proofen = 0;
	iter i, j;
	// randomize $(STYPE) data
  void *temp_data = util_malloc(DATA_BYTES * 2);
	if (!temp_data) {
	  PRINT_ERR("fail to allocate trial memory, for %lu bytes \n", DATA_BYTES * 2);
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
//     printf(" %.1f -",(CAST(float*)data_r)[j]);
//   printf("\n");
	PRINT_INF("Sorting Test! %d data \n"
		"|     Name     |     time     | \n"
		"|--------------|--------------| \n"
	, DATA_SIZE);
  const struct {
    const char *name; sort_funct srt;
  } sort_algo[] = {
  	{ .name = "Quick{b}", .srt = qsort,},
#if DATA_SIZE < 410
  	{ .name = "Stooge"  , .srt = sort_stooge,},
#endif
#if DATA_SIZE < 2310
  	{ .name = "Pancke"  , .srt = sort_panck,},
  	{ .name = "Gnome"   , .srt = sort_gnome,},
  	{ .name = "Brick"   , .srt = sort_brick,},
  	{ .name = "Bubble"  , .srt = sort_bubble,},
#endif
#if DATA_SIZE < 6070
  	{ .name = "Shaker"  , .srt = sort_shaker,},
  	{ .name = "Insert"  , .srt = sort_insert,},
  	{ .name = "Heap"    , .srt = sort_heap,},
#endif
#if DATA_SIZE < 12380
  	{ .name = "Select"  , .srt = sort_select,},
  	{ .name = "Shell"   , .srt = sort_shell,},
#endif
#if DATA_SIZE < 100075
  	{ .name = "Merge"   , .srt = sort_merge,},
#endif
  	{ .name = "Intro"   , .srt = sort_intro,},
  	{ .name = "Quick"   , .srt = sort_quick,},
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
  	for (j = 1, proofen = 1; proofen && (j < DATA_SIZE); ++j) {
  	  proofen &= (ret[j - 1] <= ret[j]);
  	}
	  // result log
	  string_clean(main_str);
	  profiling_append_as_time2(&main_str, c_time);
	  fflush(stdout);
	  printf("\r| %12s | %12s | %s \n", sort_algo[i].name, main_str, (proofen?(GREEN "√" RESET):(RED "x" RESET)));
  }
	string_free(&main_str);
	util_memfree(temp_data);
  return EXIT_SUCCESS;
}
static int data_compare(const void* a, const void* b) {
  const STYPE A = *(const STYPE*)a;
  const STYPE B = *(const STYPE*)b;
  return (A > B) - (A < B);
}

