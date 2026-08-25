/*
 * public domain zlib decode    v0.2  Sean Barrett 2006-11-18
 *    simple implementation
 *      - all input must be provided in an upfront buffer
 *      - all output is written to a single output buffer (can malloc/realloc)
 *    performance
 *      - fast huffman
 */

#include "stb/zlib.h"
#include "stb/local.h"
static bool zlib__err(const char *msg) {
  stb_set_error("zlib: %s", msg);
  return false;
}
// fast-way is faster to check than jpeg huffman, but slow way is slower
#define STBI__ZFAST_BITS 9 // accelerate all cases in default tables
#define STBI__ZFAST_MASK ((1 << STBI__ZFAST_BITS) - 1)
#define STBI__ZNSYMS     288 // number of symbols in literal/length alphabet

// zlib-style huffman encoding
// (jpegs packs from left, zlib from right, so can't share code)
typedef struct {
  ushrt fast[1 << STBI__ZFAST_BITS];
  ushrt firstcode[16];
  int maxcode[17];
  ushrt firstsymbol[16];
  ubyte size[STBI__ZNSYMS];
  ushrt value[STBI__ZNSYMS];
} zlib__zhuffman;
inline static int zlib__bitreverse16(int n) {
  n = ((n & 0xAAAA) >> 1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >> 2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >> 4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
  return n;
}
inline static int zlib__bit_reverse(int v, int bits) {
  ASSERT(bits <= 16);
  // to bit reverse n bits, reverse 16 and shift
  // e.g. 11 bits, bit reverse and shift away 5
  return zlib__bitreverse16(v) >> (16 - bits);
}
static int zlib__zbuild_huffman(zlib__zhuffman *z, const ubyte *sizelist, int num) {
  int i, k = 0;
  int code, next_code[16], sizes[17];

  // DEFLATE spec for generating codes
  memset(sizes, 0, sizeof(sizes));
  memset(z->fast, 0, sizeof(z->fast));
  for (i = 0; i < num; ++i)
    ++sizes[sizelist[i]];
  sizes[0] = 0;
  for (i = 1; i < 16; ++i)
    if (sizes[i] > (1 << i))
      return zlib__err("bad sizes");
  code = 0;
  for (i = 1; i < 16; ++i) {
    next_code[i] = code;
    z->firstcode[i] = (ushrt)code;
    z->firstsymbol[i] = (ushrt)k;
    code = (code + sizes[i]);
    if (sizes[i])
      if (code - 1 >= (1 << i))
        return zlib__err("bad codelengths");
    z->maxcode[i] = code << (16 - i); // preshift for inner loop
    code <<= 1;
    k += sizes[i];
  }
  z->maxcode[16] = 0x10000; // sentinel
  for (i = 0; i < num; ++i) {
    int s = sizelist[i];
    if (s) {
      int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
      ushrt fastv = (ushrt)((s << 9) | i);
      z->size[c] = (ubyte)s;
      z->value[c] = (ushrt)i;
      if (s <= STBI__ZFAST_BITS) {
        int j = zlib__bit_reverse(next_code[s], s);
        while (j < (1 << STBI__ZFAST_BITS)) {
          z->fast[j] = fastv;
          j += (1 << s);
        }
      }
      ++next_code[s];
    }
  }
  return 1;
}
// zlib-from-memory implementation for PNG reading
//    because PNG allows splitting the zlib stream arbitrarily,
//    and it's annoying structurally to have PNG call ZLIB call PNG,
//    we require PNG read all the IDATs and combine them into a single
//    memory buffer
typedef struct {
  ubyte *zbuffer, *zbuffer_end;
  int num_bits;
  int hit_zeof_once;
  uint32 code_buffer;

  byte *zout;
  byte *zout_start;
  byte *zout_end;
  int z_expandable;

  zlib__zhuffman z_length, z_distance;
} zlib__zbuf;
inline static int zlib__zeof(zlib__zbuf *z) {
  return (z->zbuffer >= z->zbuffer_end);
}
inline static ubyte zlib__zget8(zlib__zbuf *z) {
  return zlib__zeof(z) ? 0 : *z->zbuffer++;
}
static void zlib__fill_bits(zlib__zbuf *z) {
  do {
    if (z->code_buffer >= (1U << z->num_bits)) {
      z->zbuffer = z->zbuffer_end; /* treat this as EOF so we fail. */
      return;
    }
    z->code_buffer |= (uint)zlib__zget8(z) << z->num_bits;
    z->num_bits += 8;
  } while (z->num_bits <= 24);
}
inline static uint zlib__zreceive(zlib__zbuf *z, int n) {
  uint k;
  if (z->num_bits < n)
    zlib__fill_bits(z);
  k = z->code_buffer & ((1 << n) - 1);
  z->code_buffer >>= n;
  z->num_bits -= n;
  return k;
}
static int zlib__zhuffman_decode_slowpath(zlib__zbuf *a, zlib__zhuffman *z) {
  int b, s, k;
  // not resolved by fast table, so compute it the slow way
  // use jpeg approach, which requires MSbits at top
  k = zlib__bit_reverse(a->code_buffer, 16);
  for (s = STBI__ZFAST_BITS + 1;; ++s)
    if (k < z->maxcode[s])
      break;
  if (s >= 16)
    return -1; // invalid code!
  // code size is s, so:
  b = (k >> (16 - s)) - z->firstcode[s] + z->firstsymbol[s];
  if (b >= STBI__ZNSYMS)
    return -1; // some data was corrupt somewhere!
  if (z->size[b] != s)
    return -1; // was originally an assert, but report failure instead.
  a->code_buffer >>= s;
  a->num_bits -= s;
  return z->value[b];
}
inline static int zlib__zhuffman_decode(zlib__zbuf *a, zlib__zhuffman *z) {
  int b, s;
  if (a->num_bits < 16) {
    if (zlib__zeof(a)) {
      if (!a->hit_zeof_once) {
        // This is the first time we hit eof, insert 16 extra padding btis
        // to allow us to keep going; if we actually consume any of them
        // though, that is invalid data. This is caught later.
        a->hit_zeof_once = 1;
        a->num_bits += 16; // add 16 implicit zero bits
      } else {
        // We already inserted our extra 16 padding bits and are again
        // out, this stream is actually prematurely terminated.
        return -1;
      }
    } else {
      zlib__fill_bits(a);
    }
  }
  b = z->fast[a->code_buffer & STBI__ZFAST_MASK];
  if (b) {
    s = b >> 9;
    a->code_buffer >>= s;
    a->num_bits -= s;
    return b & 511;
  }
  return zlib__zhuffman_decode_slowpath(a, z);
}
// need to make room for n bytes
static int zlib__zexpand(zlib__zbuf *z, byte *zout, int n) {
  byte *q;
  uint cur, limit, old_limit;
  z->zout = zout;
  if (!z->z_expandable)
    return zlib__err("output buffer limit");
  cur = (uint)(z->zout - z->zout_start);
  limit = old_limit = (unsigned)(z->zout_end - z->zout_start);
  if (UINT_MAX - cur < (unsigned)n)
    return zlib__err("outofmem : Out of memory");
  while (cur + n > limit) {
    if (limit > UINT_MAX / 2)
      return zlib__err("outofmem : Out of memory");
    limit *= 2;
  }
  q = (byte *)realloc(z->zout_start, limit);
  UNUSED(old_limit);
  if (q == NULL)
    return zlib__err("outofmem : Out of memory");
  z->zout_start = q;
  z->zout = q + cur;
  z->zout_end = q + limit;
  return 1;
}
static const int zlib__zlength_base[31] = {
  3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
  15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
  67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
static const int zlib__zlength_extra[31] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0};
static const int zlib__zdist_base[32] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
                                         257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0, 0};
