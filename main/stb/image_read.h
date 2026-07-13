#ifndef _STB_IMAGE_READ_INCLUDED_
#define _STB_IMAGE_READ_INCLUDED_

#include "common.h"

typedef struct {
  int (*read)(void *user, char *data, int size); // fill 'data' with 'size' bytes.  return number of bytes actually read
  void (*skip)(void *user, int n);               // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
  bool (*eof)(void *user);                        // returns nonzero if we are at end of file/data
} stbi_io_callbacks;

#ifdef __cplusplus
extern "C" {
#endif

ubyte *stbi_read_from_memory(ubyte const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
ubyte *stbi_read_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);
ubyte *stbi_read(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);

#ifndef STBI_NO_GIF
ubyte *stbi_read_gif_from_memory(ubyte const *buffer, int len, int **delays, int *x, int *y, int *z, int *comp, int req_comp);
#endif

////////////////////////////////////
//
// 16-bits-per-channel interface
//

ushrt *stbi_read_16_from_memory(ubyte const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
ushrt *stbi_read_16_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);
ushrt *stbi_read_16(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);

////////////////////////////////////
//
// float-per-channel interface
//
#ifndef STBI_NO_LINEAR
float *stbi_readf_from_memory(ubyte const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
float *stbi_readf_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);
float *stbi_readf(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
#endif

#ifndef STBI_NO_HDR
void stbi_hdr_to_ldr_gamma(float gamma);
void stbi_hdr_to_ldr_scale(float scale);
#endif // STBI_NO_HDR

#ifndef STBI_NO_LINEAR
void stbi_ldr_to_hdr_gamma(float gamma);
void stbi_ldr_to_hdr_scale(float scale);
#endif // STBI_NO_LINEAR

// stbi_is_hdr is always defined, but always returns false if STBI_NO_HDR
int stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user);
int stbi_is_hdr_from_memory(ubyte const *buffer, int len);
int stbi_is_hdr(char const *filename);

// get image dimensions & components without fully decoding
int stbi_info_from_memory(ubyte const *buffer, int len, int *x, int *y, int *comp);
int stbi_info_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp);
int stbi_is_16_bit_from_memory(ubyte const *buffer, int len);
int stbi_is_16_bit_from_callbacks(stbi_io_callbacks const *clbk, void *user);
int stbi_info(char const *filename, int *x, int *y, int *comp);
int stbi_is_16_bit(char const *filename);

// ZLIB client - used by PNG, available for other purposes
char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen);
char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header);
char *stbi_zlib_decode_malloc(const char *buffer, int len, int *outlen);
int stbi_zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
int stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

#ifdef __cplusplus
}
#endif

#endif // _STB_IMAGE_READ_INCLUDED_
