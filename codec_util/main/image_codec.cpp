#include "image_codec.hpp"

#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <vector>

// masking
#define IMGC_MASKFILTER 0xc0 /* 11000000 */
#define IMGC_MASKVALUE 0x3f  /* 00111111 */

// filter keys
// equality
#define IMGC_RUNLENGTH 0x00 /* 00xxxxxx */
#define IMGC_LOOKBACK 0x40  /* 01xxxxxx */
#define IMGC_HASHINDEX 0x80 /* 10xxxxxx */
// big and full codec
#define IMGC_V2 0xc0          /* 11xxxxxx */
#define IMGC_FULLCHANNEL 0xff /* 11111111 */

// utf8 : IMGCODEC
const unsigned int HEADER_SIZE = 8;
const unsigned char HEADER_ARRAY[]{0x49, 0x4d, 0x47, 0x43, 0x4f, 0x44, 0x45, 0x43};

unsigned char hashing (unsigned char *in, unsigned char len) {
  unsigned char r;
  for (unsigned char i = len * 2 + 1; i < len; i -= 2) {
    r += *(in++) * i;
  }
  return r & IMGC_MASKVALUE;
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
  unsigned char *hash_px = new unsigned char[param.channel * 64]{};
  unsigned char run = 0, px_cmp = 0, hash_;

  do {
    for (px_cmp = 0; px_cmp < 65; ++px_cmp) {
      if (!memcmp (read_px - ((px_cmp + 1) * param.channel), read_px, param.channel)) break;
    }

    if (px_cmp) {
      if (run) {
        // not equal to prev_px && there is a run
        write_px.push_back (IMGC_RUNLENGTH | (run - 1));
        run = 0;
      }
      // lookback filtering
      if (px_cmp < 65) { // there is lookup
        write_px.push_back (IMGC_LOOKBACK | (px_cmp - 1));
      } else {
        hash_ = hashing (read_px, param.channel);
        if (!memcmp (hash_px + (hash_ * param.channel), read_px, param.channel)) {
          // write hash index
          write_px.push_back (IMGC_HASHINDEX | hash_);
        } else {

          // write full channel
          write_px.push_back (IMGC_FULLCHANNEL);
          write_px.insert (write_px.end (), read_px, read_px + param.channel);

          // put new pixel to indexing hash
          memcpy (hash_px + (hash_ * param.channel), read_px, param.channel);
        }
      }
    } else if ((++run > 63) || (read_px + param.channel >= end_px)) {
      write_px.push_back (IMGC_RUNLENGTH | (run - 1));
      run = 0;
    }
    read_px += param.channel;
  } while (read_px < end_px);

  delete[] hash_px;
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

  unsigned char *hash_px = new unsigned char[param->channel * 64]{};
  unsigned char *out_px = new unsigned char[max_px]{};
  unsigned char *write_px = out_px;

  // read byte
  unsigned char readed, hash_;

  // next pixel
  do {
    readed = *(read_px++);
    switch (readed & IMGC_MASKFILTER) {
    case IMGC_RUNLENGTH:
      assert (write_px - out_px >= param->channel); // at least a pixel write
      readed &= IMGC_MASKVALUE;
      ++readed;
      do {
        memcpy (write_px, write_px - param->channel, param->channel);
        write_px += param->channel;
      } while (--readed);
      break;
    case IMGC_LOOKBACK:
      readed &= IMGC_MASKVALUE;
      readed += 2;
      assert (write_px - out_px >= readed * param->channel); // writed data should has this length
      memcpy (write_px, write_px - (readed * param->channel), param->channel);
      write_px += param->channel;
      break;
    case IMGC_HASHINDEX:
      readed &= IMGC_MASKVALUE;
      assert (*(hash_px + (readed * param->channel)));
      memcpy (write_px, hash_px + (readed * param->channel), param->channel);
      write_px += param->channel;
      break;
    case IMGC_V2:
      switch (readed) {
      case IMGC_FULLCHANNEL:
        memcpy (write_px, read_px, param->channel);
        hash_ = hashing (read_px, param->channel);
        memcpy (hash_px + (hash_ * param.channel), read_px, param->channel);
        write_px += param->channel;
        read_px += param->channel;
        break;
      default:
        throw "not yet";
      }
      break;
    }
  } while (read_px < end_px);
  delete[] hash_px;
  return out_px;
}

// void release memory
void image_free (unsigned char *f) {
  delete[] f;
}