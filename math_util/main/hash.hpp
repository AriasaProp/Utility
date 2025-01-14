#ifndef _HASH_FUNCTION
#define _HASH_FUNCTION

#include <cstdint>
#include <cstdlib>

void sha256 (const char *, size_t, uint32_t *);

#endif // _HASH_FUNCTION