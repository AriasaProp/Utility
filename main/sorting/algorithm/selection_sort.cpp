/*

selection sort
# # # .. ..  ..
   ->
evaluate to find minimum each iteration
swap it to the front
*/

#include <cstdint>
#include <cstdlib>
#include <cstring>

static void data_swamp(char*, char*, size_t);

void selection_sort(void *dat, size_t size, size_t bytes, int(*cmp)(const void*,const void*)) {
	if (size < 2) return;
	char *data = (char*)dat;
	// loop right
	for(char *i = data, *en = data + (size * bytes); i < en; i += bytes) {
		//loop left
		char *min = i;
		for (char *j = i + bytes; j < en; j += bytes) {
			if (cmp(j, min) < 0)
				min = j;
		}
		if (min != i) data_swamp(i, min, bytes);
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