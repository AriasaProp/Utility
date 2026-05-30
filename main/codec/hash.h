/* *****************************************************************************
 * hash.h v0.0.0000
 * 
 * Hashing decoder
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef _HASH_FUNCTION_
#define _HASH_FUNCTION_

#include "common.h"

#define HASH256_IN_HEXS   64
#define HASH256_IN_BYTES  32
#define HASH256_IN_SHORTS 16
#define HASH256_IN_INT32   8
#define HASH256_IN_INT64   4

#define HASH512_IN_HEXS  128
#define HASH512_IN_BYTES  64
#define HASH512_IN_SHORTS 32
#define HASH512_IN_INT32  16
#define HASH512_IN_INT64   8

typedef union {
  ubyte b[HASH256_IN_BYTES];
  ushrt s[HASH256_IN_SHORTS];
  uint32 i[HASH256_IN_INT32];
  uint64 l[HASH256_IN_INT64];
} hash256;
typedef union {
  ubyte b[HASH512_IN_BYTES];
  ushrt s[HASH512_IN_SHORTS];
  uint32 i[HASH512_IN_INT32];
  uint64 l[HASH512_IN_INT64];
} hash512;


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// return 0 on valid hex string
int  hash_cstr_to_h256(hash256*, const char *);
int  hash_cstr_to_h512(hash512*, const char *);

void hash_h256_append_string(String*, const hash256);
void hash_h512_append_string(String*, const hash512);
// run hash on valid input will never fail
void hash_sha256      (hash256*, const char *, uint64);

// void hash_scrypt      (const char *, uint64);
// void hash_randomX256  (const char *, uint64);
// void hash_randomX512  (const char *, uint64);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _HASH_FUNCTION_
