#include "stb/local.h"
#include "stb/image_write.h"
#include "stb/zlib.h"
#include "algorithm/hash.h"

// flip the image result vertically, so the first pixel is the bottom left
// #define VERTICALLY_FLIP
#define RLE_IN_TGA

typedef struct {
  stbi_write_func *func;
  void *context;
  ubyte buffer[128];
  iter buf_used;
} stbi__write_context;

static void stbiw__writefv(stbi__write_context *s, const char *fmt, va_list v) {
  while (*fmt) {
    switch (*fmt++) {
    case ' ':
      break;
    case '1': {
      ubyte x = (ubyte)(va_arg(v, int));
      s->func(s->context, &x, 1);
      break;
    }
    case '2': {
      int x = va_arg(v, int);
      ubyte b[2];
      b[0] = (ubyte)(x);
      b[1] = (ubyte)(x >> 8);
      s->func(s->context, b, 2);
      break;
    }
    case '4': {
      uint32 x = va_arg(v, int);
      ubyte b[4];
      b[0] = (ubyte)(x);
      b[1] = (ubyte)(x >> 8);
      b[2] = (ubyte)(x >> 16);
      b[3] = (ubyte)(x >> 24);
      s->func(s->context, b, 4);
      break;
    }
    default:
      ASSERT(0);
      return;
    }
  }
}
#ifdef RLE_IN_TGA
static void stbiw__writef(stbi__write_context *s, const char *fmt, ...) {
  va_list v;
  va_start(v, fmt);
  stbiw__writefv(s, fmt, v);
  va_end(v);
}
#endif // RLE_IN_TGA

static void stbiw__write_flush(stbi__write_context *s) {
  if (s->buf_used) {
    s->func(s->context, &s->buffer, s->buf_used);
    s->buf_used = 0;
  }
}
static void stbiw__putc(stbi__write_context *s, ubyte c) {
  s->func(s->context, &c, 1);
}
static void stbiw__write1(stbi__write_context *s, ubyte a) {
  if ((iter)s->buf_used + 1 > sizeof(s->buffer))
    stbiw__write_flush(s);
  s->buffer[s->buf_used++] = a;
}
static void stbiw__write3(stbi__write_context *s, ubyte a, ubyte b, ubyte c) {
  int n;
  if ((iter)s->buf_used + 3 > sizeof(s->buffer))
    stbiw__write_flush(s);
  n = s->buf_used;
  s->buf_used = n + 3;
  s->buffer[n + 0] = a;
  s->buffer[n + 1] = b;
  s->buffer[n + 2] = c;
}
// suspose to write that smaller than buffer
static void stbiw__write_smalln(stbi__write_context *s, const void *x, ubyte n) {
  if ((iter)s->buf_used + n > sizeof(s->buffer))
    stbiw__write_flush(s);
  memcpy(s->buffer + s->buf_used, x, n);
  s->buf_used += n;
}

static void stbiw__write_pixel(stbi__write_context *s, int rgb_dir, int comp, int write_alpha, int expand_mono, ubyte *d) {
  ubyte bg[3] = {255, 0, 255}, px[3];
  int k;

  if (write_alpha < 0)
    stbiw__write1(s, d[comp - 1]);

  switch (comp) {
  case 2: // 2 pixels = mono + alpha, alpha is written separately, so same as 1-channel case
  case 1:
    if (expand_mono)
      stbiw__write3(s, d[0], d[0], d[0]); // monochrome bmp
    else
      stbiw__write1(s, d[0]); // monochrome TGA
    break;
  case 4:
    if (!write_alpha) {
      // composite against pink background
      for (k = 0; k < 3; ++k)
        px[k] = bg[k] + ((d[k] - bg[k]) * d[3]) / 255;
      stbiw__write3(s, px[1 - rgb_dir], px[1], px[1 + rgb_dir]);
      break;
    }
    /* FALLTHROUGH */
  case 3:
    stbiw__write3(s, d[1 - rgb_dir], d[1], d[1 + rgb_dir]);
    break;
  }
  if (write_alpha > 0)
    stbiw__write1(s, d[comp - 1]);
}

static void stbiw__write_pixels(stbi__write_context *s, int rgb_dir, int vdir, int x, int y, int comp, void *data, int write_alpha, int scanline_pad, int expand_mono) {
  uint32 zero = 0;
  int i, j, j_end;

  if (y <= 0)
    return;

#ifdef VERTICALLY_FLIP
  vdir *= -1;
#endif // VERTICALLY_FLIP

  if (vdir < 0) {
    j_end = -1;
    j = y - 1;
  } else {
    j_end = y;
    j = 0;
  }

  for (; j != j_end; j += vdir) {
    for (i = 0; i < x; ++i) {
      ubyte *d = (ubyte *)data + (j * x + i) * comp;
      stbiw__write_pixel(s, rgb_dir, comp, write_alpha, expand_mono, d);
    }
    stbiw__write_flush(s);
    s->func(s->context, &zero, scanline_pad);
  }
}

static bool stbiw__outfile(stbi__write_context *s, int rgb_dir, int vdir, int x, int y, int comp, int expand_mono, void *data, int alpha, int pad, const char *fmt, ...) {
  if (y < 0 || x < 0) {
    return false;
  } else {
    va_list v;
    va_start(v, fmt);
    stbiw__writefv(s, fmt, v);
    va_end(v);
    stbiw__write_pixels(s, rgb_dir, vdir, x, y, comp, data, alpha, pad, expand_mono);
    return true;
  }
}

