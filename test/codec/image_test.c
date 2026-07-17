#include "common.h"
#include "util/console_out.h"
#include "stb/image_read.h"
#include "stb/local.h"
#include "stb/image_write.h"
#include <sys/stat.h>

const char *Image_Test[] = {
  "data/codec/image/640x426",
};

int main(int UNUSED_ARG(argc), char** UNUSED_ARG(argv)) {
  PRINT_INF("Image test is ");
  char filename[256];
  int channel;
  iter i;
  if ((access("bin/data", F_OK) != 0) && mkdir("bin/data",0775)) {
    printf("fail make bin/data directory!");
    return EXIT_FAILURE;
  }
  if ((access("bin/data/codec", F_OK) != 0) && mkdir("bin/data/codec", 0775)) {
    printf("fail make bin/data/codec directory!");
    return EXIT_FAILURE;
  }
  if ((access("bin/data/codec/image", F_OK) != 0) && mkdir("bin/data/codec/image", 0775)) {
    printf("fail make bin/data/codec/image directory!");
    return EXIT_FAILURE;
  }
  bool wr;
  for (i = 0; i < STACK_ARR_LEN(Image_Test); ++i) {
#define RUN_FRMT(F, X) do {\
    snprintf(filename, 255, "%s." #F, Image_Test[i]); \
    image_bitmap ibitmap = image_read(filename); \
    if (!ibitmap) {                              \
      printf("Image load fail for %s!\n", filename); \
      printf("reason: %s!\n", stb_get_error()); \
      break; \
    } \
    snprintf(filename, 255, "bin/%s." #F, Image_Test[i]); \
    wr = image_write(filename, ibitmap, ImageFile_##X, .quality = 100);\
    image_free(ibitmap);\
    if (!wr) {\
      printf("fail rewrite %s!\n", filename);\
      printf("reason: %s!\n", stb_get_error());\
      break;\
    }\
} while (0)
    RUN_FRMT(png, PNG);
    // RUN_FRMT(bmp, BMP);
    // RUN_FRMT(tga, TGA);
    // RUN_FRMT(jpg, JPG);
    // RUN_FRMT(hdr, HDR);
#undef RUN_FRMT
  }
  
  if (i < STACK_ARR_LEN(Image_Test)) {
    return EXIT_FAILURE;
  }
  printf(GREEN"Success\n"RESET);
  return EXIT_SUCCESS;
}