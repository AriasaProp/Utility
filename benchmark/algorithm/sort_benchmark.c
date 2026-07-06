#include "algorithm/sort.h"
#include "util/profiling.h"
#include "util/console_out.h"
#include "common.h"
#include "array/dstring.h"

#define STYPE       float
#define DATA_RANDOM imath_rand_float ()
// some sorting algorithm need more optimization
// it's too slow for builtin qsort
// #define DATA_SIZE   2118047
#define DATA_SIZE   200789
#define DATA_BYTES  (DATA_SIZE * sizeof(STYPE))

typedef void (*sort_funct)(void*, iter, iter, compare_funct);
static int data_compare (const void*,const void*);
static iter cmp_call;
int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  dstring main_str = NULL;
  int proofen = 0;
	iter i;
	// randomize $(STYPE) data
  void *temp_data = util_malloc(DATA_BYTES * 2);
	if (!temp_data) {
	  PRINT_ERR("fail to allocate trial memory, for %lu bytes \n", DATA_BYTES * 2);
	  return EXIT_FAILURE;
	}
  STYPE *data_r = (CAST(STYPE*) temp_data) + DATA_SIZE;
	for (i = 0; i < DATA_SIZE; ++i)
		data_r[i] = DATA_RANDOM;
	PRINT_INF("Sorting Test! %d data \n"
		"|     Name     |     time     |  compare  | \n"
		"|--------------|--------------|-----------| \n"
	, DATA_SIZE);
  pr_time c_time;
#define CASE(N, F) do {\
  cmp_call = 0;\
  util_memcpy(temp_data, data_r, DATA_BYTES);\
  c_time = profiling_current_time();\
  F(temp_data, DATA_SIZE, sizeof(STYPE), data_compare);\
  c_time = profiling_time_since(c_time);\
  STYPE *ret = CAST(STYPE*) temp_data;\
  for (i = 1, proofen = 1; proofen && (i < DATA_SIZE); ++i) \
    proofen &= (ret[i - 1] <= ret[i]);\
  dstring_clean(main_str);\
  profiling_append_as_time2(&main_str, c_time);\
  printf("\r| %12s | %12s | %09zu | %s \n", N, main_str, cmp_call, (proofen?(GREEN "√" RESET):(RED "x" RESET)));\
  if (!proofen) goto end;\
} while (0)
  
  CASE("Quick{b}",qsort);
#if DATA_SIZE < 210
  CASE("Stooge"  ,sort_stooge);
#endif
#if DATA_SIZE < 1310
  CASE("Pancke"  ,sort_panck);
  CASE("Gnome"   ,sort_gnome);
  CASE("Brick"   ,sort_brick);
  CASE("Bubble"  ,sort_bubble);
#endif
#if DATA_SIZE < 4070
  CASE("Shaker"  ,sort_shaker);
  CASE("Insert"  ,sort_insert);
  CASE("Heap"    ,sort_heap);
#endif
#if DATA_SIZE < 180380
  CASE("Select"  ,sort_select);
  CASE("Shell"   ,sort_shell);
#endif
#if DATA_SIZE < 304875
  CASE("Merge"   ,sort_merge);
#endif
  CASE("Intro"   ,sort_intro);
  CASE("Intro_"  ,sort_intro_opt);
  CASE("Quick"   ,sort_quick);
#undef CASE
end:
	dstring_free(&main_str);
	util_memfree(temp_data);
  return EXIT_SUCCESS;
}

static int data_compare(const void* a, const void* b) {
  const STYPE A = *(const STYPE*)a;
  const STYPE B = *(const STYPE*)b;
  ++cmp_call;
  return (A > B) - (A < B);
}
