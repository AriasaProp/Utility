/*

insertion sort
# # # .. ..  ..
   ->
evaluate from second item to end
.. .. .. .. # # # .. ..
             <-
each evaluation do compare with left item
as well as small item placed into the left
*/

#include <cstdint>
#include <cstdlib>
#include <cstring>

static void data_swamp(char*, char*, size_t);

void insertion_sort(void *dat, size_t size, size_t bytes, int(*cmp)(const void*,const void*)) {
	if (size < 2) return;
	char *data = (char*)dat;
	// loop right
	for(char *i = data + bytes, *en = data + (size * bytes); i < en; i += bytes) {
		//loop left
		for (char *j = i; j > data; j -= bytes) {
			if (cmp(j, j - bytes) < 0) data_swamp(j, j - bytes, bytes);
			else break;
		}
	}
}

static void data_swamp(char *a,char *b, size_t l) {
	char t;
	for (size_t i = 0; i < l; ++i) {
		t = a[i];
		a[i] = b[i];
		b[i] = t;
	}
}