static const int zlib__zdist_extra[32] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
static bool zlib__parse_huffman_block(zlib__zbuf *a) {
  byte *zout = a->zout;
  for (;;) {
    int z = zlib__zhuffman_decode(a, &a->z_length);
    if (z < 0) return zlib__err("bad huffman code"); // error in huffman codes
    else if (z < 256) {
      if (zout >= a->zout_end) {
        if (!zlib__zexpand(a, zout, 1))
          return false;
        zout = a->zout;
      }
      *zout++ = CAST(byte)z;
    } else {
      ubyte *p;
      int len, dist;
      if (z == 256) {
        a->zout = zout;
        if (a->hit_zeof_once && a->num_bits < 16) {
          // The first time we hit zeof, we inserted 16 extra zero bits into our bit
          // buffer so the decoder can just do its speculative decoding. But if we
          // actually consumed any of those bits (which is the case when num_bits < 16),
          // the stream actually read past the end so it is malformed.
          return zlib__err("unexpected end");
        }
        return true;
      }
      if (z >= 286)
        return zlib__err("bad huffman code"); // per DEFLATE, length codes 286 and 287 must not appear in compressed data
      z -= 257;
      len = zlib__zlength_base[z];
      if (zlib__zlength_extra[z])
        len += zlib__zreceive(a, zlib__zlength_extra[z]);
      z = zlib__zhuffman_decode(a, &a->z_distance);
      if (z < 0 || z >= 30)
        return zlib__err("bad huffman code"); // per DEFLATE, distance codes 30 and 31 must not appear in compressed data
      dist = zlib__zdist_base[z];
      if (zlib__zdist_extra[z])
        dist += zlib__zreceive(a, zlib__zdist_extra[z]);
      if (zout - a->zout_start < dist)
        return zlib__err("bad dist");
      if (len > a->zout_end - zout) {
        if (!zlib__zexpand(a, zout, len))
          return false;
        zout = a->zout;
      }
      p = (ubyte *)(zout - dist);
      if (dist == 1) { // run of one byte; common in images.
        ubyte v = *p;
        if (len) {
          do
            *zout++ = v;
          while (--len);
        }
      } else {
        if (len) {
          do
            *zout++ = *p++;
          while (--len);
        }
      }
    }
  }
}
static bool zlib__compute_huffman_codes(zlib__zbuf *a) {
  static const ubyte length_dezigzag[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
  zlib__zhuffman z_codelength;
  ubyte lencodes[286 + 32 + 137]; // padding for maximum single op
  ubyte codelength_sizes[19];
  int i, n;

  int hlit = zlib__zreceive(a, 5) + 257;
  int hdist = zlib__zreceive(a, 5) + 1;
  int hclen = zlib__zreceive(a, 4) + 4;
  int ntot = hlit + hdist;

  memset(codelength_sizes, 0, sizeof(codelength_sizes));
  for (i = 0; i < hclen; ++i) {
    int s = zlib__zreceive(a, 3);
    codelength_sizes[length_dezigzag[i]] = (ubyte)s;
  }
  if (!zlib__zbuild_huffman(&z_codelength, codelength_sizes, 19))
    return false;

  n = 0;
  while (n < ntot) {
    int c = zlib__zhuffman_decode(a, &z_codelength);
    if (c < 0 || c >= 19)
      return zlib__err("bad codelengths");
    if (c < 16)
      lencodes[n++] = (ubyte)c;
    else {
      ubyte fill = 0;
      if (c == 16) {
        c = zlib__zreceive(a, 2) + 3;
        if (n == 0)
          return zlib__err("bad codelengths");
        fill = lencodes[n - 1];
      } else if (c == 17) {
        c = zlib__zreceive(a, 3) + 3;
      } else if (c == 18) {
        c = zlib__zreceive(a, 7) + 11;
      } else {
        return zlib__err("bad codelengths");
      }
      if (ntot - n < c)
        return zlib__err("bad codelengths");
      memset(lencodes + n, fill, c);
      n += c;
    }
  }
  if (n != ntot)
    return zlib__err("bad codelengths");
  if (!zlib__zbuild_huffman(&a->z_length, lencodes, hlit) ||
      !zlib__zbuild_huffman(&a->z_distance, lencodes + hlit, hdist))
    return false;
  return true;
}
static int zlib__parse_uncompressed_block(zlib__zbuf *a) {
  ubyte header[4];
  int len, nlen, k;
  if (a->num_bits & 7)
    zlib__zreceive(a, a->num_bits & 7); // discard
  // drain the bit-packed data into header
  k = 0;
  while (a->num_bits > 0) {
    header[k++] = (ubyte)(a->code_buffer & 255); // suppress MSVC run-time check
    a->code_buffer >>= 8;
    a->num_bits -= 8;
  }
  if (a->num_bits < 0)
    return zlib__err("zlib corrupt");
  // now fill header the normal way
  while (k < 4)
    header[k++] = zlib__zget8(a);
  len = header[1] * 256 + header[0];
  nlen = header[3] * 256 + header[2];
  if (nlen != (len ^ 0xffff))
    return zlib__err("zlib corrupt");
  if (a->zbuffer + len > a->zbuffer_end)
    return zlib__err("read past buffer");
  if (a->zout + len > a->zout_end)
    if (!zlib__zexpand(a, a->zout, len))
      return 0;
  memcpy(a->zout, a->zbuffer, len);
  a->zbuffer += len;
  a->zout += len;
  return 1;
}
static bool zlib__parse_zlib_header(zlib__zbuf *a) {
  int cmf = zlib__zget8(a);
  int cm = cmf & 15;
  /* int cinfo = cmf >> 4; */
  int flg = zlib__zget8(a);
  if (zlib__zeof(a))
    return zlib__err("bad zlib header"); // zlib spec
  if ((cmf * 256 + flg) % 31 != 0)
    return zlib__err("bad zlib header"); // zlib spec
  if (flg & 32)
    return zlib__err("no preset dict"); // preset dictionary not allowed in png
  if (cm != 8)
    return zlib__err("bad compression"); // DEFLATE required for png
  // window = 1 << (8 + cinfo)... but who cares, we fully buffer output
  return true;
}
static const ubyte zlib__zdefault_length[STBI__ZNSYMS] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8};
static const ubyte zlib__zdefault_distance[32] = {
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
/*
Init algorithm: {
   int i;   // use <= to match clearly with spec
   for (i=0; i <= 143; ++i)     zlib__zdefault_length[i]   = 8;
   for (   ; i <= 255; ++i)     zlib__zdefault_length[i]   = 9;
   for (   ; i <= 279; ++i)     zlib__zdefault_length[i]   = 7;
   for (   ; i <= 287; ++i)     zlib__zdefault_length[i]   = 8;

   for (i=0; i <=  31; ++i)     zlib__zdefault_distance[i] = 5;
}
*/

static bool zlib__parse_zlib(zlib__zbuf *a, int parse_header) {
  int final, type;
  if (parse_header)
    if (!zlib__parse_zlib_header(a))
      return false;
  a->num_bits = 0;
  a->code_buffer = 0;
  a->hit_zeof_once = 0;
  do {
    final = zlib__zreceive(a, 1);
    type = zlib__zreceive(a, 2);
    if (type == 0) {
      if (!zlib__parse_uncompressed_block(a))
        return false;
    } else if (type == 3) {
      return false;
    } else {
      if (type == 1) {
        // use fixed code lengths
        if (!zlib__zbuild_huffman(&a->z_length, zlib__zdefault_length, STBI__ZNSYMS) ||
            !zlib__zbuild_huffman(&a->z_distance, zlib__zdefault_distance, 32))
          return false;
      } else if (!zlib__compute_huffman_codes(a)) {
        return false;
      }
      if (!zlib__parse_huffman_block(a))
        return false;
    }
  } while (!final);
  return true;
}
static bool zlib__do_zlib(zlib__zbuf *a, byte *obuf, int olen, int exp, int parse_header) {
  a->zout_start = obuf;
  a->zout = obuf;
  a->zout_end = obuf + olen;
  a->z_expandable = exp;

  return zlib__parse_zlib(a, parse_header);
}
byte *zlib_decode_malloc_guesssize(const byte *buffer, int len, int initial_size, int *outlen) {
  zlib__zbuf a;
  byte *p = (byte *)malloc(initial_size);
  if (p == NULL)
    return NULL;
  a.zbuffer = (ubyte *)buffer;
  a.zbuffer_end = (ubyte *)buffer + len;
  if (zlib__do_zlib(&a, p, initial_size, 1, 1)) {
    if (outlen)
      *outlen = (int)(a.zout - a.zout_start);
    return a.zout_start;
  } else {
    free(a.zout_start);
    return NULL;
  }
}
byte *zlib_decode_malloc(byte const *buffer, int len, int *outlen) {
  return zlib_decode_malloc_guesssize(buffer, len, 16384, outlen);
}
byte *zlib_decode_malloc_guesssize_headerflag(const byte *buffer, int len, int initial_size, int *outlen, int parse_header) {
  zlib__zbuf a;
  byte *p = (byte *)malloc(initial_size);
  if (p == NULL)
    return NULL;
  a.zbuffer = (ubyte *)buffer;
  a.zbuffer_end = (ubyte *)buffer + len;
  if (zlib__do_zlib(&a, p, initial_size, 1, parse_header)) {
    if (outlen)
      *outlen = (int)(a.zout - a.zout_start);
    return a.zout_start;
  } else {
    free(a.zout_start);
    return NULL;
  }
}
int zlib_decode_buffer(byte *obuffer, int olen, byte const *ibuffer, int ilen) {
  zlib__zbuf a;
  a.zbuffer = (ubyte *)ibuffer;
  a.zbuffer_end = (ubyte *)ibuffer + ilen;
  if (zlib__do_zlib(&a, obuffer, olen, 0, 1))
    return (int)(a.zout - a.zout_start);
  else
    return -1;
}
byte *zlib_decode_noheader_malloc(byte const *buffer, int len, int *outlen) {
  zlib__zbuf a;
  byte *p = (byte *)malloc(16384);
  if (p == NULL)
    return NULL;
  a.zbuffer = (ubyte *)buffer;
  a.zbuffer_end = (ubyte *)buffer + len;
  if (zlib__do_zlib(&a, p, 16384, 1, 0)) {
    if (outlen)
      *outlen = (int)(a.zout - a.zout_start);
    return a.zout_start;
  } else {
    free(a.zout_start);
    return NULL;
  }
}
int zlib_decode_noheader_buffer(byte *obuffer, int olen, const byte *ibuffer, int ilen) {
  zlib__zbuf a;
  a.zbuffer = (ubyte *)ibuffer;
  a.zbuffer_end = (ubyte *)ibuffer + ilen;
  if (zlib__do_zlib(&a, obuffer, olen, 0, 0))
    return (int)(a.zout - a.zout_start);
  else
    return -1;
}


// stretchy buffer; zlib__sbpush() == vector<>::push_back() -- zlib__sbcount() == vector<>::size()
#define zlib__sbraw(a) ((int *)(void *)(a) - 2)
#define zlib__sbm(a)   zlib__sbraw(a)[0]
#define zlib__sbn(a)   zlib__sbraw(a)[1]

#define zlib__sbneedgrow(a, n)  ((a) == 0 || zlib__sbn(a) + n >= zlib__sbm(a))
#define zlib__sbmaybegrow(a, n) (zlib__sbneedgrow(a, (n)) ? zlib__sbgrow(a, n) : 0)
#define zlib__sbgrow(a, n)      zlib__sbgrowf((void **)&(a), (n), sizeof(*(a)))

#define zlib__sbpush(a, v) (zlib__sbmaybegrow(a, 1), (a)[zlib__sbn(a)++] = (v))
#define zlib__sbcount(a)   ((a) ? zlib__sbn(a) : 0)
#define zlib__sbfree(a)    ((a) ? free(zlib__sbraw(a)), 0 : 0)

static void *zlib__sbgrowf(void **arr, int increment, int itemsize) {
  int m = *arr ? 2 * zlib__sbm(*arr) + increment : increment + 1;
  void *p = realloc(*arr ? zlib__sbraw(*arr) : 0, itemsize * m + sizeof(int) * 2);
  ASSERT(p);
  if (p) {
    if (!*arr)
      ((int *)p)[1] = 0;
    *arr = (void *)((int *)p + 2);
    zlib__sbm(*arr) = m;
  }
  return *arr;
}
static ubyte *zlib__zlib_flushf(ubyte *data, uint *bitbuffer, int *bitcount) {
  while (*bitcount >= 8) {
    zlib__sbpush(data, (ubyte)(*bitbuffer));
    *bitbuffer >>= 8;
    *bitcount -= 8;
  }
  return data;
}
static int zlib__zlib_bitrev(int code, int codebits) {
  int res = 0;
  while (codebits--) {
    res = (res << 1) | (code & 1);
    code >>= 1;
  }
  return res;
}
static uint zlib__zlib_countm(ubyte *a, ubyte *b, int limit) {
  int i;
  for (i = 0; i < limit && i < 258; ++i)
    if (a[i] != b[i])
      break;
  return i;
}
static uint zlib__zhash(ubyte *data) {
  uint32 hash = data[0] + (data[1] << 8) + (data[2] << 16);
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;
  return hash;
}
#define zlib__zlib_flush() (out = zlib__zlib_flushf(out, &bitbuf, &bitcount))
#define zlib__zlib_add(code, codebits) (bitbuf |= (code) << bitcount, bitcount += (codebits), zlib__zlib_flush())
#define zlib__zlib_huffa(b, c) zlib__zlib_add(zlib__zlib_bitrev(b, c), c)
// default huffman tables
#define zlib__zlib_huff1(n) zlib__zlib_huffa(0x30 + (n), 8)
#define zlib__zlib_huff2(n) zlib__zlib_huffa(0x190 + (n) - 144, 9)
#define zlib__zlib_huff3(n) zlib__zlib_huffa(0 + (n) - 256, 7)
#define zlib__zlib_huff4(n) zlib__zlib_huffa(0xc0 + (n) - 280, 8)
#define zlib__zlib_huff(n) do { \
  if ((n) <= 143) zlib__zlib_huff1(n);\
  else if ((n) <= 255) zlib__zlib_huff2(n); \
  else if ((n) <= 279) zlib__zlib_huff3(n); \
  else zlib__zlib_huff4(n);\
} while (0)
#define zlib__zlib_huffb(n) ((n) <= 143 ? zlib__zlib_huff1(n) : zlib__zlib_huff2(n))
#define zlib__ZHASH 16384