static bool stbi_write_bmp_core(stbi__write_context *s, int x, int y, int comp, const void *data) {
  if (comp != 4) {
    // write RGB bitmap
    int pad = (-x * 3) & 3;
    return stbiw__outfile(s, -1, -1, x, y, comp, 1, (void *)data, 0, pad,
                          "11 4 22 4"
                          "4 44 22 444444",
                          'B', 'M', 14 + 40 + (x * 3 + pad) * y, 0, 0, 14 + 40, // file header
                          40, x, y, 1, 24, 0, 0, 0, 0, 0, 0);                   // bitmap header
  } else {
    // RGBA bitmaps need a v4 header
    // use BI_BITFIELDS mode with 32bpp and alpha mask
    // (straight BI_RGB with alpha mask doesn't work in most readers)
    return stbiw__outfile(s, -1, -1, x, y, comp, 1, (void *)data, 1, 0,
                          "11 4 22 4"
                          "4 44 22 444444 4444 4 444 444 444 444",
                          'B', 'M', 14 + 108 + x * y * 4, 0, 0, 14 + 108,                                                                  // file header
                          108, x, y, 1, 32, 3, 0, 0, 0, 0, 0, 0xff0000, 0xff00, 0xff, 0xff000000u, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0); // bitmap V4 header
  }
}
static bool stbi_write_tga_core(stbi__write_context *s, int x, int y, int comp, void *data) {
  int has_alpha = (comp == 2 || comp == 4);
  int colorbytes = has_alpha ? comp - 1 : comp;
  int format = colorbytes < 2 ? 3 : 2; // 3 color channels (RGB/RGBA) = 2, 1 color channel (Y/YA) = 3

  if (y < 0 || x < 0)
    return false;

#ifndef RLE_IN_TGA
  return stbiw__outfile(s, -1, -1, x, y, comp, 0, (void *)data, has_alpha, 0, "111 221 2222 11", 0, 0, format, 0, 0, 0, 0, 0, x, y, (colorbytes + has_alpha) * 8, has_alpha * 8);
#else
  int i, j, k;
  int jend, jdir;

  stbiw__writef(s, "111 221 2222 11", 0, 0, format + 8, 0, 0, 0, 0, 0, x, y, (colorbytes + has_alpha) * 8, has_alpha * 8);

  #ifdef VERTICALLY_FLIP
  j = 0;
  jend = y;
  jdir = 1;
  #else
  j = y - 1;
  jend = -1;
  jdir = -1;
  #endif // VERTICALLY_FLIP
  for (; j != jend; j += jdir) {
    ubyte *row = (ubyte *)data + j * x * comp;
    int len;

    for (i = 0; i < x; i += len) {
      ubyte *begin = row + i * comp;
      int diff = 1;
      len = 1;

      if (i < x - 1) {
        ++len;
        diff = memcmp(begin, row + (i + 1) * comp, comp);
        if (diff) {
          const ubyte *prev = begin;
          for (k = i + 2; k < x && len < 128; ++k) {
            if (memcmp(prev, row + k * comp, comp)) {
              prev += comp;
              ++len;
            } else {
              --len;
              break;
            }
          }
        } else {
          for (k = i + 2; k < x && len < 128; ++k) {
            if (!memcmp(begin, row + k * comp, comp)) {
              ++len;
            } else {
              break;
            }
          }
        }
      }

      if (diff) {
        ubyte header = (ubyte)(len - 1);
        stbiw__write1(s, header);
        for (k = 0; k < len; ++k) {
          stbiw__write_pixel(s, -1, comp, has_alpha, 0, begin + k * comp);
        }
      } else {
        ubyte header = (ubyte)(len - 129);
        stbiw__write1(s, header);
        stbiw__write_pixel(s, -1, comp, has_alpha, 0, begin);
      }
    }
  }
  stbiw__write_flush(s);
#endif // RLE_IN_TGA
  return true;
}

// *************************************************************************************************
// Radiance RGBE HDR writer
// by Baldur Karlsson

