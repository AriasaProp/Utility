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
#define IMGC_FULLCHANNEL 0xc0 /* 11000000 */

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
  write_px.insert (write_px.end (), reinterpret_cast<const unsigned char *> (&param), reinterpret_cast<const unsigned char *> (&param) + sizeof (image_param));
  unsigned char *buff_px = new unsigned char[param.channel * 64]{};
  unsigned char *prev_px = new unsigned char[param.channel]{};
  unsigned char i, run = 0;

  while (read_px < end_px) {
    // apply some filter encoding
    // compare to previous pixel
    int cmp_px = memcmp (prev_px, read_px, param.channel);

    if (!cmp_px) {
      // if equal, add run_length
      if (++run > 63) {
        // write down when pass the limit
        write_px.push_back (IMGC_RUNLENGTH | (run - 1));
        run = 0;
      }
    } else {
      if (run) {
        // write down when run_length end
        write_px.push_back (IMGC_RUNLENGTH | (run - 1));
        run = 0;
      }
      bool notfound = true;
      for (i = 0; notfound && i < 64; ++i) {
        notfound = memcmp (buff_px + (i * param.channel), read_px, param.channel);
      }
      if (!notfound) {
      	write_px.push_back (IMGC_LOOKBACK | i);
      } else {
        // write code for full channel
        write_px.push_back (IMGC_FULLCHANNEL);
        // write full channel for current pixel
        write_px.insert (write_px.end (), read_px, read_px + param.channel);
        // put previous pixel to buffer for look up
        memmove (buff_px + param.channel, buff_px, param.channel * 63);
        memcpy (buff_px, prev_px, param.channel);
        // put current pixel to previous
        memcpy (prev_px, read_px, param.channel);
      }
    }
    read_px += param.channel;
  }
  delete[] prev_px;
  delete[] buff_px;
  // write down when pixel out
  if (run)
    write_px.push_back (IMGC_RUNLENGTH | (run - 1));

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
  read_px += sizeof (image_param);
  // check param validity
  if ((end_px - read_px) < param->channel) throw "data is too small";

  unsigned char *buff_px = new unsigned char[param->channel * 64]{};
  unsigned char *prev_px = new unsigned char[param->channel]{};
  unsigned char *out_px = new unsigned char[param->width * param->height * param->channel]{};
  unsigned char *write_px = out_px, *end_out_px = out_px + param->width * param->height * param->channel;

  // read byte
  unsigned char readed, i;

  // next pixel
  while (read_px < end_px) {
    readed = *(read_px++);

    switch (readed & IMGC_MASKFILTER) {
    case IMGC_RUNLENGTH:
      readed &= IMGC_MASKVALUE;
      readed++;
      do {
        memcpy (write_px, prev_px, param->channel);
        write_px += param->channel;
      } while (--readed);
      break;
    case IMGC_LOOKBACK:
      readed &= IMGC_MASKVALUE;
      memcpy (write_px, buff_px + (readed * param->channel), param->channel);
      write_px += param->channel;
      break;
    case IMGC_V1:
      throw "not yet";
    case IMGC_V2:
      switch (readed) {
      case IMGC_FULLCHANNEL:
        memcpy (write_px, read_px, param->channel);
        write_px += param->channel;

        memmove (buff_px + param->channel, buff_px, param->channel * 63);
        memcpy (buff_px, prev_px, param->channel);

        memcpy (prev_px, read_px, param->channel);
        read_px += param->channel;
        break;
      default:
        throw "not yet";
      }
      break;
    }
  }
  delete[] prev_px;
  delete[] buff_px;
  return write_px;
}

// void release memory
void image_free (unsigned char *f) {
  delete[] f;
}