#ifndef _HASH_FUNCTION
#define _HASH_FUNCTION

#include <cstdint>
#include <cstdlib>

union hash256 {
  char b[32];
  uint32_t i[8];
};
std::ostream &operator<< (std::ostream &, const hash256);

hash256 sha256 (const char *, size_t);

#endif // _HASH_FUNCTION