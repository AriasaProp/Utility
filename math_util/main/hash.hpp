#ifndef _HASH_FUNCTION
#define _HASH_FUNCTION

#include <cstdint>

int sha256(uint32_t *input, int bitlength, uint32_t *outputlocation);

#endif // _HASH_FUNCTION