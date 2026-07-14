#ifndef _ZLIB_INCLUDED_
#define _ZLIB_INCLUDED_

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

byte *zlib_decode_malloc_guesssize(const byte *buffer, int len, int initial_size, int *outlen);
byte *zlib_decode_malloc_guesssize_headerflag(const byte *buffer, int len, int initial_size, int *outlen, int parse_header);
byte *zlib_decode_malloc(const byte *buffer, int len, int *outlen);
int zlib_decode_buffer(byte *obuffer, int olen, const byte *ibuffer, int ilen);

byte *zlib_decode_noheader_malloc(const byte *buffer, int len, int *outlen);
int zlib_decode_noheader_buffer(byte *obuffer, int olen, const byte *ibuffer, int ilen);

ubyte *zlib_encode(ubyte *data, int data_len, int *out_len, int quality);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _ZLIB_INCLUDED_