static void stbiw__linear_to_rgbe(ubyte *rgbe, float *linear) {
  int exponent;
  float maxcomp = MAX(linear[0], MAX(linear[1], linear[2]));

  if (maxcomp < 1e-32f) {
    rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
  } else {
    float normalize = (float)imath_frexp(maxcomp, &exponent) * 256.0f / maxcomp;

    rgbe[0] = (ubyte)(linear[0] * normalize);
    rgbe[1] = (ubyte)(linear[1] * normalize);
    rgbe[2] = (ubyte)(linear[2] * normalize);
    rgbe[3] = (ubyte)(exponent + 128);
  }
}
static void stbiw__write_run_data(stbi__write_context *s, int length, ubyte databyte) {
  ubyte lengthbyte = (ubyte)(length + 128);
  ASSERT(length + 128 <= 255);
  s->func(s->context, &lengthbyte, 1);
  s->func(s->context, &databyte, 1);
}
static void stbiw__write_dump_data(stbi__write_context *s, int length, ubyte *data) {
  ubyte lengthbyte = (ubyte)(length);
  ASSERT(length <= 128); // inconsistent with spec but consistent with official code
  s->func(s->context, &lengthbyte, 1);
  s->func(s->context, data, length);
}
static void stbiw__write_hdr_scanline(stbi__write_context *s, int width, int ncomp, ubyte *scratch, float *scanline) {
  ubyte scanlineheader[4] = {2, 2, 0, 0};
  ubyte rgbe[4];
  float linear[3];
  int x;

  scanlineheader[2] = (width & 0xff00) >> 8;
  scanlineheader[3] = (width & 0x00ff);

  /* skip RLE for images too small or large */
  if (width < 8 || width >= 32768) {
    for (x = 0; x < width; x++) {
      switch (ncomp) {
      case 4: /* fallthrough */
      case 3:
        linear[2] = scanline[x * ncomp + 2];
        linear[1] = scanline[x * ncomp + 1];
        linear[0] = scanline[x * ncomp + 0];
        break;
      default:
        linear[0] = linear[1] = linear[2] = scanline[x * ncomp + 0];
        break;
      }
      stbiw__linear_to_rgbe(rgbe, linear);
      s->func(s->context, rgbe, 4);
    }
  } else {
    int c, r;
    /* encode into scratch buffer */
    for (x = 0; x < width; x++) {
      switch (ncomp) {
      case 4: /* fallthrough */
      case 3:
        linear[2] = scanline[x * ncomp + 2];
        linear[1] = scanline[x * ncomp + 1];
        linear[0] = scanline[x * ncomp + 0];
        break;
      default:
        linear[0] = linear[1] = linear[2] = scanline[x * ncomp + 0];
        break;
      }
      stbiw__linear_to_rgbe(rgbe, linear);
      scratch[x + width * 0] = rgbe[0];
      scratch[x + width * 1] = rgbe[1];
      scratch[x + width * 2] = rgbe[2];
      scratch[x + width * 3] = rgbe[3];
    }

    s->func(s->context, scanlineheader, 4);

    /* RLE each component separately */
    for (c = 0; c < 4; c++) {
      ubyte *comp = &scratch[width * c];

      x = 0;
      while (x < width) {
        // find first run
        r = x;
        while (r + 2 < width) {
          if (comp[r] == comp[r + 1] && comp[r] == comp[r + 2])
            break;
          ++r;
        }
        if (r + 2 >= width)
          r = width;
        // dump up to first run
        while (x < r) {
          int len = r - x;
          if (len > 128)
            len = 128;
          stbiw__write_dump_data(s, len, &comp[x]);
          x += len;
        }
        // if there's a run, output it
        if (r + 2 < width) { // same test as what we break out of in search loop, so only true if we break'd
          // find next byte after run
          while (r < width && comp[r] == comp[x])
            ++r;
          // output run up to r
          while (x < r) {
            int len = r - x;
            if (len > 127)
              len = 127;
            stbiw__write_run_data(s, len, comp[x]);
            x += len;
          }
        }
      }
    }
  }
}
static bool stbi_write_hdr_core(stbi__write_context *s, int x, int y, int comp, float *data) {
  if (y <= 0 || x <= 0 || data == NULL)
    return false;
  else {
    // Each component is stored separately. Allocate scratch space for full output scanline.
    ubyte *scratch = (ubyte *)malloc(x * 4);
    int i, len;
    char buffer[128];
    char header[] = "#?RADIANCE\n# Written by stb_image_write.h\nFORMAT=32-bit_rle_rgbe\n";
    s->func(s->context, header, sizeof(header) - 1);

#ifdef __STDC_LIB_EXT1__
    len = sprintf_s(buffer, sizeof(buffer), "EXPOSURE=          1.0000000000000\n\n-Y %d +X %d\n", y, x);
#else
    len = sprintf(buffer, "EXPOSURE=          1.0000000000000\n\n-Y %d +X %d\n", y, x);
#endif
    s->func(s->context, buffer, len);

    for (i = 0; i < y; i++) {
#ifdef VERTICALLY_FLIP
      int Y = y - 1 - i;
#else
      int Y = i;
#endif // VERTICALLY_FLIP
      stbiw__write_hdr_scanline(s, x, comp, scratch, data + comp * x * Y);
    }
    free(scratch);
    return true;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// PNG writer
//

static ubyte stbiw__paeth(int a, int b, int c) {
  int p = a + b - c, pa = imath_iabs(p - a), pb = imath_iabs(p - b), pc = imath_iabs(p - c);
  if (pa <= pb && pa <= pc)
    return (ubyte)(a);
  if (pb <= pc)
    return (ubyte)(b);
  return (ubyte)(c);
}
static inline void stbiw__encode_png_line(const ubyte *pixels, int w, int h, int y, int n, int filter_type, ubyte *filt) {
  *filt = CAST(ubyte)filter_type & 0xff;
  byte *line_buffer = CAST(byte*)(filt + 1);
  static const int mapping[] = {/* firstmap */ 0, 1, 0, 5, 6, /* filter map */ 0, 1, 2, 3, 4};
#ifdef VERTICALLY_FLIP
  const ubyte *z = pixels + w * n * (h - 1 - y);
  int signed_stride = -w * n;
#else
  const ubyte *z = pixels + w * n * y;
  int signed_stride = w * n;
#endif // VERTICALLY_FLIP

  // first loop isn't optimized since it's just one pixel
  int i;
  switch (mapping[filter_type + (y != 0) * 5]) {
  case 0:
    memcpy(line_buffer, z, w * n);
    break;
  case 1:
    for (i = 0; i < n; ++i)
      line_buffer[i] = z[i];
    for (; i < w * n; ++i)
      line_buffer[i] = z[i] - z[i - n];
    break;
  case 2:
    for (i = 0; i < w * n; ++i)
      line_buffer[i] = z[i] - z[i - signed_stride];
    break;
  case 3:
    for (i = 0; i < n; ++i)
      line_buffer[i] = z[i] - (z[i - signed_stride] >> 1);
    for (; i < w * n; ++i)
      line_buffer[i] = z[i] - ((z[i - n] + z[i - signed_stride]) >> 1);
    break;
  case 4:
    for (i = 0; i < n; ++i)
      line_buffer[i] = (byte)(z[i] - stbiw__paeth(0, z[i - signed_stride], 0));
    for (; i < w * n; ++i)
      line_buffer[i] = z[i] - stbiw__paeth(z[i - n], z[i - signed_stride], z[i - signed_stride - n]);
    break;
  case 5:
    for (i = 0; i < n; ++i)
      line_buffer[i] = z[i];
    for (; i < w * n; ++i)
      line_buffer[i] = z[i] - (z[i - n] >> 1);
    break;
  case 6:
    for (i = 0; i < n; ++i)
      line_buffer[i] = z[i];
    for (; i < w * n; ++i)
      line_buffer[i] = z[i] - stbiw__paeth(z[i - n], 0, 0);
    break;
  }
}
static inline int stbiw__encode_png_line_test(const ubyte *pixels, int w, int h, int y, int n, int filter_type, int *est) {
  static const int mapping[] = {/* firstmap */ 0, 1, 0, 5, 6, /* filter map */ 0, 1, 2, 3, 4};
#ifdef VERTICALLY_FLIP
  const ubyte *z = pixels + w * n * (h - 1 - y);
  int signed_stride = -w * n;
#else
  const ubyte *z = pixels + w * n * y;
  int signed_stride = w * n;
#endif // VERTICALLY_FLIP
  
  int estimate = 0;
  // first loop isn't optimized since it's just one pixel
  int i;
  switch (mapping[filter_type + (y != 0) * 5]) {
  case 0:
    for (i = 0; (estimate < *est) && (i < w * n); ++i)
      estimate += imath_iabs((byte)z[i]);
    break;
  case 1:
    for (i = 0; (estimate < *est) && (i < n); ++i)
      estimate += imath_iabs((byte)z[i]);
    for (; (estimate < *est) && (i < w * n); ++i)
      estimate += imath_iabs((byte)(z[i] - z[i - n]));
    break;
  case 2:
    for (i = 0; (estimate < *est) && (i < w * n); ++i)
      estimate += imath_iabs((byte)(z[i] - z[i - signed_stride]));
    break;
  case 3:
    for (i = 0; (estimate < *est) && (i < n); ++i)
      estimate += imath_iabs((byte)(z[i] - (z[i - signed_stride] >> 1)));
    for (; (estimate < *est) && (i < w * n); ++i)
      estimate += imath_iabs((byte)(z[i] - ((z[i - n] + z[i - signed_stride]) >> 1)));
    break;
  case 4:
    for (i = 0; (estimate < *est) && (i < n); ++i)
      estimate += imath_iabs((byte)(z[i] - stbiw__paeth(0, z[i - signed_stride], 0)));
    for (; (estimate < *est) && (i < w * n); ++i)
      estimate += imath_iabs((byte)(z[i] - stbiw__paeth(z[i - n], z[i - signed_stride], z[i - signed_stride - n])));
    break;
  case 5:
    for (i = 0; (estimate < *est) && (i < n); ++i)
      estimate += imath_iabs((byte)z[i]);
    for (; (estimate < *est) && (i < w * n); ++i)
      estimate += imath_iabs((byte)(z[i] - (z[i - n] >> 1)));
    break;
  case 6:
    for (i = 0; (estimate < *est) && (i < n); ++i)
      estimate += imath_iabs((byte)z[i]);
    for (; (estimate < *est) && (i < w * n); ++i)
      estimate += imath_iabs((byte)(z[i] - stbiw__paeth(z[i - n], 0, 0)));
    break;
  }
  bool res = *est > estimate;
  if (res) *est = estimate;
  return res;
}

bool stbi_write_png_core(stbi__write_context *s, int x, int y, int n, const void *data) {
  // png signaturev137 PNG \r\n 26 \n (header len, 13) \0\0\0\r *****
  union {
    ubyte ub[4];
    uint32 ui;
  } temp;
  uint32 crc;

  stbiw__write_smalln(s, PNG_SIGNATURE, 8);

#define WRCB(x) do {       \
  temp.ub[0] = (x);        \
  stbiw__write1(s, temp.ub[0]); \
  hash_crc32_append(&crc, temp.ub[0]); \
} while (0)
#define WRCRC32(x) do {                                                                                  \
  temp.ui = imath_flip32(x); \
  stbiw__write_smalln(s, &temp.ui, 4);  \
  hash_crc32_appends(&crc, &temp.ui, 4); \
} while (0)
#define WR32(x) do {                                                   \
  temp.ui = imath_flip32(x); \
  stbiw__write_smalln(s, &temp.ui, 4); \
} while (0)
#define HEAD(x, l) do { \
  temp.ui = imath_flip32(l); \
  stbiw__write_smalln(s, &temp.ui, 4); \
  hash_crc32_start(&crc); \
  stbiw__write_smalln(s, (x), 4); \
  hash_crc32_appends(&crc, (x), 4); \
} while (0)
#define SIGN do {\
  hash_crc32_end(&crc); \
  crc = imath_flip32(crc); \
  stbiw__write_smalln(s, &crc, 4); \
} while (0)

  HEAD("IHDR", 13);
  WRCRC32(x);
  WRCRC32(y);
  WRCB(8);
  const ubyte ctype[5] = {0xff, 0, 4, 2, 6};
  WRCB(ctype[n]);
  temp.ui = 0;
  stbiw__write_smalln(s, temp.ub, 3);
  hash_crc32_appends(&crc, temp.ub, 3);
  SIGN;
  {
    const ubyte *pixels = (const ubyte *)data;
    iter stride_bytes = x * n, i, j;
    int best_filter, est, zlen;
    ubyte *filt = CAST(ubyte*)malloc((stride_bytes + 1) * y);
    if (!filt) return false;
    for (i = 0; i < y; ++i) {
      // Estimate the best filter by running through all of them:
      best_filter = 0, est = 0x7fffffff;
      for (j = 0; j < 5; ++j) {
        // Estimate the entropy of the line using this filter; the less, the better.
        if (stbiw__encode_png_line_test(pixels, x, y, i, n, j, &est))
          best_filter = j;
      }
      // when we get here, filter_type contains the filter type and data
      stbiw__encode_png_line(pixels, x, y, i, n, best_filter, filt + i * (stride_bytes + 1));
    }
    // add compression level
    ubyte *zlib = zlib_encode(filt, (stride_bytes + 1) * y, &zlen, 0);
    free(filt);
    if (!zlib) return false;
    HEAD("IDAT", zlen);
    stbiw__write_flush(s);
    s->func(s->context, zlib, zlen);
    hash_crc32_appends(&crc, zlib, zlen);
    free(zlib);
    SIGN;
  }
  stbiw__write_smalln(s, CLIT(uint32[]){0, 0x444e4549, 0x826042ae}, 12);
  stbiw__write_flush(s);
