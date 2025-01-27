#ifndef IMAGE_CODEC_
#define IMAGE_CODEC_

// width pixel, height pixel, channel per pixels ? 3 or 4
typedef struct {
  unsigned int width, height;
  unsigned char channel;
} image_param;
char err_msg[2048];
#ifdef __cplusplus
extern "C" {
#endif
// input: data, image param in, out len in bytes
unsigned char *image_encode (const unsigned char *, const image_param, unsigned int *);
// input: data, data length in byte, image params out
unsigned char *image_decode (const unsigned char *, const unsigned int, image_param *);
// void release memory
void image_free (unsigned char *);
#ifdef __cplusplus
}
#endif
#endif // IMAGE_CODEC_