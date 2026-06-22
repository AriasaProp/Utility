/* *****************************************************************************
 * image.h v0.0.0000
 * 
 * image codec
 * mostly stolen from http://nothings.org/stb
 * 
 * 
 * *****************************************************************************/

#ifndef _IMAGE_CODEC_
#define _IMAGE_CODEC_

#include "common.h"

typedef struct {
  ushrt width;
  ushrt height;
  ushrt channel;
  ushrt bits_channel;
} image_info;

typedef struct {
  image_info info;
  void *image_data;
} image;
// as image preferences
typedef enum : int {
  // preferences type
  IMAGE_PREF_INVALID = 0,
  IMAGE_PREF_FLIPY   = 1,
  IMAGE_PREF_FLIPX   = 1 << 1,
} image_pref_flag;
// as user input code
typedef enum : int {
  // writing type 
  IMAGE_WRITE_INVALID = 0x100,
  IMAGE_WRITE_PNG     = 0x101,
  IMAGE_WRITE_JPNG    = 0x102,
} image_cin;
// as generated output code
typedef enum : int {
  IMAGE_SUCCESS = 0,
  IMAGE_BAD_SIGN,
  IMAGE_HEADER_INVALID,
  IMAGE_FAILURE_FILE,
} image_cout;

typedef struct {
  void *user;
  void(void*)(*restart);
  int(void*,byte*,iter)(*read);
  int(void*,byte*,iter)(*write);
  int(void*,iter)(*seek);
  int(void*)(*eof);
} image_funct;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void  image_switch_pref         (const image_pref_flag);

image_info image_loadinfo       (image_funct, image_cout*);
image_info image_loadinfo_mem   (void*, iter, image_cout*);
image_info image_loadinfo_file  (const char*, image_cout*);

image      image_loadimage      (image_funct, image_cout*);
image      image_loadimage_mem  (void*, iter, image_cout*);
image      image_loadimage_file (const char*, image_cout*);

image_cout image_write     (image_funct,const image);
image_cout image_write_mem (void*, iter,const image);
image_cout image_write_file(const char*,const image);

void image_free(image*);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _IMAGE_CODEC_