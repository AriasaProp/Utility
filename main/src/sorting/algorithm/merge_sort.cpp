/*

merge sort (not yet)
.. .. .. .. # # # # .. .. .. ..
 { .. .. .. # } { # .. .. .. } 
.. { .. # } { .. # } { .. # } ..
            .....
 .. .. { # # } { # # # } .. .. ..
 .. .. .. .. # # # # .. ..   . 
 
 split group until end up to 1
 
 combine back to the parent group (neighbour) then sort it (repeat)

*/

#include <cstdint>
#include <cstdlib>
#include <cstring>

static void data_shift_front(char *, char *, size_t);
static void inner_recursion(char *, size_t, size_t, int(*)(const void*,const void*));

void merge_sort(void *dat, size_t size, size_t bytes, int(*cmp)(const void*,const void*)) {
	if (size < 2) return;
	char *data = (char*)dat;
	inner_recursion(data, size, bytes, cmp);
}

static void inner_recursion(char *data, size_t size, size_t bytes, int(*cmp)(const void*,const void*)) {
	if (size < 4) {
		for (size_t i = 0; i < size; ++i) {
			for (size_t j = 0; j < size; ++j) {
				
			}
		}
		return;
	}
	size_t j = size / 2;
	inner_recursion(data, j, bytes, cmp);
	inner_recursion(data + j * bytes, size - j, bytes, cmp);
	// data was split and sorted each part
	char *a,*b;
	for (size_t i = 0; (i < j) && (j < size); ++i) {
		a = data + i * bytes, b = data + j * bytes;
		if (cmp(a, b) > 0) {
			// first split bigger
			data_shift_front (a, b, bytes);
			++j;
		}
	}
}

static void data_shift_front(char *a, char *b, size_t l) {
	char t;
	for (size_t i = 0; i < l; ++i) {
		t = a[i];
		a[i] = b[i];
		b[i] = t;
	}
}
 


