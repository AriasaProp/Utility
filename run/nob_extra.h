#include <stdint.h>
#include <stdlib.h>
#include <string.h>

char *nob_strndup(const char *str, const size_t len) {
	char *ret = malloc(len+1);
	if (ret) {
		memcpy(ret, str, len);
		ret[len] = 0;
	}
	return ret;
}