#undef WRCB
#undef WRCRC32
#undef WR32
#undef HEAD
#undef SIGN
  return true;
}

/* ***************************************************************************
 *
 * JPEG writer
 *
 * This is based on Jon Olick's jo_jpeg.cpp:
 * public domain Simple, Minimalistic JPEG writer - http://www.jonolick.com/code.html
 */

static const ubyte stbiw__jpg_ZigZag[] = {0, 1, 5, 6, 14, 15, 27, 28, 2, 4, 7, 13, 16, 26, 29, 42, 3, 8, 12, 17, 25, 30, 41, 43, 9, 11, 18,
                                                  24, 31, 40, 44, 53, 10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38, 46, 51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63};
static void stbiw__jpg_writeBits(stbi__write_context *s, int *bitBufP, int *bitCntP, const ushrt *bs) {
  int bitBuf = *bitBufP, bitCnt = *bitCntP;
  bitCnt += bs[1];
  bitBuf |= bs[0] << (24 - bitCnt);
  while (bitCnt >= 8) {
    ubyte c = (bitBuf >> 16) & 255;
    stbiw__putc(s, c);
    if (c == 255) {
      stbiw__putc(s, 0);
    }
    bitBuf <<= 8;
    bitCnt -= 8;
  }
  *bitBufP = bitBuf;
  *bitCntP = bitCnt;
}
static void stbiw__jpg_DCT(float *d0p, float *d1p, float *d2p, float *d3p, float *d4p, float *d5p, float *d6p, float *d7p) {
  float d0 = *d0p, d1 = *d1p, d2 = *d2p, d3 = *d3p, d4 = *d4p, d5 = *d5p, d6 = *d6p, d7 = *d7p;
  float z1, z2, z3, z4, z5, z11, z13;

  float tmp0 = d0 + d7;
  float tmp7 = d0 - d7;
  float tmp1 = d1 + d6;
  float tmp6 = d1 - d6;
  float tmp2 = d2 + d5;
  float tmp5 = d2 - d5;
  float tmp3 = d3 + d4;
  float tmp4 = d3 - d4;

  // Even part
  float tmp10 = tmp0 + tmp3; // phase 2
  float tmp13 = tmp0 - tmp3;
  float tmp11 = tmp1 + tmp2;
  float tmp12 = tmp1 - tmp2;

  d0 = tmp10 + tmp11; // phase 3
  d4 = tmp10 - tmp11;

  z1 = (tmp12 + tmp13) * 0.707106781f; // c4
  d2 = tmp13 + z1;                     // phase 5
  d6 = tmp13 - z1;

  // Odd part
  tmp10 = tmp4 + tmp5; // phase 2
  tmp11 = tmp5 + tmp6;
  tmp12 = tmp6 + tmp7;

  // The rotator is modified from fig 4-8 to avoid extra negations.
  z5 = (tmp10 - tmp12) * 0.382683433f; // c6
  z2 = tmp10 * 0.541196100f + z5;      // c2-c6
  z4 = tmp12 * 1.306562965f + z5;      // c2+c6
  z3 = tmp11 * 0.707106781f;           // c4

  z11 = tmp7 + z3; // phase 5
  z13 = tmp7 - z3;

  *d5p = z13 + z2; // phase 6
  *d3p = z13 - z2;
  *d1p = z11 + z4;
  *d7p = z11 - z4;

  *d0p = d0;
  *d2p = d2;
  *d4p = d4;
  *d6p = d6;
}
static void stbiw__jpg_calcBits(int val, ushrt bits[2]) {
  int tmp1 = val < 0 ? -val : val;
  val = val < 0 ? val - 1 : val;
  bits[1] = 1;
  while (tmp1 >>= 1) {
    ++bits[1];
  }
  bits[0] = val & ((1 << bits[1]) - 1);
}
static int stbiw__jpg_processDU(stbi__write_context *s, int *bitBuf, int *bitCnt, float *CDU, int du_stride, float *fdtbl, int DC, const ushrt HTDC[256][2], const ushrt HTAC[256][2]) {
  const ushrt EOB[2] = {HTAC[0x00][0], HTAC[0x00][1]};
  const ushrt M16zeroes[2] = {HTAC[0xF0][0], HTAC[0xF0][1]};
  int dataOff, i, j, n, diff, end0pos, x, y;
  int DU[64];

  // DCT rows
  for (dataOff = 0, n = du_stride * 8; dataOff < n; dataOff += du_stride) {
    stbiw__jpg_DCT(&CDU[dataOff], &CDU[dataOff + 1], &CDU[dataOff + 2], &CDU[dataOff + 3], &CDU[dataOff + 4], &CDU[dataOff + 5], &CDU[dataOff + 6], &CDU[dataOff + 7]);
  }
  // DCT columns
  for (dataOff = 0; dataOff < 8; ++dataOff) {
    stbiw__jpg_DCT(&CDU[dataOff], &CDU[dataOff + du_stride], &CDU[dataOff + du_stride * 2], &CDU[dataOff + du_stride * 3], &CDU[dataOff + du_stride * 4],
                   &CDU[dataOff + du_stride * 5], &CDU[dataOff + du_stride * 6], &CDU[dataOff + du_stride * 7]);
  }
  // Quantize/descale/zigzag the coefficients
  for (y = 0, j = 0; y < 8; ++y) {
    for (x = 0; x < 8; ++x, ++j) {
      float v;
      i = y * du_stride + x;
      v = CDU[i] * fdtbl[j];
      // DU[stbiw__jpg_ZigZag[j]] = (int)(v < 0 ? ceilf(v - 0.5f) : floorf(v + 0.5f));
      // ceilf() and floorf() are C99, not C89, but I /think/ they're not needed here anyway?
      DU[stbiw__jpg_ZigZag[j]] = (int)(v < 0 ? v - 0.5f : v + 0.5f);
    }
  }

  // Encode DC
  diff = DU[0] - DC;
  if (diff == 0) {
    stbiw__jpg_writeBits(s, bitBuf, bitCnt, HTDC[0]);
  } else {
    ushrt bits[2];
    stbiw__jpg_calcBits(diff, bits);
    stbiw__jpg_writeBits(s, bitBuf, bitCnt, HTDC[bits[1]]);
    stbiw__jpg_writeBits(s, bitBuf, bitCnt, bits);
  }
  // Encode ACs
  end0pos = 63;
  for (; (end0pos > 0) && (DU[end0pos] == 0); --end0pos) {
  }
  // end0pos = first element in reverse order !=0
  if (end0pos == 0) {
    stbiw__jpg_writeBits(s, bitBuf, bitCnt, EOB);
    return DU[0];
  }
  for (i = 1; i <= end0pos; ++i) {
    int startpos = i;
    int nrzeroes;
    ushrt bits[2];
    for (; DU[i] == 0 && i <= end0pos; ++i) {
    }
    nrzeroes = i - startpos;
    if (nrzeroes >= 16) {
      int lng = nrzeroes >> 4;
      int nrmarker;
      for (nrmarker = 1; nrmarker <= lng; ++nrmarker)
        stbiw__jpg_writeBits(s, bitBuf, bitCnt, M16zeroes);
      nrzeroes &= 15;
    }
    stbiw__jpg_calcBits(DU[i], bits);
    stbiw__jpg_writeBits(s, bitBuf, bitCnt, HTAC[(nrzeroes << 4) + bits[1]]);
    stbiw__jpg_writeBits(s, bitBuf, bitCnt, bits);
  }
  if (end0pos != 63) {
    stbiw__jpg_writeBits(s, bitBuf, bitCnt, EOB);
  }
  return DU[0];
}
static bool stbi_write_jpg_core(stbi__write_context *s, int width, int height, int comp, const void *data, int quality) {
  // Constants that don't pollute global namespace
  static const ubyte std_dc_luminance_nrcodes[] = {0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
  static const ubyte std_dc_luminance_values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
  static const ubyte std_ac_luminance_nrcodes[] = {0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d};
  static const ubyte std_ac_luminance_values[] = {
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa};
  static const ubyte std_dc_chrominance_nrcodes[] = {0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
  static const ubyte std_dc_chrominance_values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
  static const ubyte std_ac_chrominance_nrcodes[] = {0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77};
  static const ubyte std_ac_chrominance_values[] = {
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa};
  // Huffman tables
  static const ushrt YDC_HT[256][2] = {{0, 2}, {2, 3}, {3, 3}, {4, 3}, {5, 3}, {6, 3}, {14, 4}, {30, 5}, {62, 6}, {126, 7}, {254, 8}, {510, 9}};
  static const ushrt UVDC_HT[256][2] = {{0, 2}, {1, 2}, {2, 2}, {6, 3}, {14, 4}, {30, 5}, {62, 6}, {126, 7}, {254, 8}, {510, 9}, {1022, 10}, {2046, 11}};
  static const ushrt YAC_HT[256][2] = {
    {10, 4}, {0, 2}, {1, 2}, {4, 3}, {11, 4}, {26, 5}, {120, 7}, {248, 8}, {1014, 10}, {65410, 16}, {65411, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {12, 4}, {27, 5}, {121, 7}, {502, 9}, {2038, 11}, {65412, 16}, {65413, 16}, {65414, 16}, {65415, 16}, {65416, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {28, 5}, {249, 8}, {1015, 10}, {4084, 12}, {65417, 16}, {65418, 16}, {65419, 16}, {65420, 16}, {65421, 16}, {65422, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {58, 6}, {503, 9}, {4085, 12}, {65423, 16}, {65424, 16}, {65425, 16}, {65426, 16}, {65427, 16}, {65428, 16}, {65429, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {59, 6}, {1016, 10}, {65430, 16}, {65431, 16}, {65432, 16}, {65433, 16}, {65434, 16}, {65435, 16}, {65436, 16}, {65437, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {122, 7}, {2039, 11}, {65438, 16}, {65439, 16}, {65440, 16}, {65441, 16}, {65442, 16}, {65443, 16}, {65444, 16}, {65445, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {123, 7}, {4086, 12}, {65446, 16}, {65447, 16}, {65448, 16}, {65449, 16}, {65450, 16}, {65451, 16}, {65452, 16}, {65453, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {250, 8}, {4087, 12}, {65454, 16}, {65455, 16}, {65456, 16}, {65457, 16}, {65458, 16}, {65459, 16}, {65460, 16}, {65461, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {504, 9}, {32704, 15}, {65462, 16}, {65463, 16}, {65464, 16}, {65465, 16}, {65466, 16}, {65467, 16}, {65468, 16}, {65469, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {505, 9}, {65470, 16}, {65471, 16}, {65472, 16}, {65473, 16}, {65474, 16}, {65475, 16}, {65476, 16}, {65477, 16}, {65478, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {506, 9}, {65479, 16}, {65480, 16}, {65481, 16}, {65482, 16}, {65483, 16}, {65484, 16}, {65485, 16}, {65486, 16}, {65487, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {1017, 10}, {65488, 16}, {65489, 16}, {65490, 16}, {65491, 16}, {65492, 16}, {65493, 16}, {65494, 16}, {65495, 16}, {65496, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {1018, 10}, {65497, 16}, {65498, 16}, {65499, 16}, {65500, 16}, {65501, 16}, {65502, 16}, {65503, 16}, {65504, 16}, {65505, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {2040, 11}, {65506, 16}, {65507, 16}, {65508, 16}, {65509, 16}, {65510, 16}, {65511, 16}, {65512, 16}, {65513, 16}, {65514, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {65515, 16}, {65516, 16}, {65517, 16}, {65518, 16}, {65519, 16}, {65520, 16}, {65521, 16}, {65522, 16}, {65523, 16}, {65524, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {2041, 11}, {65525, 16}, {65526, 16}, {65527, 16}, {65528, 16}, {65529, 16}, {65530, 16}, {65531, 16}, {65532, 16}, {65533, 16}, {65534, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  static const ushrt UVAC_HT[256][2] = {
    {0, 2}, {1, 2}, {4, 3}, {10, 4}, {24, 5}, {25, 5}, {56, 6}, {120, 7}, {500, 9}, {1014, 10}, {4084, 12}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {11, 4}, {57, 6}, {246, 8}, {501, 9}, {2038, 11}, {4085, 12}, {65416, 16}, {65417, 16}, {65418, 16}, {65419, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {26, 5}, {247, 8}, {1015, 10}, {4086, 12}, {32706, 15}, {65420, 16}, {65421, 16}, {65422, 16}, {65423, 16}, {65424, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {27, 5}, {248, 8}, {1016, 10}, {4087, 12}, {65425, 16}, {65426, 16}, {65427, 16}, {65428, 16}, {65429, 16}, {65430, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {58, 6}, {502, 9}, {65431, 16}, {65432, 16}, {65433, 16}, {65434, 16}, {65435, 16}, {65436, 16}, {65437, 16}, {65438, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {59, 6}, {1017, 10}, {65439, 16}, {65440, 16}, {65441, 16}, {65442, 16}, {65443, 16}, {65444, 16}, {65445, 16}, {65446, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {121, 7}, {2039, 11}, {65447, 16}, {65448, 16}, {65449, 16}, {65450, 16}, {65451, 16}, {65452, 16}, {65453, 16}, {65454, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {122, 7}, {2040, 11}, {65455, 16}, {65456, 16}, {65457, 16}, {65458, 16}, {65459, 16}, {65460, 16}, {65461, 16}, {65462, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {249, 8}, {65463, 16}, {65464, 16}, {65465, 16}, {65466, 16}, {65467, 16}, {65468, 16}, {65469, 16}, {65470, 16}, {65471, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {503, 9}, {65472, 16}, {65473, 16}, {65474, 16}, {65475, 16}, {65476, 16}, {65477, 16}, {65478, 16}, {65479, 16}, {65480, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {504, 9}, {65481, 16}, {65482, 16}, {65483, 16}, {65484, 16}, {65485, 16}, {65486, 16}, {65487, 16}, {65488, 16}, {65489, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {505, 9}, {65490, 16}, {65491, 16}, {65492, 16}, {65493, 16}, {65494, 16}, {65495, 16}, {65496, 16}, {65497, 16}, {65498, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {506, 9}, {65499, 16}, {65500, 16}, {65501, 16}, {65502, 16}, {65503, 16}, {65504, 16}, {65505, 16}, {65506, 16}, {65507, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {2041, 11}, {65508, 16}, {65509, 16}, {65510, 16}, {65511, 16}, {65512, 16}, {65513, 16}, {65514, 16}, {65515, 16}, {65516, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {16352, 14}, {65517, 16}, {65518, 16}, {65519, 16}, {65520, 16}, {65521, 16}, {65522, 16}, {65523, 16}, {65524, 16}, {65525, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {1018, 10}, {32707, 15}, {65526, 16}, {65527, 16}, {65528, 16}, {65529, 16}, {65530, 16}, {65531, 16}, {65532, 16}, {65533, 16}, {65534, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  static const int YQT[] = {16, 11, 10, 16, 24, 40, 51, 61, 12, 12, 14, 19, 26, 58, 60, 55, 14, 13, 16, 24, 40, 57, 69, 56, 14, 17, 22, 29, 51, 87, 80, 62, 18, 22,
                            37, 56, 68, 109, 103, 77, 24, 35, 55, 64, 81, 104, 113, 92, 49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99};
  static const int UVQT[] = {17, 18, 24, 47, 99, 99, 99, 99, 18, 21, 26, 66, 99, 99, 99, 99, 24, 26, 56, 99, 99, 99, 99, 99, 47, 66, 99, 99, 99, 99, 99, 99,
                             99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99};
  static const float aasf[] = {1.0f * 2.828427125f, 1.387039845f * 2.828427125f, 1.306562965f * 2.828427125f, 1.175875602f * 2.828427125f,
                               1.0f * 2.828427125f, 0.785694958f * 2.828427125f, 0.541196100f * 2.828427125f, 0.275899379f * 2.828427125f};

  int row, col, i, k, subsample;
  float fdtbl_Y[64], fdtbl_UV[64];
  ubyte YTable[64], UVTable[64];

  if (!data || !width || !height || comp > 4 || comp < 1) {
    return false;
  }

  quality = quality ? quality : 90;
  subsample = quality <= 90 ? 1 : 0;
  quality = quality < 1 ? 1 : quality > 100 ? 100
                                            : quality;
  quality = quality < 50 ? 5000 / quality : 200 - quality * 2;

  for (i = 0; i < 64; ++i) {
    int uvti, yti = (YQT[i] * quality + 50) / 100;
    YTable[stbiw__jpg_ZigZag[i]] = (ubyte)(yti < 1 ? 1 : yti > 255 ? 255
                                                                           : yti);
    uvti = (UVQT[i] * quality + 50) / 100;
    UVTable[stbiw__jpg_ZigZag[i]] = (ubyte)(uvti < 1 ? 1 : uvti > 255 ? 255
                                                                              : uvti);
  }

  for (row = 0, k = 0; row < 8; ++row) {
    for (col = 0; col < 8; ++col, ++k) {
      fdtbl_Y[k] = 1 / (YTable[stbiw__jpg_ZigZag[k]] * aasf[row] * aasf[col]);
      fdtbl_UV[k] = 1 / (UVTable[stbiw__jpg_ZigZag[k]] * aasf[row] * aasf[col]);
    }
  }

  // Write Headers
  {
    static const ubyte head0[] = {0xFF, 0xD8, 0xFF, 0xE0, 0, 0x10, 'J', 'F', 'I', 'F', 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0xFF, 0xDB, 0, 0x84, 0};
    static const ubyte head2[] = {0xFF, 0xDA, 0, 0xC, 3, 1, 0, 2, 0x11, 3, 0x11, 0, 0x3F, 0};
    const ubyte head1[] = {0xFF, 0xC0, 0, 0x11, 8, (ubyte)(height >> 8), (ubyte)(height), (ubyte)(width >> 8), (ubyte)(width),
                                   3, 1, (ubyte)(subsample ? 0x22 : 0x11), 0, 2, 0x11, 1, 3, 0x11, 1, 0xFF, 0xC4, 0x01, 0xA2, 0};
    s->func(s->context, (void *)head0, sizeof(head0));
    s->func(s->context, (void *)YTable, sizeof(YTable));
    stbiw__putc(s, 1);
    s->func(s->context, UVTable, sizeof(UVTable));
    s->func(s->context, (void *)head1, sizeof(head1));
    s->func(s->context, (void *)(std_dc_luminance_nrcodes + 1), sizeof(std_dc_luminance_nrcodes) - 1);
    s->func(s->context, (void *)std_dc_luminance_values, sizeof(std_dc_luminance_values));
    stbiw__putc(s, 0x10); // HTYACinfo
    s->func(s->context, (void *)(std_ac_luminance_nrcodes + 1), sizeof(std_ac_luminance_nrcodes) - 1);
    s->func(s->context, (void *)std_ac_luminance_values, sizeof(std_ac_luminance_values));
    stbiw__putc(s, 1); // HTUDCinfo
    s->func(s->context, (void *)(std_dc_chrominance_nrcodes + 1), sizeof(std_dc_chrominance_nrcodes) - 1);
    s->func(s->context, (void *)std_dc_chrominance_values, sizeof(std_dc_chrominance_values));
    stbiw__putc(s, 0x11); // HTUACinfo
    s->func(s->context, (void *)(std_ac_chrominance_nrcodes + 1), sizeof(std_ac_chrominance_nrcodes) - 1);
    s->func(s->context, (void *)std_ac_chrominance_values, sizeof(std_ac_chrominance_values));
    s->func(s->context, (void *)head2, sizeof(head2));
  }

  // Encode 8x8 macroblocks
  {
    static const ushrt fillBits[] = {0x7F, 7};
    int DCY = 0, DCU = 0, DCV = 0;
    int bitBuf = 0, bitCnt = 0;
    // comp == 2 is grey+alpha (alpha is ignored)
    int ofsG = comp > 2 ? 1 : 0, ofsB = comp > 2 ? 2 : 0;
    const ubyte *dataR = (const ubyte *)data;
    const ubyte *dataG = dataR + ofsG;
    const ubyte *dataB = dataR + ofsB;
    int x, y, pos;
    if (subsample) {
      for (y = 0; y < height; y += 16) {
        for (x = 0; x < width; x += 16) {
          float Y[256], U[256], V[256];
          for (row = y, pos = 0; row < y + 16; ++row) {
            // row >= height => use last input row
            int clamped_row = (row < height) ? row : height - 1;
#ifdef VERTICALLY_FLIP
            int base_p = (height - 1 - clamped_row) * width * comp;
#else
            int base_p = clamped_row * width * comp;
#endif // VERTICALLY_FLIP
            for (col = x; col < x + 16; ++col, ++pos) {
              // if col >= width => use pixel from last input column
              int p = base_p + ((col < width) ? col : (width - 1)) * comp;
              float r = dataR[p], g = dataG[p], b = dataB[p];
              Y[pos] = +0.29900f * r + 0.58700f * g + 0.11400f * b - 128;
              U[pos] = -0.16874f * r - 0.33126f * g + 0.50000f * b;
              V[pos] = +0.50000f * r - 0.41869f * g - 0.08131f * b;
            }
          }
          DCY = stbiw__jpg_processDU(s, &bitBuf, &bitCnt, Y + 0, 16, fdtbl_Y, DCY, YDC_HT, YAC_HT);
          DCY = stbiw__jpg_processDU(s, &bitBuf, &bitCnt, Y + 8, 16, fdtbl_Y, DCY, YDC_HT, YAC_HT);
          DCY = stbiw__jpg_processDU(s, &bitBuf, &bitCnt, Y + 128, 16, fdtbl_Y, DCY, YDC_HT, YAC_HT);
          DCY = stbiw__jpg_processDU(s, &bitBuf, &bitCnt, Y + 136, 16, fdtbl_Y, DCY, YDC_HT, YAC_HT);

          // subsample U,V
          {
            float subU[64], subV[64];
            int yy, xx;
            for (yy = 0, pos = 0; yy < 8; ++yy) {
              for (xx = 0; xx < 8; ++xx, ++pos) {
                int j = yy * 32 + xx * 2;
                subU[pos] = (U[j + 0] + U[j + 1] + U[j + 16] + U[j + 17]) * 0.25f;
                subV[pos] = (V[j + 0] + V[j + 1] + V[j + 16] + V[j + 17]) * 0.25f;
              }
            }
            DCU = stbiw__jpg_processDU(s, &bitBuf, &bitCnt, subU, 8, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
            DCV = stbiw__jpg_processDU(s, &bitBuf, &bitCnt, subV, 8, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
          }
        }
      }
    } else {
      for (y = 0; y < height; y += 8) {
        for (x = 0; x < width; x += 8) {
          float Y[64], U[64], V[64];
          for (row = y, pos = 0; row < y + 8; ++row) {
            // row >= height => use last input row
            int clamped_row = (row < height) ? row : height - 1;
#ifdef VERTICALLY_FLIP
            int base_p = (height - 1 - clamped_row) * width * comp;
#else
            int base_p = clamped_row * width * comp;
#endif // VERTICALLY_FLIP
            for (col = x; col < x + 8; ++col, ++pos) {
              // if col >= width => use pixel from last input column
              int p = base_p + ((col < width) ? col : (width - 1)) * comp;
              float r = dataR[p], g = dataG[p], b = dataB[p];
              Y[pos] = +0.29900f * r + 0.58700f * g + 0.11400f * b - 128;
              U[pos] = -0.16874f * r - 0.33126f * g + 0.50000f * b;
              V[pos] = +0.50000f * r - 0.41869f * g - 0.08131f * b;
            }
          }

          DCY = stbiw__jpg_processDU(s, &bitBuf, &bitCnt, Y, 8, fdtbl_Y, DCY, YDC_HT, YAC_HT);
          DCU = stbiw__jpg_processDU(s, &bitBuf, &bitCnt, U, 8, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
          DCV = stbiw__jpg_processDU(s, &bitBuf, &bitCnt, V, 8, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
        }
      }
    }

    // Do the bit alignment of the EOI marker
    stbiw__jpg_writeBits(s, &bitBuf, &bitCnt, fillBits);
  }

  // EOI
  stbiw__putc(s, 0xFF);
  stbiw__putc(s, 0xD9);

  return true;
}
// most static

static void image_io_write(void *usr, void *data, iter size) {
  fwrite(data, size, CAST(FILE*)usr);
}
static void image_mem_write(void *usr, void *data, iter size) {
  image_file *imf = CAST(image_file*)usr;
  while (imf->cap < (imf->len + size)) {
    imf->cap <<= 2;
    imf->data = CAST(ubyte*)realloc(imf->data, imf->cap);
  }
  memcpy(imf->data + imf->len, data, size);
  imf->len += size;
}
// global

bool image_write_opt(char const *filename,const image_bitmap bitmap, image_file_format format, image_file_opt opt) {
  bool r = false;
  FILE *f = fopen(filename, "wb");
  if (f) {
    r = image_write_to_func_opt(image_io_write, CAST(void *)f, bitmap, format, opt);
    fclose(f);
  } else {
    stb_set_error("Fail to open file!");
  }
  return r;
}
image_file image_write_to_mem_opt(const image_bitmap bitmap, image_file_format format, image_file_opt opt) {
  image_file imf = {
    .len = 0,
    .cap = 2048,
    .data = CAST(ubyte*)malloc(2048),
  };
  if (imf.data) {
    if (image_write_to_func_opt(image_mem_write, CAST(void *)&imf, bitmap, format, opt)) {
      imf.data = CAST(ubyte*)realloc(imf.data, imf.len);
      imf.cap = imf.len;
    } else {
      free(CAST(void*)imf.data);
      stb_set_error("Failure!");
      memset(&imf, 0, sizeof(imf));
    }
  } else {
    stb_set_error("Failure!");
    memset(&imf, 0, sizeof(imf));
  }
  return imf;
}
bool image_write_to_func_opt(stbi_write_func *funct, void *user, const image_bitmap bitmap, image_file_format format, image_file_opt opt) {
  bool r = false;
  stbi__write_context s = {
    .func = funct, 
    .context = user,
    .buf_used = 0,
  };
  switch (format) {
    case ImageFile_JPG:
      r = stbi_write_jpg_core(&s, bitmap.w, bitmap.h, bitmap.chnl, bitmap.data, opt.quality);
      break;
    case ImageFile_PNG:
      r = stbi_write_png_core(&s, bitmap.w, bitmap.h, bitmap.chnl, bitmap.data);
      break;
    case ImageFile_BMP:
      r = stbi_write_bmp_core(&s, bitmap.w, bitmap.h, bitmap.chnl, bitmap.data);
      break;
    case ImageFile_TGA:
      r = stbi_write_tga_core(&s, bitmap.w, bitmap.h, bitmap.chnl, bitmap.data);
      break;
    case ImageFile_HDR:
      if (bitmap.bpc == 32)
        r = stbi_write_hdr_core(&s, bitmap.w, bitmap.h, bitmap.chnl, CAST(float*)bitmap.data);
      else
        stb_set_error("Unknown hdr need float format format");
      break;
    default:
      stb_set_error("Unknown Image file format");
      break;
  }
  return r;
}

