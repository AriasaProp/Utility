/* *****************************************************************************
 * hash.h v0.0.0000
 * 
 * Hashing algorithm
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef _HASH_FUNCTION_
#define _HASH_FUNCTION_

#include "common.h"

#define HASH128_IN_BYTES  16
#define HASH160_IN_BYTES  20
#define HASH224_IN_BYTES  28
#define HASH256_IN_BYTES  32
#define HASH384_IN_BYTES  48
#define HASH512_IN_BYTES  64

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// return 0 on valid hex string
int  hash_cstr_to_ubyte(ubyte*, const char *,iter);
void hash_ubyte_append_string(String*, const ubyte*, iter);
// run hash on valid input will never fail
void hash_md5         (uint32*, const char *, uint64);
void hash_sha1        (uint32*, const char *, uint64);
void hash_sha224      (uint32*, const char *, uint64);
void hash_sha256      (uint32*, const char *, uint64);
void hash_sha384      (uint64*, const char *, uint64);
void hash_sha512      (uint64*, const char *, uint64);

// void hash_scrypt      (const char *, uint64);
// void hash_randomX256  (const char *, uint64);
// void hash_randomX512  (const char *, uint64);

void hash_crc32_start  (uint32*);
void hash_crc32_append (uint32*,const byte);
void hash_crc32_appends(uint32*,const byte *, iter);
void hash_crc32_end    (uint32*);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _HASH_FUNCTION_
