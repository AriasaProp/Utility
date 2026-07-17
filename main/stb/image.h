#ifndef _IMAGE_INCLUDED_
#define _IMAGE_INCLUDED_

#include "common.h"

typedef enum {
  ImageFile_Invalid = 0,
  ImageFile_JPG,
  ImageFile_PNG,
  ImageFile_BMP,
  ImageFile_TGA,
  ImageFile_HDR,
} image_file_format;
typedef struct {
  void *user;
  void (*write)(void*,const void*,iter);
} image_write_func;
typedef struct {
  void *user;
  iter (*read)(void*,void*,iter); // fill 'data' with 'size' bytes.  return number of bytes actually read
  void (*skip)(void*,int); // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
  void (*rewind)(void*);
  bool (*eof)(void*);
} image_read_func;

typedef struct {
  bool flip_vertically;
  bool flip_horizontally;
  ubyte quality;
} image_opt

typedef struct {
  iter len, cap;
  ubyte *data;
} image_file;

typedef struct {
  uint32 w, h;
  ushrt bpc, chnl;
} image_info;
typedef struct {
  image_info inf;
  ubyte *data;
} image_bitmap;


#ifdef __cplusplus
extern "C" {
#endif

image_info image_readinfo          (char const*);
image_info image_readinfo_from_func(image_read_func);
image_info image_readinfo_from_mem (const image_file);
image_bitmap image_read_opt          (char const*,      image_opt);
image_bitmap image_read_from_func_opt(image_read_func,  image_opt);
image_bitmap image_read_from_mem_opt (const image_file, image_opt);
bool       image_write_opt        (char const*,       const image_bitmap, image_file_format, image_opt);
bool       image_write_to_func_opt(image_write_func*, const image_bitmap, image_file_format, image_opt);
image_file image_write_to_mem_opt (                   const image_bitmap, image_file_format, image_opt);

void image_bitmap_free(image_bitmap*);
void image_file_free(image_file*);
  
#define image_read_from_func(func, ...) image_read_from_func_opt((func) , CLIT(image_opt){ __VA_ARGS__ })
#define image_read_from_mem(mem, ...)   image_read_from_mem_opt ((mem)  , CLIT(image_opt){ __VA_ARGS__ })
#define image_read(fpath, ...)          image_read_opt          ((fpath), CLIT(image_opt){ __VA_ARGS__ })
#define image_write_to_func(funct, bitmap, file_format, ...) image_write_to_funct_opt((funct), (bitmap), (file_format), CLIT(image_opt){ __VA_ARGS__ })
#define image_write_to_mem(bitmap, file_format, ...)         image_write_to_mem_opt  (         (bitmap), (file_format), CLIT(image_opt){ __VA_ARGS__ })
#define image_write(fpath, bitmap, file_format, ...)         image_write_opt         ((fpath), (bitmap), (file_format), CLIT(image_opt){ __VA_ARGS__ })

#ifdef __cplusplus
}
#endif
  
#endif // _IMAGE_INCLUDED_
