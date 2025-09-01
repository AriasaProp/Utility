#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <ctime>

#include "sorting/sorter.hpp"

static void run_sort (const char *, const void *, size_t, void (*)(void*, size_t, size_t, int(*)(const void*, const void*)));
static int data_compare (const void*,const void*);
static int sorting_proof (const void *, size_t);

static void generate_random_data(void *dat, size_t size) {
	srand((unsigned int)time(NULL));
	float *ret = (float*) dat;
	for (size_t i = 0; i < size; ++i) {
		ret[i] = (float)((double)rand() / (double)RAND_MAX / 2.0) + 0.5f;
	}
}


void sorting_test() {
	unsigned long data_size = 1000;
	void *data_r = malloc (data_size * sizeof(float));
	generate_random_data(data_r, data_size);
	printf(
		"Sorting Test! \n"
		"|           Name           ||         time         || \n"
		"|--------------------------||----------------------|| \n"
	);
	
	run_sort("Qsort_builtin", data_r, data_size, qsort);
	run_sort("Insertion", data_r, data_size, insertion_sort);
	run_sort("Bubble", data_r, data_size, bubble_sort);
	run_sort("Selection", data_r, data_size, selection_sort);
	run_sort("Merge", data_r, data_size, merge_sort);

	free (data_r);
}

static void run_sort (const char *name, const void *data, size_t sz, void (*s)(void*, size_t, size_t, int(*cmp)(const void*, const void*))) {
	clock_t c = clock();
	void *temp_data = malloc (sz * sizeof(float));
	memcpy(temp_data, data, sz * sizeof(float));
	s(temp_data, sz, sizeof(float), data_compare);
	int isSorted = sorting_proof(temp_data, sz);
	free (temp_data);
	unsigned long t = clock() - c; // clock
	
	printf("| %24s || %014lu clock ||%c \n", name, t, isSorted?'Y':'N');
}

static int data_compare(const void* a, const void* b) {
  float arg1 = *(const float*)a;
  float arg2 = *(const float*)b;
  return (arg1 < arg2) * -1 + (arg1 > arg2) * 1;
}
static int sorting_proof(const void *dat, size_t l) {
	float *ret = (float*) dat;
	for (size_t i = 1; i < l; ++i) {
		if (ret[i - 1] > ret[i]) return 0;
	}
	return 1;
}