ubyte *zlib_encode(ubyte *data, int data_len, int *out_len, int quality) {
  static ushrt lengthc[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 259};
  static ubyte lengtheb[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
  static ushrt distc[] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 32768};
  static ubyte disteb[] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
  uint bitbuf = 0;
  int i, j, bitcount = 0;
  ubyte *out = NULL;
  ubyte ***hash_table = (ubyte ***)malloc(zlib__ZHASH * sizeof(ubyte **));
  if (hash_table == NULL)
    return NULL;
  if (quality < 5)
    quality = 5;

  zlib__sbpush(out, 0x78); // DEFLATE 32K window
  zlib__sbpush(out, 0x5e); // FLEVEL = 1
  zlib__zlib_add(1, 1);    // BFINAL = 1
  zlib__zlib_add(1, 2);    // BTYPE = 1 -- fixed huffman

  for (i = 0; i < zlib__ZHASH; ++i)
    hash_table[i] = NULL;

  i = 0;
  while (i < data_len - 3) {
    // hash next 3 bytes of data to be compressed
    int h = zlib__zhash(data + i) & (zlib__ZHASH - 1), best = 3;
    ubyte *bestloc = 0;
    ubyte **hlist = hash_table[h];
    int n = zlib__sbcount(hlist);
    for (j = 0; j < n; ++j) {
      if (hlist[j] - data > i - 32768) { // if entry lies within window
        int d = zlib__zlib_countm(hlist[j], data + i, data_len - i);
        if (d >= best) {
          best = d;
          bestloc = hlist[j];
        }
      }
    }
    // when hash table entry is too long, delete half the entries
    if (hash_table[h] && zlib__sbn(hash_table[h]) == 2 * quality) {
      memmove(hash_table[h], hash_table[h] + quality, sizeof(hash_table[h][0]) * quality);
      zlib__sbn(hash_table[h]) = quality;
    }
    zlib__sbpush(hash_table[h], data + i);

    if (bestloc) {
      // "lazy matching" - check match at *next* byte, and if it's better, do cur byte as literal
      h = zlib__zhash(data + i + 1) & (zlib__ZHASH - 1);
      hlist = hash_table[h];
      n = zlib__sbcount(hlist);
      for (j = 0; j < n; ++j) {
        if (hlist[j] - data > i - 32767) {
          int e = zlib__zlib_countm(hlist[j], data + i + 1, data_len - i - 1);
          if (e > best) { // if next match is better, bail on current match
            bestloc = NULL;
            break;
          }
        }
      }
    }

    if (bestloc) {
      int d = (int)(data + i - bestloc); // distance back
      ASSERT(d <= 32767 && best <= 258);
      for (j = 0; best > lengthc[j + 1] - 1; ++j)
        ;
      zlib__zlib_huff(j + 257);
      if (lengtheb[j])
        zlib__zlib_add(best - lengthc[j], lengtheb[j]);
      for (j = 0; d > distc[j + 1] - 1; ++j)
        ;
      zlib__zlib_add(zlib__zlib_bitrev(j, 5), 5);
      if (disteb[j])
        zlib__zlib_add(d - distc[j], disteb[j]);
      i += best;
    } else {
      zlib__zlib_huffb(data[i]);
      ++i;
    }
  }
  // write out final bytes
  for (; i < data_len; ++i)
    zlib__zlib_huffb(data[i]);
  zlib__zlib_huff(256); // end of block
  // pad with 0 bits to byte boundary
  while (bitcount)
    zlib__zlib_add(0, 1);

  for (i = 0; i < zlib__ZHASH; ++i)
    (void)zlib__sbfree(hash_table[i]);
  free(hash_table);

  // store uncompressed instead if compression was worse
  if (zlib__sbn(out) > data_len + 2 + ((data_len + 32766) / 32767) * 5) {
    zlib__sbn(out) = 2; // truncate to DEFLATE 32K window and FLEVEL = 1
    for (j = 0; j < data_len;) {
      int blocklen = data_len - j;
      if (blocklen > 32767)
        blocklen = 32767;
      zlib__sbpush(out, data_len - j == blocklen); // BFINAL = ?, BTYPE = 0 -- no compression
      zlib__sbpush(out, (ubyte)(blocklen));    // LEN
      zlib__sbpush(out, (ubyte)(blocklen >> 8));
      zlib__sbpush(out, (ubyte)(~blocklen)); // NLEN
      zlib__sbpush(out, (ubyte)(~blocklen >> 8));
      memcpy(out + zlib__sbn(out), data + j, blocklen);
      zlib__sbn(out) += blocklen;
      j += blocklen;
    }
  }

  {
    // compute adler32 on input
    uint s1 = 1, s2 = 0;
    int blocklen = (int)(data_len % 5552);
    j = 0;
    while (j < data_len) {
      for (i = 0; i < blocklen; ++i) {
        s1 += data[j + i];
        s2 += s1;
      }
      s1 %= 65521;
      s2 %= 65521;
      j += blocklen;
      blocklen = 5552;
    }
    zlib__sbpush(out, (ubyte)(s2 >> 8));
    zlib__sbpush(out, (ubyte)(s2));
    zlib__sbpush(out, (ubyte)(s1 >> 8));
    zlib__sbpush(out, (ubyte)(s1));
  }
  *out_len = zlib__sbn(out);
  // make returned pointer freeable
  memmove(zlib__sbraw(out), out, *out_len);
  return CAST(ubyte*)zlib__sbraw(out);
}
