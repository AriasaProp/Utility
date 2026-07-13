#ifndef _STB_IMAGE_WRITE_INCLUDED_
#define _STB_IMAGE_WRITE_INCLUDED_

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
  bool flip_vertically;
  bool flip_horizontally;
  ubyte quality;
} image_file_opt;

typedef struct {
  iter len, cap;
  ubyte *data;
} image_file;

typedef struct {
  uint32 w, h;
  ushrt bpc, chnl;
  ubyte *data;
} image_bitmap;

#ifdef __cplusplus
extern "C" {
#endif

typedef void stbi_write_func(void*,void*,iter);
bool       image_write_opt        (char const*,             const image_bitmap, image_file_format, image_file_opt);
bool       image_write_to_func_opt(stbi_write_func*, void*, const image_bitmap, image_file_format, image_file_opt);
image_file image_write_to_mem_opt (                         const image_bitmap, image_file_format, image_file_opt);
#define image_write_to_func(funct, user, bitmap, file_format, ... ) image_write_to_funct_opt((funct), (user), (bitmap), (file_format), CLIT(image_file_opt){ __VA_ARGS__ })
#define image_write_to_mem(bitmap, file_format, ... )               image_write_to_mem_opt  (                 (bitmap), (file_format), CLIT(image_file_opt){ __VA_ARGS__ })
#define image_write(filename, bitmap, file_format, ... )            image_write_opt         ((filename),      (bitmap), (file_format), CLIT(image_file_opt){ __VA_ARGS__ })

#ifdef __cplusplus
}
#endif
  
#endif // _STB_IMAGE_WRITE_INCLUDED_
