/*

bubble sort
# # .. .. .. .. # # # 
 ->             
do compare from first to last (repeat)
each compare it's neighbours
each repeat ignore n time last item because is proofen as bigger

*/

#include <cstdint>
#include <cstdlib>
#include <cstring>

static void data_swamp(char*, char*, size_t);

void bubble_sort(void *dat, size_t size, size_t bytes, int(*cmp)(const void*,const void*)) {
	if (size < 2) return;
	char *data = (char*)dat;
	// put right
	for(size_t i = size - 1; i; --i) {
		for (size_t j = 0; j < i; ++j) {
			//loop right
			if (cmp(data + j * bytes, data + (j + 1) * bytes) > 0)
				data_swamp(data + j * bytes, data + (j + 1) * bytes, bytes);
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