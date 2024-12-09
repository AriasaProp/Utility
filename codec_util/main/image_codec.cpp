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

#define IMGC_V1 0x80 /* 10xxxxxx */

#define IMGC_V2 0xc0          /* 11xxxxxx */
#define IMGC_FULLCHANNEL 0xff /* 11111111 */

// utf8 : IMGCODEC
const unsigned int HEADER_SIZE = 8;
const unsigned char HEADER_ARRAY[]{0x49, 0x4d, 0x47, 0x43, 0x4f, 0x44, 0x45, 0x43};

// input: data, width pixel, height pixel, channel per pixels ? 3 or 4
unsigned char *image_encode (const unsigned char *pixels, const image_param param, unsigned int *out_byte) {
  if (!pixels) throw "data pixels is null";
  const unsigned int max_px = param.width * param.height * param.channel;
  if (!max_px) throw "pixels parameters is 0";
  const unsigned char *read_px = pixels, *end_px = pixels + max_px;

  std::vector<unsigned char> write_px;
  write_px.reserve (max_px);
  // write header 8 bytes
  write_px.insert (write_px.end (), HEADER_ARRAY, HEADER_ARRAY + HEADER_SIZE);
  // write informations 9 bytes
  write_px.insert (write_px.end (), reinterpret_cast<const unsigned char *> (&param), reinterpret_cast<const unsigned char *> (&param) + sizeof (image_param));
  unsigned char *prev_px = new unsigned char[param.channel * 65]{};
  unsigned char run = 0, px_cmp = 0;

  do {
    for (px_cmp = 0; px_cmp < 65; ++px_cmp)
      if (!memcmp (prev_px + (px_cmp * param.channel), read_px, param.channel)) break;

    // run length filtering
    if (
        ((!px_cmp) && ((++run > 63) || (read_px + param.channel >= end_px))) || // equal to prev_px && (run length over limit 64 - 1 or pixel read gonna end)
        (px_cmp && run)                                                         // not equal to prev_px && there is a run
    ) {
      write_px.push_back (IMGC_RUNLENGTH | (run - 1));
      run = 0;
    }

    // lookback filtering
    if (px_cmp) {
      if (px_cmp < 65) { // there is lookup
        write_px.push_back (IMGC_LOOKBACK | (px_cmp - 1));
      } else {
        // write full channel
        write_px.push_back (IMGC_FULLCHANNEL);
        write_px.insert (write_px.end (), read_px, read_px + param.channel);
        memmove (prev_px + param.channel, prev_px, param.channel * 64);
        memcpy (prev_px, read_px, param.channel);
      }
    }
    read_px += param.channel;
  } while (read_px < end_px);

  delete[] prev_px;
  *out_byte = write_px.size ();
  unsigned char *out = new unsigned char[*out_byte];
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
  unsigned int max_px = param->width * param->height * param->channel;
  read_px += sizeof (image_param);

  unsigned char *prev_px = new unsigned char[param->channel * 65]{};
  unsigned char *out_px = new unsigned char[max_px]{};
  unsigned char *write_px = out_px;

  // read byte
  unsigned char readed;

  // next pixel
  do {
    readed = *(read_px++);
    switch (readed & IMGC_MASKFILTER) {
    case IMGC_RUNLENGTH:
      readed &= IMGC_MASKVALUE;
      ++readed;
      do {
        memcpy (write_px, prev_px, param->channel);
        write_px += param->channel;
      } while (--readed);
      break;
    case IMGC_LOOKBACK:
      readed &= IMGC_MASKVALUE;
      ++readed;
      memcpy (write_px, prev_px + (readed * param->channel), param->channel);
      write_px += param->channel;
      break;
    case IMGC_V1:
      throw "not yet";
    case IMGC_V2:
      switch (readed) {
      case IMGC_FULLCHANNEL:
        memcpy (write_px, read_px, param->channel);
        write_px += param->channel;
        memmove (prev_px + param->channel, prev_px, param->channel * 64);
        memcpy (prev_px, read_px, param->channel);
        read_px += param->channel;
        break;
      default:
        throw "not yet";
      }
      break;
    }
  } while (read_px < end_px);
  delete[] prev_px;
  return out_px;
}

// void release memory
void image_free (unsigned char *f) {
  delete[] f;
}