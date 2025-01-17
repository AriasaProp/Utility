#ifndef _HASH_FUNCTION
#define _HASH_FUNCTION

#include <cstdint>
#include <cstdlib>
#include <iostream>

union hash256 {
  char b[32];
  uint32_t i[8];
};
union hash512 {
  char b[64];
  uint32_t i[16];
};
std::ostream &operator<< (std::ostream &, const hash256);
std::ostream &operator<< (std::ostream &, const hash512);

hash256 sha256 (const char *, uint64_t);

hash256 randomX256 (const char *, uint64_t);
hash512 randomX512 (const char *, uint64_t);

#endif // _HASH_FUNCTION