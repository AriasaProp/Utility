#include "image_codec.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <vector>

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
const unsigned char HEADER_ARRAY[]{0x49, 0x4d, 0x47, 0x43, 0x4f, 0x44, 0x45, 0x43};
const size_t HEADER_SIZE = sizeof (HEADER_ARRAY);

unsigned char hashing (const unsigned char *in, unsigned int len) {
  unsigned char r = 0;
  for (unsigned int i = 0; i < len; ++i)
    r ^= (in[i] & 63) ^ (in[i] >> 6);
  return r;
}

// input: data, width pixel, height pixel, channel per pixels ? 3 or 4
unsigned char *image_encode (const unsigned char *pixels, const image_param param, unsigned int *out_byte) {
  if (!pixels)
    throw "data pixels is null";
  const unsigned int max_px = param.width * param.height * param.channel;
  if (!max_px)
    throw "pixels parameters is 0";
  const unsigned char *read_px = pixels, *end_px = pixels + max_px;

  std::vector<unsigned char> write_px;
  write_px.reserve (max_px);
  // write header 8 bytes
  write_px.insert (write_px.end (), HEADER_ARRAY, HEADER_ARRAY + HEADER_SIZE);
  // write informations 9 bytes
  write_px.insert (write_px.end (), reinterpret_cast<const unsigned char *> (&param), reinterpret_cast<const unsigned char *> (&param) + sizeof (image_param));
  // buffer for caching pixels difference
  int *db = (int *)calloc (sizeof (int), param.channel);
  unsigned char
      // buffer for indexing pixels
      *index = (unsigned char *)calloc (64, param.channel),
      // counting run length encoding, store temporary hash
      *index_view, temp1, temp2;
  // look ahead with compare most longer length
  int saved_lookahead = -1, saved_len_lookahead = -1;
  // write first pixel
  temp2 = hashing (read_px, param.channel);
  write_px.insert (write_px.end (), read_px, read_px + param.channel);
  memcpy (index + (temp2 * param.channel), read_px, param.channel);
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
      write_px.push_back (((saved_lookahead & 0x7) << 4) | (saved_len_lookahead & 0xf));
      read_px += param.channel * (saved_len_lookahead + 1);
      saved_len_lookahead = -1;
      saved_lookahead = -1;
    } else {
      // index compare
      temp2 = hashing (read_px, param.channel);
      index_view = index + (temp2 * param.channel);
      if (memcmp (index_view, read_px, param.channel)) {
        bool diff = true;
        // find difference
        for (temp2 = 0; (temp2 < param.channel) && diff; ++temp2) {
          db[temp2] = *(read_px + temp2 - param.channel) - *(read_px + temp2);
          diff = ((!(param.channel & 1) && (param.channel == (temp2 + 1))) ? 128 : 8) > std::abs (db[temp2]);
        }
        // write difference
        if (diff) {
          temp2 = 0;
          write_px.push_back (IMGC_DIFF | std::abs (db[temp2]) | (8 * (db[temp2] < 0)));
          ++temp2;
          while ((temp2 + 2) <= param.channel) {
            temp1 = std::abs (db[temp2]) | (8 * (db[temp2] < 0));
            ++temp2;
            temp1 |= (std::abs (db[temp2]) | (8 * (db[temp2] < 0))) << 4;
            ++temp2;
            write_px.push_back (temp1);
          }
          if (!(param.channel & 1))
            write_px.push_back (std::abs (db[temp2]) | (128 * (db[temp2] < 0)));
        } else {
          // write full channel
          write_px.push_back (IMGC_FULLCHANNEL);
          write_px.insert (write_px.end (), read_px, read_px + param.channel);
        }
        memcpy (index_view, read_px, param.channel);
      } else {
        write_px.push_back (IMGC_HASHINDEX | temp2);
      }
      read_px += param.channel;
    }
  }
  free (db);
  free (index);
  *out_byte = write_px.size ();
  unsigned char *out = (unsigned char *)malloc (*out_byte);
  memcpy (out, write_px.data (), *out_byte);
  return out;
}
// input: data, data length in byte
unsigned char *image_decode (const unsigned char *bytes, const unsigned int byte_len, image_param *param) {
  if (!bytes)
    throw "data memory is null";
  if (byte_len < (HEADER_SIZE + sizeof (image_param)))
    throw "byte length is to small";
  const unsigned char *read_px = bytes, *end_px = bytes + byte_len;
  // read header
  if (memcmp (read_px, HEADER_ARRAY, HEADER_SIZE))
    throw "header is wrong";
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
          throw "not yet imgc diff";
        case 3:
          switch (temp1) {
          case IMGC_FULLCHANNEL:
            memcpy (write_px, read_px, param->channel);
            break;
          default:
            throw "not yet full channel";
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