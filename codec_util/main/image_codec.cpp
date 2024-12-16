#include "image_codec.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <vector>

// filter keys
// equality
#define IMGC_LOOKAHEAD 0x00 /* 0xxxxxxx */

// big and full codec
#define IMGC_V1 0xc0          /* 1xxxxxxx */
#define IMGC_HASHINDEX 0x40   /* 10xxxxxx */
#define IMGC_FULLCHANNEL 0xff /* 11111111 */

// utf8 : IMGCODEC
const unsigned int HEADER_SIZE = 8;
const unsigned char HEADER_ARRAY[]{0x49, 0x4d, 0x47, 0x43, 0x4f, 0x44, 0x45, 0x43};

static const unsigned int primes[]{3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
unsigned char hashing (const unsigned char *in, unsigned int len) {
  unsigned char r = 0;
  for (unsigned int i = 0; i < len; ++i) {
    r += *(in + i) * primes[i % 12];
  }
  return r % 64;
}

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
  // buffet for indexing pixels
  unsigned char
      *index = new unsigned char[64 * param.channel]{},
      // counting run length encoding, store temporary hash
      *index_view, run = 0, h_;
  // look ahead with compare most longer length
  int saved_lookahead = -1, saved_len_lookahead = -1;

  // write first pixel
  h_ = hashing (read_px, param.channel);
  write_px.insert (write_px.end (), read_px, read_px + param.channel);
  memcpy (index + (h_ * param.channel), read_px, param.channel);
  read_px += param.channel;

  while (read_px < end_px) {
    for (const unsigned char *c = std::max (pixels, read_px - (8 * param.channel)); c < read_px; c += param.channel) {
      if (!memcmp (c, read_px, param.channel)) {
        const unsigned char *d = read_px;
        const unsigned char *e = c;
        const unsigned char *dend = std::min (end_px, read_px + (0xf * param.channel));
        do {
          d += param.channel, e += param.channel;
        } while ((d < dend) && (!memcmp (d, e, param.channel)));
        d -= param.channel;
        e -= param.channel;

        int len = (e - c) / param.channel;
        if (saved_len_lookahead < len) {
          saved_lookahead = (read_px - c) / param.channel;
          saved_len_lookahead = len;
        }
      }
    }
    if (saved_lookahead > 0) {
      write_px.push_back ((((saved_lookahead - 1) & 0x7) << 4) | (saved_len_lookahead & 0xf));
      read_px += param.channel * (saved_len_lookahead + 1);
      saved_len_lookahead = -1;
      saved_lookahead = -1;
    } else {
      // index compare
      h_ = hashing (read_px, param.channel);
      index_view = index + (h_ * param.channel);
      if (memcmp (index_view, read_px, param.channel)) {
        // write full channel
        write_px.push_back (IMGC_FULLCHANNEL);
        write_px.insert (write_px.end (), read_px, read_px + param.channel);
        memcpy (index_view, read_px, param.channel);
      } else {
        write_px.push_back (IMGC_HASHINDEX | h_);
      }
      read_px += param.channel;
    }
  }
  delete[] index;
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
  unsigned char
      // buffer for indexing pixels
      *index = new unsigned char[64 * param->channel]{},
      // output
      *out_px = new unsigned char[max_px]{},
      // write state
          *write_px = out_px,
      // temporary read byte, hashing
      val1, val2;

  // write first pixel
  memcpy (write_px, read_px, param->channel);
  val2 = hashing (read_px, param->channel);
  memcpy (index + (val2 * param->channel), read_px, param->channel);
  write_px += param->channel;
  read_px += param->channel;

  // next pixel
  do {
    val1 = *(read_px++);
    if (val1 & 0x80) { /* 1000 0000 */
      // IMGC_V1
      if (val1 & 0x40) { /* 0100 0000 */
        switch (val1) {
        case IMGC_FULLCHANNEL:
          memcpy (write_px, read_px, param->channel);
          break;
        default:
          throw "not yet";
        }
        // all v1 data stored into index
        val2 = hashing (read_px, param->channel);
        memcpy (index + (val2 * param->channel), read_px, param->channel);
        write_px += param->channel;
        read_px += param->channel;
      } else {
        // IMGC_HASHINDEX
        val1 &= IMGC_MASKVALUE;
        memcpy (write_px, index + (val1 * param->channel), param->channel);
        write_px += param->channel;
      }
    } else {
      // IMGC_LOOKAHEAD
      val2 = (val1 & 0x70) >> 4; /* 0111 0000 */
      val1 &= 0xf;               /* 0000 1111 */
      ++val1;
      ++val2;
      do {
        memcpy (write_px, write_px - (param->channel * val2), param->channel);
        write_px += param->channel;
      } while (--val1);
    }
  } while (read_px < end_px);

  delete[] index;
  return out_px;
}

// void release memory
void image_free (unsigned char *f) {
  delete[] f;
}