#include "image_codec.hpp"

#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <vector>

// masking
#define IMGC_MASKFILTER 0xc0 /* 11000000 */
#define IMGC_MASKVALUE 0x3f  /* 00111111 */

// filter keys
#define IMGC_RUNLENGTH 0x00 /* 00xxxxxx */
#define IMGC_LOOKBACK 0x40  /* 01xxxxxx */

//#define IMGC_?????????		 0x80		/* 10xxxxxx */
#define IMGC_FULLCHANNEL 0x80 /* 10000000 */

//#define IMGC_?????v???		 0xc0		/* 11xxxxxx */

// utf8 : IMGCODEC
const unsigned int HEADER_SIZE = 8;
const unsigned char HEADER_ARRAY[]{0x49, 0x4d, 0x47, 0x43, 0x4f, 0x44, 0x45, 0x43};

// input: data, width pixel, height pixel, channel per pixels ? 3 or 4
unsigned char *image_encode (const unsigned char *pixels, const image_param param, unsigned int *out_byte) {
  if (!pixels) throw "data pixels is null";
  if (!(param.width * param.height * param.channel)) throw "pixels parameters is 0";
  const unsigned char *read_px = pixels, *end_px = pixels + param.width * param.height * param.channel;

  std::vector<unsigned char> write_px;
  // write header 8 bytes
  write_px.insert (write_px.end (), HEADER_ARRAY, HEADER_ARRAY + HEADER_SIZE);
  // write informations 12 bytes
  write_px.insert (write_px.end (), static_cast<const unsigned char *> (&param), static_cast<const unsigned char *> (&param) + sizeof (image_param));
  std::vector<unsigned char> prev_px;
  unsigned char i, run = 0;

  // first pixel
  write_px.insert (write_px.end (), read_px, read_px + param.channel);
  prev_px.insert (prev_px.begin (), read_px, read_px + param.channel);
  read_px += param.channel;

  while (read_px < end_px) {
    // apply some filter encoding
    int cmp_px = memcmp (prev_px.data (), read_px, param.channel);

    if (!cmp_px) {
      if (++run > 63) {
        write_px.push_back (IMGC_RUNLENGTH & (run - 1));
        run = 0;
      }
    } else {
      if (run) {
        write_px.push_back (IMGC_RUNLENGTH & (run - 1));
        run = 0;
      }

      write_px.push_back (IMGC_FULLCHANNEL);
      write_px.insert (write_px.end (), read_px, read_px + param.channel);
      prev_px.insert (prev_px.begin (), read_px, read_px + param.channel);
    }
    read_px += param.channel;
  }
  if (run)
    write_px.push_back (IMGC_RUNLENGTH & (run - 1));

  *out_byte = write_px.size ();
  unsigned char *out = static_cast<unsigned char *> (malloc (*out_byte));
  memcpy (out, write_px.data (), *out_byte);
  return out;
}
// input: data, data length in byte
unsigned char *image_decode (const unsigned char *bytes, const unsigned int byte_len, image_param *param) {
  if (!bytes) throw "data memory is null";
  if (byte_len < (HEADER_SIZE + sizeof (image_param))) throw "byte length is to small";
  const unsigned char *read_px = bytes, *end_px = bytes + byte_len;
  // read header
  if (memcmp (read_px, HEADER_ARRAY, HEADER_SIZE)) throw "header is wrong";
  read_px += HEADER_SIZE;
  // read params
  memcpy (param, read_px, sizeof (image_param));
  read_px += sizeof (image_param);
  // check param validity
  if ((end_px - read_px) < param->channel) throw "data is too small";

  std::vector<unsigned char> prev_px;
  unsigned char *out_px = static_cast<unsigned char *> (malloc (param->width * param->height * param->channel));
  unsigned char *write_px = out_px, *end_out_px = out_px + param->width * param->height * param->channel;

  // first pixel
  memcpy (write_px, read_px, param->channel);
  prev_px.insert (prev_px.begin (), read_px, read_px + param->channel);
  read_px += param->channel;
  write_px += param->channel;

  // read byte
  unsigned char readed, i;

  // next pixel
  while (read_px < end_px) {
    readed = *(read_px++);

    if ((readed & IMGC_MASKFILTER) == IMGC_RUNLENGTH) {
      readed &= IMGC_MASKVALUE;
      readed++;
      do {
        memcpy (write_px, prev_px.data (), param->channel);
        write_px += param->channel;
      } while (--readed);
    } else if (readed == IMGC_FULLCHANNEL) {
      memcpy (write_px, read_px, param->channel);
      prev_px.insert (prev_px.begin (), read_px, read_px + param->channel);
      read_px += param->channel;
      write_px += param->channel;
    } else
      throw "error";
  }

  return write_px;
}

// void release memory
void image_free (unsigned char *f) {
  free ((void *)f);
}