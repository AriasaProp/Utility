#include "image_codec.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// filter keys

// equality
#define IMGC_LOOKAHEAD 0x00 /* 0xxxxxxx */

#define IMGC_V1 0x80 /* 1xxxxxxx */
// equality from previous
#define IMGC_HASHINDEX 0x80 /* 10xxxxxx */
#define IMGC_DIFF 0xc0      /* 1100xxxx */

// big and full codec
#define IMGC_FULLCHANNEL 0xff /* 11111111 */

// utf8 : IMGCODEC
constexpr char *HEADER_ARRAY = "IMGCODEC";
constexpr size_t HEADER_SIZE = strlen (HEADER_ARRAY);

unsigned char hashing (const unsigned char *in, unsigned int len) {
  unsigned char r = 0;
  for (unsigned int i = 0; i < len; ++i)
    r ^= (in[i] & 63) ^ (in[i] >> 6);
  return r;
}
char err_msg[2048];
#define THROW(x) memcpy(err_msg, x, strlen(x)), return NULL

// input: data, width pixel, height pixel, channel per pixels ? 3 or 4
unsigned char *image_encode (const unsigned char *pixels, const image_param param, unsigned int *out_byte) {
  if (!pixels)
    THROW("data pixels is null");
  const size_t max_px = param.width * param.height * param.channel;
  if (!max_px)
    THROW("pixels parameters is 0");
  const unsigned char *read_px = pixels, *end_px = pixels + max_px;

  // buffer for caching pixels difference
  int *db = (int *)calloc (sizeof (int), param.channel);
  unsigned char
      // buffer for write pixels
  		*write_px = (unsigned char *)malloc(max_px + 17),
      // buffer for indexing pixels
      *index = (unsigned char *)calloc (64, param.channel),
      // counting run length encoding, store temporary hash
      *index_view, temp1, *cw = write_px;
  // write header 8 bytes
  memcpy(cw, HEADER_ARRAY, HEADER_SIZE);
  cw += HEADER_SIZE;
  // write informations 9 bytes
  memcpy(cw, reinterpret_cast<const unsigned char *> (&param), sizeof (image_param));
  cw += sizeof (image_param);
  // look ahead with compare most longer length
  int saved_lookahead = -1, saved_len_lookahead = -1;
  // write first pixel
  temp1 = hashing (read_px, param.channel);
  memcpy (cw, read_px, param.channel);
  cw += param.channel;
  memcpy (index + (temp1 * param.channel), read_px, param.channel);
  read_px += param.channel;

  while (read_px < end_px) {
    for (const unsigned char *c = std::max (pixels, read_px - (0x8 * param.channel)); c < read_px; c += param.channel) {
      if (!memcmp (c, read_px, param.channel)) {
        const unsigned char
            *d = read_px,
            *e = c,
            *dend = std::min (end_px, read_px + (0xf * param.channel));
        do {
          d += param.channel, e += param.channel;
        } while ((d < dend) && (!memcmp (d, e, param.channel)));

        int len = (e - c) / param.channel - 1;
        if (saved_len_lookahead < len) {
          saved_lookahead = (read_px - c) / param.channel - 1;
          saved_len_lookahead = len;
        }
      }
    }
    if (saved_lookahead > -1) {
      *(cw++) = ((saved_lookahead & 0x7) << 4) | (saved_len_lookahead & 0xf);
      read_px += param.channel * (saved_len_lookahead + 1);
      saved_len_lookahead = -1;
      saved_lookahead = -1;
    } else {
      // index compare
      temp1 = hashing (read_px, param.channel);
      index_view = index + (temp1 * param.channel);
      if (memcmp (index_view, read_px, param.channel)) {
        int diff = 1;
        // find difference
        for (temp1 = 0; (temp1 < param.channel) && diff; ++temp1) {
          db[temp1] = *(read_px + temp1 - param.channel) - *(read_px + temp1);
          diff = ((!(param.channel & 1) && (param.channel == (temp1 + 1))) ? 128 : 8) > std::abs (db[temp1]);
        }
        // write difference
        if (diff) {
          temp1 = 0;
          *(cw++) = IMGC_DIFF | std::abs (db[temp1]) | (8 * (db[temp1] < 0));
          
          ++temp1;
          while ((temp1 + 2) <= param.channel) {
            *cw = std::abs (db[temp1]) | (8 * (db[temp1] < 0));
            ++temp1;
            *cw |= (std::abs (db[temp1]) | (8 * (db[temp1] < 0))) << 4;
            ++temp1;
            ++cw;
          }
          if (!(param.channel & 1))
            *(cw++) = std::abs (db[temp1]) | (128 * (db[temp1] < 0));
        } else {
          // write full channel
          *(cw++) = IMGC_FULLCHANNEL;
          memcpy (cw, read_px, param.channel);
          cw += param.channel;
        }
        memcpy (index_view, read_px, param.channel);
      } else {
        *(cw++) = IMGC_HASHINDEX | temp1;
      }
      read_px += param.channel;
    }
  }
  free (db);
  free (index);
  *out_byte = (unsigned int)(cw - write_px);
  unsigned char *out = (unsigned char *)malloc (*out_byte);
  memcpy (out, write_px, *out_byte);
  free (write_px);
  return out;
}
// input: data, data length in byte
unsigned char *image_decode (const unsigned char *bytes, const unsigned int byte_len, image_param *param) {
  if (!bytes)
    THROW("data memory is null");
  if (byte_len < (HEADER_SIZE + sizeof (image_param)))
    THROW("byte length is to small");
  const unsigned char *read_px = bytes, *end_px = bytes + byte_len;
  // read header
  if (memcmp (read_px, HEADER_ARRAY, HEADER_SIZE))
    THROW("header is wrong");
  read_px += HEADER_SIZE;
  // read params
  memcpy (param, read_px, sizeof (image_param));
  unsigned int max_px = param->width * param->height * param->channel;
  read_px += sizeof (image_param);
  unsigned char
      // buffer for indexing pixels
      *index = (unsigned char *)calloc (64, param->channel),
      // output
      *out_px = (unsigned char *)malloc (max_px),
      // write state
          *write_px = out_px,
      // temporary read byte, hashing
      temp1 = IMGC_FULLCHANNEL, temp2;
  int itemp;
  do {
    if (temp1 & 0x80) { /* 1000 0000 */
      // IMGC_V1
      if (temp1 & 0x40) {              /* 0100 0000 */
        switch ((temp1 & 0x30) >> 4) { /* 0011 0000 */
        case 0:                        // IMGC_DIFF
          temp1 &= 0xf;
          itemp = temp1 & 7;
          itemp *= (temp1 & 8) ? 1 : -1;
          *write_px = *(write_px - param->channel) + itemp;
          temp2 = 1;
          while ((temp2 + 2) <= param->channel) {
            temp1 = *(read_px++);
            itemp = temp1 & 7;
            itemp *= (temp1 & 8) ? 1 : -1;
            *(write_px + temp2) = *(write_px + temp2 - param->channel) + itemp;
            temp1 >>= 4, ++temp2;
            itemp = temp1 & 7;
            itemp *= (temp1 & 8) ? 1 : -1;
            *(write_px + temp2) = *(write_px + temp2 - param->channel) + itemp;
            ++temp2;
          }
          if (!(param->channel & 1)) {
            temp1 = *(read_px++);
            itemp = temp1 & 127;
            itemp *= (temp1 & 128) ? 1 : -1;
            *(write_px + temp2) = *(write_px + temp2 - param->channel) + itemp;
          }
          break;
        default:
          THROW("not yet imgc diff");
        case 3:
          switch (temp1) {
          case IMGC_FULLCHANNEL:
            memcpy (write_px, read_px, param->channel);
            break;
          default:
            THROW("not yet full channel");
          }
          read_px += param->channel;
          break;
        }
        // all v1 data stored into index
        temp2 = hashing (write_px, param->channel);
        memcpy (index + (temp2 * param->channel), write_px, param->channel);
        write_px += param->channel;
      } else {
        // IMGC_HASHINDEX
        temp1 &= 0x3f; /* 01xx xxxx */
        memcpy (write_px, index + (temp1 * param->channel), param->channel);
        write_px += param->channel;
      }
    } else {
      // IMGC_LOOKAHEAD
      temp2 = (temp1 & 0x70) >> 4; /* 0111 0000 */
      temp1 &= 0xf;                /* 0000 1111 */
      ++temp1;
      ++temp2;
      do {
        memcpy (write_px, write_px - (param->channel * temp2), param->channel);
        write_px += param->channel;
      } while (--temp1);
    }
    temp1 = *(read_px++);
  } while (read_px <= end_px);

  free (index);
  return out_px;
}
// void release memory
void image_free (unsigned char *f) {
  free ((void *)f);
}