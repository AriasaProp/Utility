/* stb_image - v2.30 - public domain image loader - http://nothings.org/stb
  no warranty implied; use at your own risk
*/
#include "stb/local.h"
#include "stb/image.h"
#include "stb/zlib.h"
#include "algorithm/hash.h"

#include <stddef.h> // ptrdiff_t on osx

// for image formats that explicitly notate that they have premultiplied alpha,
// we just return the colors as stored in the file. set this flag to force
// unpremultiplication. results are undefined if the unpremultiply overflow.
// #define UNPREMULTIPLY
// indicate whether we should process iphone images back to canonical format,
// or just pass them through "as-is"
#define IPHONE_PNG_TO_RGB
// flip the image result vertically, so the first pixel is the bottom left
// #define VERTICALLY_FLIP



// x86/x64 detection
#if defined(__x86_64__) || defined(_M_X64)
#define image__X64_TARGET
#elif defined(__i386) || defined(_M_IX86)
#define image__X86_TARGET
#endif

#if defined(__GNUC__) && defined(image__X86_TARGET) && !defined(__SSE2__) && !defined(STBI_NO_SIMD)
// gcc doesn't support sse2 intrinsics unless you compile with -msse2,
// which in turn means it gets to use SSE2 everywhere. This is unfortunate,
// but previous attempts to provide the SSE2 functions with runtime
// detection caused numerous issues. The way architecture extensions are
// exposed in GCC/Clang is, sadly, not really suited for one-file libs.
// New behavior: if compiled with -msse2, we use SSE2 without any
// detection; if not, we don't use it at all.
#define STBI_NO_SIMD
#endif

#if defined(__MINGW32__) && defined(image__X86_TARGET) && !defined(STBI_MINGW_ENABLE_SSE2) && !defined(STBI_NO_SIMD)
// Note that __MINGW32__ doesn't actually mean 32-bit, so we have to avoid image__X64_TARGET
//
// 32-bit MinGW wants ESP to be 16-byte aligned, but this is not in the
// Windows ABI and VC++ as well as Windows DLLs don't maintain that invariant.
// As a result, enabling SSE2 on 32-bit MinGW is dangerous when not
// simultaneously enabling "-mstackrealign".
//
// See https://github.com/nothings/stb/issues/81 for more information.
//
// So default to no SSE2 on 32-bit MinGW. If you've read this far and added
// -mstackrealign to your build settings, feel free to #define STBI_MINGW_ENABLE_SSE2.
#define STBI_NO_SIMD
#endif

#if !defined(STBI_NO_SIMD) && (defined(image__X86_TARGET) || defined(image__X64_TARGET))
#define STBI_SSE2
#include <emmintrin.h>

#ifdef _MSC_VER

#if _MSC_VER >= 1400 // not VC6
#include <intrin.h>  // __cpuid
static int image__cpuid3(void) {
  int info[4];
  __cpuid(info, 1);
  return info[3];
}
#else
static int image__cpuid3(void) {
  int res;
  __asm {
      mov  eax,1
      cpuid
      mov  res,edx
  }
  return res;
}
#endif

#define STBI_SIMD_ALIGN(type, name) __declspec(align(16)) type name

#ifdef STBI_SSE2
static bool image__sse2_available(void) {
  return (image__cpuid3() & (1 << 26)) != 0;
}
#endif

#else // assume GCC-style if not VC++
#define STBI_SIMD_ALIGN(type, name) type name __attribute__((aligned(16)))

#if !defined(STBI_NO_JPEG) && defined(STBI_SSE2)
static bool image__sse2_available(void) {
  // If we're even attempting to compile this on GCC/Clang, that means
  // -msse2 is on, which means the compiler is allowed to use SSE2
  // instructions at will, and so are we.
  return true;
}
#endif

#endif
#endif

// ARM NEON
#if defined(STBI_NO_SIMD) && defined(STBI_NEON)
#undef STBI_NEON
#endif

#ifdef STBI_NEON
#include <arm_neon.h>
#ifdef _MSC_VER
#define STBI_SIMD_ALIGN(type, name) __declspec(align(16)) type name
#else
#define STBI_SIMD_ALIGN(type, name) type name __attribute__((aligned(16)))
#endif
#endif

#ifndef STBI_SIMD_ALIGN
#define STBI_SIMD_ALIGN(type, name) type name
#endif

///////////////////////////////////////////////
//
//  image__context struct and functions

typedef struct {
  void *user;
  iter (*read)(void*,void*,iter); // fill 'data' with 'size' bytes.  return number of bytes actually read
  void (*skip)(void*,int); // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
  void (*rewind)(void*);
  bool (*eof)(void*);

  union {
    byte  b[128];
    shrt  s[ 64];
    int32 i[ 32];
    int64 l[ 16];
    ubyte  ub[128];
    ushrt  us[ 64];
    uint32 ui[ 32];
    uint64 ul[ 16];
  } buffer;
} image__context;

static inline bool image__validadd(iter a, iter b) {
  return (a + b) >= b;
}
static inline bool image__validmul(iter a, iter b) {
  return sizeof(iter) >= (util_bitlead(a) + util_bitlead(b));
}
static inline bool image__validmad(iter a, iter b, iter add) {
  return image__validmul(a, b) && image__validadd(a * b, add);
}
static inline bool image__validm3ad(iter a, iter b, iter c, iter add) {
  return image__validmul(a, b) && image__validmul(a * b, c) && image__validadd(a * b * c, add);
}
static inline bool image__validm4ad(iter a, iter b, iter c, iter d, iter add) {
  return image__validmul(a, b) && image__validmul(a * b, c) && image__validmul(a * b * c, d) && image__validadd(a * b * c * d, add);
}
static void *image__malloc_mad2(iter a, iter b, iter add) {
  if (!image__validmad(a, b, add)) return NULL;
  return util_malloc(a * b + add);
}
static void *image__malloc_mad3(iter a, iter b, iter c, iter add) {
  if (!image__validm3ad(a, b, c, add)) return NULL;
  return util_malloc(a * b * c + add);
}
static void *image__malloc_mad4(iter a, iter b, iter c, iter d, iter add) {
  if (!image__validm4ad(a, b, c, d, add)) return NULL;
  return util_malloc(a * b * c * d + add);
}
// returns 1 if the sum of two signed ints is valid (between -2^31 and 2^31-1 inclusive), 0 on overflow.
static bool image__addints_valid(int a, int b) {
  if ((a >= 0) != (b >= 0))
    return true; // a and b have different signs, so no overflow
  if (a < 0 && b < 0)
    return a >= INT_MIN - b; // same as a + b >= INT_MIN; INT_MIN - b cannot overflow since b < 0.
  return a <= INT_MAX - b;
}
// returns 1 if the product of two ints fits in a signed shrt, 0 on overflow.
static bool image__mul2shrts_valid(int a, int b) {
  if (b == 0 || b == -1) return true; // multiplication by 0 is always 0; check for -1 so SHRT_MIN/b doesn't overflow
  if ((a >= 0) == (b >= 0))
    return a <= SHRT_MAX / b; // product is positive, so similar to mul2sizes_valid
  if (b < 0)
    return a <= SHRT_MIN / b; // same as a * b >= SHRT_MIN
  return a >= SHRT_MIN / b;
}

static float *image__ldr_to_hdr(ubyte *data, int x, int y, int comp);
static ubyte *image__hdr_to_ldr(float *data, int x, int y, int comp);

static ubyte *image__convert_16_to_8(ushrt *orig, int w, int h, int channels) {
  int i;
  int img_len = w * h * channels;
  ubyte *reduced;

  reduced = (ubyte *)util_malloc(img_len);
  if (reduced == NULL)
    return (ubyte *)(iter)stb_set_error("outofmem : Out of memory");

  for (i = 0; i < img_len; ++i)
    reduced[i] = (ubyte)((orig[i] >> 8) & 0xFF); // top half of each byte is sufficient approx of 16->8 bit scaling

  util_memfree(orig);
  return reduced;
}
static ushrt *image__convert_8_to_16(ubyte *orig, int w, int h, int channels) {
  int i;
  int img_len = w * h * channels;
  ushrt *enlarged;

  enlarged = (ushrt *)util_malloc(img_len * 2);
  if (enlarged == NULL)
    return (ushrt *)(ubyte *)(iter)stb_set_error("outofmem : Out of memory");

  for (i = 0; i < img_len; ++i)
    enlarged[i] = (ushrt)((orig[i] << 8) + orig[i]); // replicate to high and low byte, maps 0->0, 255->0xffff

  util_memfree(orig);
  return enlarged;
}
static void image__vertical_flip(void *image, iter w, iter h, iter bytes_per_pixel) {
  const iter bytes_per_row = w * bytes_per_pixel;
  ubyte *bytes = CAST(ubyte *)image;
  for (iter top = 0, bot = (h - 1) * bytes_per_row; top < bot; top += bytes_per_row, bot -= bytes_per_row)
    util_memswap(bytes + top * bytes_per_row);
}
static void image__vertical_flip_slices(void *image, int w, int h, int z, int bytes_per_pixel) {
  int slice;
  int slice_size = w * h * bytes_per_pixel;

  ubyte *bytes = (ubyte *)image;
  for (slice = 0; slice < z; ++slice) {
    image__vertical_flip(bytes, w, h, bytes_per_pixel);
    bytes += slice_size;
  }
}

#ifndef STBI_NO_LINEAR
static float image__l2h_gamma = 2.2f, image__l2h_scale = 1.0f;

void stbi_ldr_to_hdr_gamma(float gamma) { image__l2h_gamma = gamma; }
void stbi_ldr_to_hdr_scale(float scale) { image__l2h_scale = scale; }
#endif

static float image__h2l_gamma_i = 1.0f / 2.2f, image__h2l_scale_i = 1.0f;

void stbi_hdr_to_ldr_gamma(float gamma) { image__h2l_gamma_i = 1 / gamma; }
void stbi_hdr_to_ldr_scale(float scale) { image__h2l_scale_i = 1 / scale; }

static ushrt image__get8(image__context *s) {
  if (1 != s->read(s->user, s->buffer.ub, 1))
    ASSERT(0, "Failed 8 bytes read personal");
  return s->buffer.us;
}
static ushrt image__get16le(image__context *s) {
  if (2 != s->read(s->user, s->buffer.ub, 2))
    ASSERT(0, "Failed 16 bytes read personal");
  return s->buffer.us;
}
static uint32 image__get32le(image__context *s) {
  if (4 != s->read(s->user, s->buffer.ub, 4))
    ASSERT(0, "Failed 32 bytes read personal");
  return s->buffer.ui;
}
static uint64 image__get64le(image__context *s) {
  if (8 != s->read(s->user, s->buffer.ub, 8))
    ASSERT(0, "Failed 64 bytes read personal");
  return s->buffer.ul;
}
static ushrt image__get16be(image__context *s) {
  if (2 != s->read(s->user, s->buffer.ub, 2))
    ASSERT(0, "Failed 16 bytes read personal");
  return imath_flip16(s->buffer.us);
}
static uint32 image__get32be(image__context *s) {
  if (4 != s->read(s->user, s->buffer.ub, 4))
    ASSERT(0, "Failed 32 bytes read personal");
  return imath_flip32(s->buffer.ui);
}
static uint64 image__get64be(image__context *s) {
  if (8 != s->read(s->user, s->buffer.ub, 8))
    ASSERT(0, "Failed 64 bytes read personal");
  return imath_flip64(s->buffer.ul);
}
static ubyte *image__readtemp(image__context *s, ubyte n) {
  if (n != s->read(s->user, s->buffer.ub, n))
    return NULL;
  return s->buffer.ub;
}
inline static iter image__read(image__context *s, void *dst, iter n) {
  return s->read(s->user, dst, n);
}
inline static void image__skip(image__context *s, int n) {
  return s->skip(s->user, n);
}
inline static void image__rewind(image__context *s) {
  return s->rewind(s->user);
}
inline static bool image__eof(image__context *s) {
  return s->eof(s->user);
}

#define image__BYTECAST(x) ((ubyte)((x) & 255)) // truncate int to byte without warnings
static ubyte image__compute_y(int r, int g, int b) {
  return CAST(ubyte)(((r * 77) + (g * 150) + (29 * b)) >> 8);
}
static ubyte *image__convert_format(ubyte *data, int img_n, int req_comp, uint x, uint y) {
  int i, j;
  ubyte *good;
  if (req_comp == img_n) return data;
  ASSERT(req_comp >= 1 && req_comp <= 4);

  good = (ubyte *)image__malloc_mad3(req_comp, x, y, 0);
  if (good == NULL) {
    util_memfree(data);
    return (ubyte *)(iter)stb_set_error("outofmem : Out of memory");
  }

  for (j = 0; j < (int)y; ++j) {
    ubyte *src = data + j * x * img_n;
    ubyte *dest = good + j * x * req_comp;

#define image__COMBO(a, b) (((a) << 3) | (b))
#define image__CASE(a, b)  \
  case image__COMBO(a, b): \
    for (i = x - 1; i >= 0; --i, src += a, dest += b)
    // convert source image with img_n components to one with req_comp components;
    // avoid switch per pixel, so use switch per scanline and massive macros
    switch (image__COMBO(img_n, req_comp)) {
      image__CASE(1, 2) {
        dest[0] = src[0];
        dest[1] = 255;
      }
      break;
      image__CASE(1, 3) { dest[0] = dest[1] = dest[2] = src[0]; }
      break;
      image__CASE(1, 4) {
        dest[0] = dest[1] = dest[2] = src[0];
        dest[3] = 255;
      }
      break;
      image__CASE(2, 1) { dest[0] = src[0]; }
      break;
      image__CASE(2, 3) { dest[0] = dest[1] = dest[2] = src[0]; }
      break;
      image__CASE(2, 4) {
        dest[0] = dest[1] = dest[2] = src[0];
        dest[3] = src[1];
      }
      break;
      image__CASE(3, 4) {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest[3] = 255;
      }
      break;
      image__CASE(3, 1) { dest[0] = image__compute_y(src[0], src[1], src[2]); }
      break;
      image__CASE(3, 2) {
        dest[0] = image__compute_y(src[0], src[1], src[2]);
        dest[1] = 255;
      }
      break;
      image__CASE(4, 1) { dest[0] = image__compute_y(src[0], src[1], src[2]); }
      break;
      image__CASE(4, 2) {
        dest[0] = image__compute_y(src[0], src[1], src[2]);
        dest[1] = src[3];
      }
      break;
      image__CASE(4, 3) {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
      }
      break;
    default:
      ASSERT(0);
      util_memfree(data);
      util_memfree(good);
      return (ubyte *)(iter)stb_set_error("unsupported : Unsupported format conversion");
    }
#undef image__CASE
  }

  util_memfree(data);
  return good;
}

static ushrt image__compute_y_16(int r, int g, int b) {
  return (ushrt)(((r * 77) + (g * 150) + (29 * b)) >> 8);
}
static ushrt *image__convert_format16(ushrt *data, int img_n, int req_comp, uint x, uint y) {
  int i, j;
  ushrt *good;

  if (req_comp == img_n)
    return data;
  ASSERT(req_comp >= 1 && req_comp <= 4);

  good = (ushrt *)util_malloc(req_comp * x * y * 2);
  if (good == NULL) {
    util_memfree(data);
    return (ushrt *)(ubyte *)(iter)stb_set_error("outofmem : Out of memory");
  }

  for (j = 0; j < (int)y; ++j) {
    ushrt *src = data + j * x * img_n;
    ushrt *dest = good + j * x * req_comp;

#define image__COMBO(a, b) ((a) * 8 + (b))
#define image__CASE(a, b)  \
  case image__COMBO(a, b): \
    for (i = x - 1; i >= 0; --i, src += a, dest += b)
    // convert source image with img_n components to one with req_comp components;
    // avoid switch per pixel, so use switch per scanline and massive macros
    switch (image__COMBO(img_n, req_comp)) {
      image__CASE(1, 2) {
        dest[0] = src[0];
        dest[1] = 0xffff;
      }
      break;
      image__CASE(1, 3) { dest[0] = dest[1] = dest[2] = src[0]; }
      break;
      image__CASE(1, 4) {
        dest[0] = dest[1] = dest[2] = src[0];
        dest[3] = 0xffff;
      }
      break;
      image__CASE(2, 1) { dest[0] = src[0]; }
      break;
      image__CASE(2, 3) { dest[0] = dest[1] = dest[2] = src[0]; }
      break;
      image__CASE(2, 4) {
        dest[0] = dest[1] = dest[2] = src[0];
        dest[3] = src[1];
      }
      break;
      image__CASE(3, 4) {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest[3] = 0xffff;
      }
      break;
      image__CASE(3, 1) { dest[0] = image__compute_y_16(src[0], src[1], src[2]); }
      break;
      image__CASE(3, 2) {
        dest[0] = image__compute_y_16(src[0], src[1], src[2]);
        dest[1] = 0xffff;
      }
      break;
      image__CASE(4, 1) { dest[0] = image__compute_y_16(src[0], src[1], src[2]); }
      break;
      image__CASE(4, 2) {
        dest[0] = image__compute_y_16(src[0], src[1], src[2]);
        dest[1] = src[3];
      }
      break;
      image__CASE(4, 3) {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
      }
      break;
    default:
      ASSERT(0);
      util_memfree(data);
      util_memfree(good);
      return CAST(ushrt *)stb_set_error("unsupported : Unsupported format conversion");
    }
#undef image__CASE
  }
  util_memfree(data);
  return good;
}
static float *image__ldr_to_hdr(ubyte *data, int x, int y, int comp) {
  int i, k, n;
  float *output;
  if (!data)
    return NULL;
  output = (float *)image__malloc_mad4(x, y, comp, sizeof(float), 0);
  if (output == NULL) {
    util_memfree(data);
    return CAST(float *)stb_set_error("outofmem : Out of memory");
  }
  // compute number of non-alpha components
  n = comp - (comp & 1);
  for (i = 0; i < x * y; ++i)
    for (k = 0; k < n; ++k)
      output[i * comp + k] = (float)(imath_pow(data[i * comp + k] / 255.0f, image__l2h_gamma) * image__l2h_scale);
  if (n < comp)
    for (i = 0; i < x * y; ++i)
      output[i * comp + n] = data[i * comp + n] / 255.0f;
  util_memfree(data);
  return output;
}
static ubyte *image__hdr_to_ldr(float *data, int x, int y, int comp) {
  int i, k, n;
  ubyte *output;
  if (!data)
    return NULL;
  output = (ubyte *)image__malloc_mad3(x, y, comp, 0);
  if (output == NULL) {
    util_memfree(data);
    return (ubyte *)(iter)stb_set_error("outofmem : Out of memory");
  }
  // compute number of non-alpha components
  n = comp - (comp & 1);
  for (i = 0; i < x * y; ++i) {
    for (k = 0; k < n; ++k) {
      float z = (float)imath_pow(data[i * comp + k] * image__h2l_scale_i, image__h2l_gamma_i) * 255 + 0.5f;
      if (z < 0)
        z = 0;
      if (z > 255)
        z = 255;
      output[i * comp + k] = (ubyte)CAST(int)(z);
    }
    if (k < comp) {
      float z = data[i * comp + k] * 255 + 0.5f;
      if (z < 0)
        z = 0;
      if (z > 255)
        z = 255;
      output[i * comp + k] = CAST(ubyte)CAST(int)(z);
    }
  }
  util_memfree(data);
  return output;
}

//////////////////////////////////////////////////////////////////////////////
//
//  "baseline" JPEG/JFIF decoder
//
//    simple implementation
//      - doesn't support delayed output of y-dimension
//      - simple interface (only one output format: 8-bit interleaved RGB)
//      - doesn't try to recover corrupt jpegs
//      - doesn't allow partial loading, loading multiple at once
//      - still fast on x86 (copying globals into locals doesn't help x86)
//      - allocates lots of intermediate memory (full size of all components)
//        - non-interleaved case requires this anyway
//        - allows good upsampling (see next)
//    high-quality
//      - upsampled channels are bilinearly interpolated, even across blocks
//      - quality integer IDCT derived from IJG's 'slow'
//    performance
//      - fast huffman; reasonable integer IDCT
//      - some SIMD kernels for common paths on targets with SSE2/NEON
//      - uses a lot of intermediate memory, could cache poorly

// huffman decoding acceleration
#define FAST_BITS 9 // larger handles more cases; smaller stomps less cache
typedef struct {
  ubyte fast[1 << FAST_BITS];
  // weirdly, repacking this into AoS is a 10% speed loss, instead of a win
  ushrt code[256];
  ubyte values[256];
  ubyte size[257];
  uint maxcode[18];
  int delta[17]; // old 'firstsymbol' - old 'firstcode'
} image__huffman;
typedef struct {
  image__context *s;
  image__huffman huff_dc[4];
  image__huffman huff_ac[4];
  ushrt dequant[4][64];
  shrt fast_ac[4][1 << FAST_BITS];

  // sizes for components, interleaved MCUs
  int img_h_max, img_v_max;
  int img_mcu_x, img_mcu_y;
  int img_mcu_w, img_mcu_h;

  // definition of jpeg image component
  struct
  {
    int id;
    int h, v;
    int tq;
    int hd, ha;
    int dc_pred;

    int x, y, w2, h2;
    ubyte *data;
    void *raw_data, *raw_coeff;
    ubyte *linebuf;
    shrt *coeff;         // progressive only
    int coeff_w, coeff_h; // number of 8x8 coefficient blocks
  } img_comp[4];

  uint32 code_buffer; // jpeg entropy-coded buffer
  int code_bits;        // number of valid bits
  ubyte marker; // marker seen while filling entropy buffer
  int nomore;           // flag if we saw a marker so must stop

  int progressive;
  int spec_start;
  int spec_end;
  int succ_high;
  int succ_low;
  int eob_run;
  int jfif;
  int app14_color_transform; // Adobe APP14 tag
  int rgb;

  int scan_n, order[4];
  int restart_interval, todo;

  // kernels
  void (*idct_block_kernel)(ubyte *out, int out_stride, shrt data[64]);
  void (*YCbCr_to_RGB_kernel)(ubyte *out, const ubyte *y, const ubyte *pcb, const ubyte *pcr, int count, int step);
  ubyte *(*resample_row_hv_2_kernel)(ubyte *out, ubyte *in_near, ubyte *in_far, int w, int hs);
} image__jpeg;
static int image__build_huffman(image__huffman *h, int *count) {
  int i, j, k = 0;
  uint code;
  // build size list for each symbol (from JPEG spec)
  for (i = 0; i < 16; ++i) {
    for (j = 0; j < count[i]; ++j) {
      h->size[k++] = (ubyte)(i + 1);
      if (k >= 257)
        return stb_set_error("bad size list : Corrupt JPEG");
    }
  }
  h->size[k] = 0;

  // compute actual symbols (from jpeg spec)
  code = 0;
  k = 0;
  for (j = 1; j <= 16; ++j) {
    // compute delta to add to code to compute symbol id
    h->delta[j] = k - code;
    if (h->size[k] == j) {
      while (h->size[k] == j)
        h->code[k++] = (ushrt)(code++);
      if (code - 1 >= (1u << j))
        return stb_set_error("bad code lengths : Corrupt JPEG");
    }
    // compute largest code + 1 for this size, preshifted as needed later
    h->maxcode[j] = code << (16 - j);
    code <<= 1;
  }
  h->maxcode[j] = 0xffffffff;

  // build non-spec acceleration table; 255 is flag for not-accelerated
  util_memset(h->fast, 255, 1 << FAST_BITS);
  for (i = 0; i < k; ++i) {
    int s = h->size[i];
    if (s <= FAST_BITS) {
      int c = h->code[i] << (FAST_BITS - s);
      int m = 1 << (FAST_BITS - s);
      for (j = 0; j < m; ++j) {
        h->fast[c + j] = (ubyte)i;
      }
    }
  }
  return 1;
}

// build a table that decodes both magnitude and value of small ACs in
// one go.
static void image__build_fast_ac(shrt *fast_ac, image__huffman *h) {
  int i;
  for (i = 0; i < (1 << FAST_BITS); ++i) {
    ubyte fast = h->fast[i];
    fast_ac[i] = 0;
    if (fast < 255) {
      int rs = h->values[fast];
      int run = (rs >> 4) & 15;
      int magbits = rs & 15;
      int len = h->size[fast];

      if (magbits && len + magbits <= FAST_BITS) {
        // magnitude code followed by receive_extend code
        int k = ((i << len) & ((1 << FAST_BITS) - 1)) >> (FAST_BITS - magbits);
        int m = 1 << (magbits - 1);
        if (k < m)
          k += (~0U << magbits) + 1;
        // if the result is small enough, we can fit it in fast_ac table
        if (k >= -128 && k <= 127)
          fast_ac[i] = (shrt)((k * 256) + (run * 16) + (len + magbits));
      }
    }
  }
}

static void image__grow_buffer_unsafe(image__jpeg *j) {
  do {
    uint b = j->nomore ? 0 : image__get8(j->s);
    if (b == 0xff) {
      int c = image__get8(j->s);
      while (c == 0xff)
        c = image__get8(j->s); // consume fill bytes
      if (c != 0) {
        j->marker = (ubyte)c;
        j->nomore = 1;
        return;
      }
    }
    j->code_buffer |= b << (24 - j->code_bits);
    j->code_bits += 8;
  } while (j->code_bits <= 24);
}

// (1 << n) - 1
static const uint32 image__bmask[17] = {0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535};

// decode a jpeg huffman value from the bitstream
inline static int image__jpeg_huff_decode(image__jpeg *j, image__huffman *h) {
  uint temp;
  int c, k;

  if (j->code_bits < 16)
    image__grow_buffer_unsafe(j);

  // look at the top FAST_BITS and determine what symbol ID it is,
  // if the code is <= FAST_BITS
  c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS) - 1);
  k = h->fast[c];
  if (k < 255) {
    int s = h->size[k];
    if (s > j->code_bits)
      return -1;
    j->code_buffer <<= s;
    j->code_bits -= s;
    return h->values[k];
  }

  // naive test is to shift the code_buffer down so k bits are
  // valid, then test against maxcode. To speed this up, we've
  // preshifted maxcode left so that it has (16-k) 0s at the
  // end; in other words, regardless of the number of bits, it
  // wants to be compared against something shifted to have 16;
  // that way we don't need to shift inside the loop.
  temp = j->code_buffer >> 16;
  for (k = FAST_BITS + 1;; ++k)
    if (temp < h->maxcode[k])
      break;
  if (k == 17) {
    // error! code not found
    j->code_bits -= 16;
    return -1;
  }

  if (k > j->code_bits)
    return -1;

  // convert the huffman code to the symbol id
  c = ((j->code_buffer >> (32 - k)) & image__bmask[k]) + h->delta[k];
  if (c < 0 || c >= 256) // symbol id out of bounds!
    return -1;
  ASSERT((((j->code_buffer) >> (32 - h->size[c])) & image__bmask[h->size[c]]) == h->code[c]);

  // convert the id to a symbol
  j->code_bits -= k;
  j->code_buffer <<= k;
  return h->values[c];
}
// bias[n] = (-1<<n) + 1
static const int image__jbias[16] = {0, -1, -3, -7, -15, -31, -63, -127, -255, -511, -1023, -2047, -4095, -8191, -16383, -32767};
// combined JPEG 'receive' and JPEG 'extend', since baseline
// always extends everything it receives.
inline static int image__extend_receive(image__jpeg *j, int n) {
  uint k;
  int sgn;
  if (j->code_bits < n)
    image__grow_buffer_unsafe(j);
  if (j->code_bits < n)
    return 0; // ran out of bits from stream, return 0s intead of continuing

  sgn = j->code_buffer >> 31; // sign bit always in MSB; 0 if MSB clear (positive), 1 if MSB set (negative)
  k = imath_rotl32(j->code_buffer, n);
  j->code_buffer = k & ~image__bmask[n];
  k &= image__bmask[n];
  j->code_bits -= n;
  return k + (image__jbias[n] & (sgn - 1));
}
// get some unsigned bits
inline static int image__jpeg_get_bits(image__jpeg *j, int n) {
  uint k;
  if (j->code_bits < n)
    image__grow_buffer_unsafe(j);
  if (j->code_bits < n)
    return 0; // ran out of bits from stream, return 0s intead of continuing
  k = imath_rotl32(j->code_buffer, n);
  j->code_buffer = k & ~image__bmask[n];
  k &= image__bmask[n];
  j->code_bits -= n;
  return k;
}
inline static int image__jpeg_get_bit(image__jpeg *j) {
  uint k;
  if (j->code_bits < 1)
    image__grow_buffer_unsafe(j);
  if (j->code_bits < 1)
    return 0; // ran out of bits from stream, return 0s intead of continuing
  k = j->code_buffer;
  j->code_buffer <<= 1;
  --j->code_bits;
  return k & 0x80000000;
}
// given a value that's at position X in the zigzag stream,
// where does it appear in the 8x8 matrix coded as row-major?
static const ubyte image__jpeg_dezigzag[64 + 15] = {
    0, 1, 8, 16, 9, 2, 3, 10,
    17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63,
    // let corrupt input sample past end
    63, 63, 63, 63, 63, 63, 63, 63,
    63, 63, 63, 63, 63, 63, 63};

// decode one 64-entry block--
static int image__jpeg_decode_block(image__jpeg *j, shrt data[64], image__huffman *hdc, image__huffman *hac, shrt *fac, int b, ushrt *dequant) {
  int diff, dc, k;
  int t;

  if (j->code_bits < 16)
    image__grow_buffer_unsafe(j);
  t = image__jpeg_huff_decode(j, hdc);
  if (t < 0 || t > 15)
    return stb_set_error("bad huffman code : Corrupt JPEG");

  // 0 all the ac values now so we can do it 32-bits at a time
  util_memset(data, 0, 64 * sizeof(data[0]));

  diff = t ? image__extend_receive(j, t) : 0;
  if (!image__addints_valid(j->img_comp[b].dc_pred, diff))
    return stb_set_error("bad delta : Corrupt JPEG");
  dc = j->img_comp[b].dc_pred + diff;
  j->img_comp[b].dc_pred = dc;
  if (!image__mul2shrts_valid(dc, dequant[0]))
    return stb_set_error("can't merge dc and ac : Corrupt JPEG");
  data[0] = (shrt)(dc * dequant[0]);

  // decode AC components, see JPEG spec
  k = 1;
  do {
    uint zig;
    int c, r, s;
    if (j->code_bits < 16)
      image__grow_buffer_unsafe(j);
    c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS) - 1);
    r = fac[c];
    if (r) {              // fast-AC path
      k += (r >> 4) & 15; // run
      s = r & 15;         // combined length
      if (s > j->code_bits)
        return stb_set_error("bad huffman code : Combined length longer than code bits available");
      j->code_buffer <<= s;
      j->code_bits -= s;
      // decode into unzigzag'd location
      zig = image__jpeg_dezigzag[k++];
      data[zig] = (shrt)((r >> 8) * dequant[zig]);
    } else {
      int rs = image__jpeg_huff_decode(j, hac);
      if (rs < 0)
        return stb_set_error("bad huffman code : Corrupt JPEG");
      s = rs & 15;
      r = rs >> 4;
      if (s == 0) {
        if (rs != 0xf0)
          break; // end block
        k += 16;
      } else {
        k += r;
        // decode into unzigzag'd location
        zig = image__jpeg_dezigzag[k++];
        data[zig] = (shrt)(image__extend_receive(j, s) * dequant[zig]);
      }
    }
  } while (k < 64);
  return 1;
}

static int image__jpeg_decode_block_prog_dc(image__jpeg *j, shrt data[64], image__huffman *hdc, int b) {
  int diff, dc;
  int t;
  if (j->spec_end != 0)
    return stb_set_error("can't merge dc and ac : Corrupt JPEG");

  if (j->code_bits < 16)
    image__grow_buffer_unsafe(j);

  if (j->succ_high == 0) {
    // first scan for DC coefficient, must be first
    util_memset(data, 0, 64 * sizeof(data[0])); // 0 all the ac values now
    t = image__jpeg_huff_decode(j, hdc);
    if (t < 0 || t > 15)
      return stb_set_error("can't merge dc and ac : Corrupt JPEG");
    diff = t ? image__extend_receive(j, t) : 0;

    if (!image__addints_valid(j->img_comp[b].dc_pred, diff))
      return stb_set_error("bad delta : Corrupt JPEG");
    dc = j->img_comp[b].dc_pred + diff;
    j->img_comp[b].dc_pred = dc;
    if (!image__mul2shrts_valid(dc, 1 << j->succ_low))
      return stb_set_error("can't merge dc and ac : Corrupt JPEG");
    data[0] = (shrt)(dc * (1 << j->succ_low));
  } else {
    // refinement scan for DC coefficient
    if (image__jpeg_get_bit(j))
      data[0] += (shrt)(1 << j->succ_low);
  }
  return 1;
}

// @OPTIMIZE: store non-zigzagged during the decode passes,
// and only de-zigzag when dequantizing
static bool image__jpeg_decode_block_prog_ac(image__jpeg *j, shrt data[64], image__huffman *hac, shrt *fac) {
  int k;
  if (j->spec_start == 0) {
    stb_set_error("can't merge dc and ac : Corrupt JPEG");
    return false;
  }

  if (j->succ_high == 0) {
    int shift = j->succ_low;

    if (j->eob_run) {
      --j->eob_run;
      return true;
    }

    k = j->spec_start;
    do {
      uint zig;
      int c, r, s;
      if (j->code_bits < 16)
        image__grow_buffer_unsafe(j);
      c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS) - 1);
      r = fac[c];
      if (r) {              // fast-AC path
        k += (r >> 4) & 15; // run
        s = r & 15;         // combined length
        if (s > j->code_bits)
          return stb_set_error("bad huffman code : Combined length longer than code bits available");
        j->code_buffer <<= s;
        j->code_bits -= s;
        zig = image__jpeg_dezigzag[k++];
        data[zig] = (shrt)((r >> 8) * (1 << shift));
      } else {
        int rs = image__jpeg_huff_decode(j, hac);
        if (rs < 0)
          return stb_set_error("bad huffman code : Corrupt JPEG");
        s = rs & 15;
        r = rs >> 4;
        if (s == 0) {
          if (r < 15) {
            j->eob_run = (1 << r);
            if (r)
              j->eob_run += image__jpeg_get_bits(j, r);
            --j->eob_run;
            break;
          }
          k += 16;
        } else {
          k += r;
          zig = image__jpeg_dezigzag[k++];
          data[zig] = (shrt)(image__extend_receive(j, s) * (1 << shift));
        }
      }
    } while (k <= j->spec_end);
  } else {
    // refinement scan for these AC coefficients

    shrt bit = (shrt)(1 << j->succ_low);

    if (j->eob_run) {
      --j->eob_run;
      for (k = j->spec_start; k <= j->spec_end; ++k) {
        shrt *p = &data[image__jpeg_dezigzag[k]];
        if (*p != 0)
          if (image__jpeg_get_bit(j))
            if ((*p & bit) == 0) {
              if (*p > 0)
                *p += bit;
              else
                *p -= bit;
            }
      }
    } else {
      k = j->spec_start;
      do {
        int r, s;
        int rs = image__jpeg_huff_decode(j, hac); // @OPTIMIZE see if we can use the fast path here, advance-by-r is so slow, eh
        if (rs < 0) {
          stb_set_error("bad huffman code : Corrupt JPEG");
          return false;
        }
        s = rs & 15;
        r = rs >> 4;
        if (s == 0) {
          if (r < 15) {
            j->eob_run = (1 << r) - 1;
            if (r)
              j->eob_run += image__jpeg_get_bits(j, r);
            r = 64; // force end of block
          } else {
            // r=15 s=0 should write 16 0s, so we just do
            // a run of 15 0s and then write s (which is 0),
            // so we don't have to do anything special here
          }
        } else {
          if (s != 1) {
            stb_set_error("bad huffman code : Corrupt JPEG");
            return false;
          }
          // sign bit
          if (image__jpeg_get_bit(j))
            s = bit;
          else
            s = -bit;
        }

        // advance by r
        while (k <= j->spec_end) {
          shrt *p = &data[image__jpeg_dezigzag[k++]];
          if (*p != 0) {
            if (image__jpeg_get_bit(j))
              if ((*p & bit) == 0) {
                if (*p > 0)
                  *p += bit;
                else
                  *p -= bit;
              }
          } else {
            if (r == 0) {
              *p = (shrt)s;
              break;
            }
            --r;
          }
        }
      } while (k <= j->spec_end);
    }
  }
  return true;
}

// take a -128..127 value and image__clamp it and convert to 0..255
inline static ubyte image__clamp(int x) {
  // trick to use a single test to catch both cases
  if ((uint)x > 255) {
    if (x < 0)
      return 0;
    if (x > 255)
      return 255;
  }
  return (ubyte)x;
}

#define image__f2f(x) ((int)(((x) * 4096 + 0.5)))
#define image__fsh(x) ((x) * 4096)

// derived from jidctint -- DCT_ISLOW
#define image__IDCT_1D(s0, s1, s2, s3, s4, s5, s6, s7)     \
  int t0, t1, t2, t3, p1, p2, p3, p4, p5, x0, x1, x2, x3; \
  p2 = s2;                                                \
  p3 = s6;                                                \
  p1 = (p2 + p3) * image__f2f(0.5411961f);                 \
  t2 = p1 + p3 * image__f2f(-1.847759065f);                \
  t3 = p1 + p2 * image__f2f(0.765366865f);                 \
  p2 = s0;                                                \
  p3 = s4;                                                \
  t0 = image__fsh(p2 + p3);                                \
  t1 = image__fsh(p2 - p3);                                \
  x0 = t0 + t3;                                           \
  x3 = t0 - t3;                                           \
  x1 = t1 + t2;                                           \
  x2 = t1 - t2;                                           \
  t0 = s7;                                                \
  t1 = s5;                                                \
  t2 = s3;                                                \
  t3 = s1;                                                \
  p3 = t0 + t2;                                           \
  p4 = t1 + t3;                                           \
  p1 = t0 + t3;                                           \
  p2 = t1 + t2;                                           \
  p5 = (p3 + p4) * image__f2f(1.175875602f);               \
  t0 = t0 * image__f2f(0.298631336f);                      \
  t1 = t1 * image__f2f(2.053119869f);                      \
  t2 = t2 * image__f2f(3.072711026f);                      \
  t3 = t3 * image__f2f(1.501321110f);                      \
  p1 = p5 + p1 * image__f2f(-0.899976223f);                \
  p2 = p5 + p2 * image__f2f(-2.562915447f);                \
  p3 = p3 * image__f2f(-1.961570560f);                     \
  p4 = p4 * image__f2f(-0.390180644f);                     \
  t3 += p1 + p4;                                          \
  t2 += p2 + p3;                                          \
  t1 += p2 + p4;                                          \
  t0 += p1 + p3;

static void image__idct_block(ubyte *out, int out_stride, shrt data[64]) {
  int i, val[64], *v = val;
  ubyte *o;
  shrt *d = data;

  // columns
  for (i = 0; i < 8; ++i, ++d, ++v) {
    // if all zeroes, shrtcut -- this avoids dequantizing 0s and IDCTing
    if (d[8] == 0 && d[16] == 0 && d[24] == 0 && d[32] == 0 && d[40] == 0 && d[48] == 0 && d[56] == 0) {
      //    no shrtcut                 0     seconds
      //    (1|2|3|4|5|6|7)==0          0     seconds
      //    all separate               -0.047 seconds
      //    1 && 2|3 && 4|5 && 6|7:    -0.047 seconds
      int dcterm = d[0] * 4;
      v[0] = v[8] = v[16] = v[24] = v[32] = v[40] = v[48] = v[56] = dcterm;
    } else {
      image__IDCT_1D(d[0], d[8], d[16], d[24], d[32], d[40], d[48], d[56])
      // constants scaled things up by 1<<12; let's bring them back
      // down, but keep 2 extra bits of precision
      x0 += 512;
      x1 += 512;
      x2 += 512;
      x3 += 512;
      v[0] = (x0 + t3) >> 10;
      v[56] = (x0 - t3) >> 10;
      v[8] = (x1 + t2) >> 10;
      v[48] = (x1 - t2) >> 10;
      v[16] = (x2 + t1) >> 10;
      v[40] = (x2 - t1) >> 10;
      v[24] = (x3 + t0) >> 10;
      v[32] = (x3 - t0) >> 10;
    }
  }

  for (i = 0, v = val, o = out; i < 8; ++i, v += 8, o += out_stride) {
    // no fast case since the first 1D IDCT spread components out
    image__IDCT_1D(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7])
    // constants scaled things up by 1<<12, plus we had 1<<2 from first
    // loop, plus horizontal and vertical each scale by sqrt(8) so together
    // we've got an extra 1<<3, so 1<<17 total we need to remove.
    // so we want to round that, which means adding 0.5 * 1<<17,
    // aka 65536. Also, we'll end up with -128 to 127 that we want
    // to encode as 0..255 by adding 128, so we'll add that before the shift
    x0 += 65536 + (128 << 17);
    x1 += 65536 + (128 << 17);
    x2 += 65536 + (128 << 17);
    x3 += 65536 + (128 << 17);
    // tried computing the shifts into temps, or'ing the temps to see
    // if any were out of range, but that was slower
    o[0] = image__clamp((x0 + t3) >> 17);
    o[7] = image__clamp((x0 - t3) >> 17);
    o[1] = image__clamp((x1 + t2) >> 17);
    o[6] = image__clamp((x1 - t2) >> 17);
    o[2] = image__clamp((x2 + t1) >> 17);
    o[5] = image__clamp((x2 - t1) >> 17);
    o[3] = image__clamp((x3 + t0) >> 17);
    o[4] = image__clamp((x3 - t0) >> 17);
  }
}

#ifdef STBI_SSE2
// sse2 integer IDCT. not the fastest possible implementation but it
// produces bit-identical results to the generic C version so it's
// fully "transparent".
static void image__idct_simd(ubyte *out, int out_stride, shrt data[64]) {
  // This is constructed to match our regular (generic) integer IDCT exactly.
  __m128i row0, row1, row2, row3, row4, row5, row6, row7;
  __m128i tmp;

// dot product constant: even elems=x, odd elems=y
#define dct_const(x, y) _mm_setr_epi16((x), (y), (x), (y), (x), (y), (x), (y))

// out(0) = c0[even]*x + c0[odd]*y   (c0, x, y 16-bit, out 32-bit)
// out(1) = c1[even]*x + c1[odd]*y
#define dct_rot(out0, out1, x, y, c0, c1)        \
  __m128i c0##lo = _mm_unpacklo_epi16((x), (y)); \
  __m128i c0##hi = _mm_unpackhi_epi16((x), (y)); \
  __m128i out0##_l = _mm_madd_epi16(c0##lo, c0); \
  __m128i out0##_h = _mm_madd_epi16(c0##hi, c0); \
  __m128i out1##_l = _mm_madd_epi16(c0##lo, c1); \
  __m128i out1##_h = _mm_madd_epi16(c0##hi, c1)

// out = in << 12  (in 16-bit, out 32-bit)
#define dct_widen(out, in)                                                            \
  __m128i out##_l = _mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), (in)), 4); \
  __m128i out##_h = _mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), (in)), 4)

// wide add
#define dct_wadd(out, a, b)                      \
  __m128i out##_l = _mm_add_epi32(a##_l, b##_l); \
  __m128i out##_h = _mm_add_epi32(a##_h, b##_h)

// wide sub
#define dct_wsub(out, a, b)                      \
  __m128i out##_l = _mm_sub_epi32(a##_l, b##_l); \
  __m128i out##_h = _mm_sub_epi32(a##_h, b##_h)

// butterfly a/b, add bias, then shift by "s" and pack
#define dct_bfly32o(out0, out1, a, b, bias, s)                                  \
  {                                                                             \
    __m128i abiased_l = _mm_add_epi32(a##_l, bias);                             \
    __m128i abiased_h = _mm_add_epi32(a##_h, bias);                             \
    dct_wadd(sum, abiased, b);                                                  \
    dct_wsub(dif, abiased, b);                                                  \
    out0 = _mm_packs_epi32(_mm_srai_epi32(sum_l, s), _mm_srai_epi32(sum_h, s)); \
    out1 = _mm_packs_epi32(_mm_srai_epi32(dif_l, s), _mm_srai_epi32(dif_h, s)); \
  }

// 8-bit interleave step (for transposes)
#define dct_interleave8(a, b)  \
  tmp = a;                     \
  a = _mm_unpacklo_epi8(a, b); \
  b = _mm_unpackhi_epi8(tmp, b)

// 16-bit interleave step (for transposes)
#define dct_interleave16(a, b)  \
  tmp = a;                      \
  a = _mm_unpacklo_epi16(a, b); \
  b = _mm_unpackhi_epi16(tmp, b)

#define dct_pass(bias, shift)                        \
  {                                                  \
    /* even part */                                  \
    dct_rot(t2e, t3e, row2, row6, rot0_0, rot0_1);   \
    __m128i sum04 = _mm_add_epi16(row0, row4);       \
    __m128i dif04 = _mm_sub_epi16(row0, row4);       \
    dct_widen(t0e, sum04);                           \
    dct_widen(t1e, dif04);                           \
    dct_wadd(x0, t0e, t3e);                          \
    dct_wsub(x3, t0e, t3e);                          \
    dct_wadd(x1, t1e, t2e);                          \
    dct_wsub(x2, t1e, t2e);                          \
    /* odd part */                                   \
    dct_rot(y0o, y2o, row7, row3, rot2_0, rot2_1);   \
    dct_rot(y1o, y3o, row5, row1, rot3_0, rot3_1);   \
    __m128i sum17 = _mm_add_epi16(row1, row7);       \
    __m128i sum35 = _mm_add_epi16(row3, row5);       \
    dct_rot(y4o, y5o, sum17, sum35, rot1_0, rot1_1); \
    dct_wadd(x4, y0o, y4o);                          \
    dct_wadd(x5, y1o, y5o);                          \
    dct_wadd(x6, y2o, y5o);                          \
    dct_wadd(x7, y3o, y4o);                          \
    dct_bfly32o(row0, row7, x0, x7, bias, shift);    \
    dct_bfly32o(row1, row6, x1, x6, bias, shift);    \
    dct_bfly32o(row2, row5, x2, x5, bias, shift);    \
    dct_bfly32o(row3, row4, x3, x4, bias, shift);    \
  }

  __m128i rot0_0 = dct_const(image__f2f(0.5411961f), image__f2f(0.5411961f) + image__f2f(-1.847759065f));
  __m128i rot0_1 = dct_const(image__f2f(0.5411961f) + image__f2f(0.765366865f), image__f2f(0.5411961f));
  __m128i rot1_0 = dct_const(image__f2f(1.175875602f) + image__f2f(-0.899976223f), image__f2f(1.175875602f));
  __m128i rot1_1 = dct_const(image__f2f(1.175875602f), image__f2f(1.175875602f) + image__f2f(-2.562915447f));
  __m128i rot2_0 = dct_const(image__f2f(-1.961570560f) + image__f2f(0.298631336f), image__f2f(-1.961570560f));
  __m128i rot2_1 = dct_const(image__f2f(-1.961570560f), image__f2f(-1.961570560f) + image__f2f(3.072711026f));
  __m128i rot3_0 = dct_const(image__f2f(-0.390180644f) + image__f2f(2.053119869f), image__f2f(-0.390180644f));
  __m128i rot3_1 = dct_const(image__f2f(-0.390180644f), image__f2f(-0.390180644f) + image__f2f(1.501321110f));

  // rounding biases in column/row passes, see image__idct_block for explanation.
  __m128i bias_0 = _mm_set1_epi32(512);
  __m128i bias_1 = _mm_set1_epi32(65536 + (128 << 17));

  // load
  row0 = _mm_load_si128((const __m128i *)(data + 0 * 8));
  row1 = _mm_load_si128((const __m128i *)(data + 1 * 8));
  row2 = _mm_load_si128((const __m128i *)(data + 2 * 8));
  row3 = _mm_load_si128((const __m128i *)(data + 3 * 8));
  row4 = _mm_load_si128((const __m128i *)(data + 4 * 8));
  row5 = _mm_load_si128((const __m128i *)(data + 5 * 8));
  row6 = _mm_load_si128((const __m128i *)(data + 6 * 8));
  row7 = _mm_load_si128((const __m128i *)(data + 7 * 8));

  // column pass
  dct_pass(bias_0, 10);

  {
    // 16bit 8x8 transpose pass 1
    dct_interleave16(row0, row4);
    dct_interleave16(row1, row5);
    dct_interleave16(row2, row6);
    dct_interleave16(row3, row7);

    // transpose pass 2
    dct_interleave16(row0, row2);
    dct_interleave16(row1, row3);
    dct_interleave16(row4, row6);
    dct_interleave16(row5, row7);

    // transpose pass 3
    dct_interleave16(row0, row1);
    dct_interleave16(row2, row3);
    dct_interleave16(row4, row5);
    dct_interleave16(row6, row7);
  }

  // row pass
  dct_pass(bias_1, 17);

  {
    // pack
    __m128i p0 = _mm_packus_epi16(row0, row1); // a0a1a2a3...a7b0b1b2b3...b7
    __m128i p1 = _mm_packus_epi16(row2, row3);
    __m128i p2 = _mm_packus_epi16(row4, row5);
    __m128i p3 = _mm_packus_epi16(row6, row7);

    // 8bit 8x8 transpose pass 1
    dct_interleave8(p0, p2); // a0e0a1e1...
    dct_interleave8(p1, p3); // c0g0c1g1...

    // transpose pass 2
    dct_interleave8(p0, p1); // a0c0e0g0...
    dct_interleave8(p2, p3); // b0d0f0h0...

    // transpose pass 3
    dct_interleave8(p0, p2); // a0b0c0d0...
    dct_interleave8(p1, p3); // a4b4c4d4...

    // store
    _mm_storel_epi64((__m128i *)out, p0);
    out += out_stride;
    _mm_storel_epi64((__m128i *)out, _mm_shuffle_epi32(p0, 0x4e));
    out += out_stride;
    _mm_storel_epi64((__m128i *)out, p2);
    out += out_stride;
    _mm_storel_epi64((__m128i *)out, _mm_shuffle_epi32(p2, 0x4e));
    out += out_stride;
    _mm_storel_epi64((__m128i *)out, p1);
    out += out_stride;
    _mm_storel_epi64((__m128i *)out, _mm_shuffle_epi32(p1, 0x4e));
    out += out_stride;
    _mm_storel_epi64((__m128i *)out, p3);
    out += out_stride;
    _mm_storel_epi64((__m128i *)out, _mm_shuffle_epi32(p3, 0x4e));
  }

#undef dct_const
#undef dct_rot
#undef dct_widen
#undef dct_wadd
#undef dct_wsub
#undef dct_bfly32o
#undef dct_interleave8
#undef dct_interleave16
#undef dct_pass
}

#endif // STBI_SSE2

#ifdef STBI_NEON

// NEON integer IDCT. should produce bit-identical
// results to the generic C version.
static void image__idct_simd(ubyte *out, int out_stride, shrt data[64]) {
  int16x8_t row0, row1, row2, row3, row4, row5, row6, row7;

  int16x4_t rot0_0 = vdup_n_s16(image__f2f(0.5411961f));
  int16x4_t rot0_1 = vdup_n_s16(image__f2f(-1.847759065f));
  int16x4_t rot0_2 = vdup_n_s16(image__f2f(0.765366865f));
  int16x4_t rot1_0 = vdup_n_s16(image__f2f(1.175875602f));
  int16x4_t rot1_1 = vdup_n_s16(image__f2f(-0.899976223f));
  int16x4_t rot1_2 = vdup_n_s16(image__f2f(-2.562915447f));
  int16x4_t rot2_0 = vdup_n_s16(image__f2f(-1.961570560f));
  int16x4_t rot2_1 = vdup_n_s16(image__f2f(-0.390180644f));
  int16x4_t rot3_0 = vdup_n_s16(image__f2f(0.298631336f));
  int16x4_t rot3_1 = vdup_n_s16(image__f2f(2.053119869f));
  int16x4_t rot3_2 = vdup_n_s16(image__f2f(3.072711026f));
  int16x4_t rot3_3 = vdup_n_s16(image__f2f(1.501321110f));

#define dct_long_mul(out, inq, coeff)                      \
  int32x4_t out##_l = vmull_s16(vget_low_s16(inq), coeff); \
  int32x4_t out##_h = vmull_s16(vget_high_s16(inq), coeff)

#define dct_long_mac(out, acc, inq, coeff)                          \
  int32x4_t out##_l = vmlal_s16(acc##_l, vget_low_s16(inq), coeff); \
  int32x4_t out##_h = vmlal_s16(acc##_h, vget_high_s16(inq), coeff)

#define dct_widen(out, inq)                               \
  int32x4_t out##_l = vshll_n_s16(vget_low_s16(inq), 12); \
  int32x4_t out##_h = vshll_n_s16(vget_high_s16(inq), 12)

// wide add
#define dct_wadd(out, a, b)                    \
  int32x4_t out##_l = vaddq_s32(a##_l, b##_l); \
  int32x4_t out##_h = vaddq_s32(a##_h, b##_h)

// wide sub
#define dct_wsub(out, a, b)                    \
  int32x4_t out##_l = vsubq_s32(a##_l, b##_l); \
  int32x4_t out##_h = vsubq_s32(a##_h, b##_h)

// butterfly a/b, then shift using "shiftop" by "s" and pack
#define dct_bfly32o(out0, out1, a, b, shiftop, s)              \
  {                                                            \
    dct_wadd(sum, a, b);                                       \
    dct_wsub(dif, a, b);                                       \
    out0 = vcombine_s16(shiftop(sum_l, s), shiftop(sum_h, s)); \
    out1 = vcombine_s16(shiftop(dif_l, s), shiftop(dif_h, s)); \
  }

#define dct_pass(shiftop, shift)                     \
  {                                                  \
    /* even part */                                  \
    int16x8_t sum26 = vaddq_s16(row2, row6);         \
    dct_long_mul(p1e, sum26, rot0_0);                \
    dct_long_mac(t2e, p1e, row6, rot0_1);            \
    dct_long_mac(t3e, p1e, row2, rot0_2);            \
    int16x8_t sum04 = vaddq_s16(row0, row4);         \
    int16x8_t dif04 = vsubq_s16(row0, row4);         \
    dct_widen(t0e, sum04);                           \
    dct_widen(t1e, dif04);                           \
    dct_wadd(x0, t0e, t3e);                          \
    dct_wsub(x3, t0e, t3e);                          \
    dct_wadd(x1, t1e, t2e);                          \
    dct_wsub(x2, t1e, t2e);                          \
    /* odd part */                                   \
    int16x8_t sum15 = vaddq_s16(row1, row5);         \
    int16x8_t sum17 = vaddq_s16(row1, row7);         \
    int16x8_t sum35 = vaddq_s16(row3, row5);         \
    int16x8_t sum37 = vaddq_s16(row3, row7);         \
    int16x8_t sumodd = vaddq_s16(sum17, sum35);      \
    dct_long_mul(p5o, sumodd, rot1_0);               \
    dct_long_mac(p1o, p5o, sum17, rot1_1);           \
    dct_long_mac(p2o, p5o, sum35, rot1_2);           \
    dct_long_mul(p3o, sum37, rot2_0);                \
    dct_long_mul(p4o, sum15, rot2_1);                \
    dct_wadd(sump13o, p1o, p3o);                     \
    dct_wadd(sump24o, p2o, p4o);                     \
    dct_wadd(sump23o, p2o, p3o);                     \
    dct_wadd(sump14o, p1o, p4o);                     \
    dct_long_mac(x4, sump13o, row7, rot3_0);         \
    dct_long_mac(x5, sump24o, row5, rot3_1);         \
    dct_long_mac(x6, sump23o, row3, rot3_2);         \
    dct_long_mac(x7, sump14o, row1, rot3_3);         \
    dct_bfly32o(row0, row7, x0, x7, shiftop, shift); \
    dct_bfly32o(row1, row6, x1, x6, shiftop, shift); \
    dct_bfly32o(row2, row5, x2, x5, shiftop, shift); \
    dct_bfly32o(row3, row4, x3, x4, shiftop, shift); \
  }

  // load
  row0 = vld1q_s16(data + 0 * 8);
  row1 = vld1q_s16(data + 1 * 8);
  row2 = vld1q_s16(data + 2 * 8);
  row3 = vld1q_s16(data + 3 * 8);
  row4 = vld1q_s16(data + 4 * 8);
  row5 = vld1q_s16(data + 5 * 8);
  row6 = vld1q_s16(data + 6 * 8);
  row7 = vld1q_s16(data + 7 * 8);

  // add DC bias
  row0 = vaddq_s16(row0, vsetq_lane_s16(1024, vdupq_n_s16(0), 0));

  // column pass
  dct_pass(vrshrn_n_s32, 10);

  // 16bit 8x8 transpose
  {
// these three map to a single VTRN.16, VTRN.32, and VSWP, respectively.
// whether compilers actually get this is another story, sadly.
#define dct_trn16(x, y)              \
  {                                  \
    int16x8x2_t t = vtrnq_s16(x, y); \
    x = t.val[0];                    \
    y = t.val[1];                    \
  }
#define dct_trn32(x, y)                                                            \
  {                                                                                \
    int32x4x2_t t = vtrnq_s32(vreinterpretq_s32_s16(x), vreinterpretq_s32_s16(y)); \
    x = vreinterpretq_s16_s32(t.val[0]);                                           \
    y = vreinterpretq_s16_s32(t.val[1]);                                           \
  }
#define dct_trn64(x, y)                                     \
  {                                                         \
    int16x8_t x0 = x;                                       \
    int16x8_t y0 = y;                                       \
    x = vcombine_s16(vget_low_s16(x0), vget_low_s16(y0));   \
    y = vcombine_s16(vget_high_s16(x0), vget_high_s16(y0)); \
  }

    // pass 1
    dct_trn16(row0, row1); // a0b0a2b2a4b4a6b6
    dct_trn16(row2, row3);
    dct_trn16(row4, row5);
    dct_trn16(row6, row7);

    // pass 2
    dct_trn32(row0, row2); // a0b0c0d0a4b4c4d4
    dct_trn32(row1, row3);
    dct_trn32(row4, row6);
    dct_trn32(row5, row7);

    // pass 3
    dct_trn64(row0, row4); // a0b0c0d0e0f0g0h0
    dct_trn64(row1, row5);
    dct_trn64(row2, row6);
    dct_trn64(row3, row7);

#undef dct_trn16
#undef dct_trn32
#undef dct_trn64
  }

  // row pass
  // vrshrn_n_s32 only supports shifts up to 16, we need
  // 17. so do a non-rounding shift of 16 first then follow
  // up with a rounding shift by 1.
  dct_pass(vshrn_n_s32, 16);

  {
    // pack and round
    uint8x8_t p0 = vqrshrun_n_s16(row0, 1);
    uint8x8_t p1 = vqrshrun_n_s16(row1, 1);
    uint8x8_t p2 = vqrshrun_n_s16(row2, 1);
    uint8x8_t p3 = vqrshrun_n_s16(row3, 1);
    uint8x8_t p4 = vqrshrun_n_s16(row4, 1);
    uint8x8_t p5 = vqrshrun_n_s16(row5, 1);
    uint8x8_t p6 = vqrshrun_n_s16(row6, 1);
    uint8x8_t p7 = vqrshrun_n_s16(row7, 1);

    // again, these can translate into one instruction, but often don't.
#define dct_trn8_8(x, y)           \
  {                                \
    uint8x8x2_t t = vtrn_u8(x, y); \
    x = t.val[0];                  \
    y = t.val[1];                  \
  }
#define dct_trn8_16(x, y)                                                      \
  {                                                                            \
    uint16x4x2_t t = vtrn_u16(vreinterpret_u16_u8(x), vreinterpret_u16_u8(y)); \
    x = vreinterpret_u8_u16(t.val[0]);                                         \
    y = vreinterpret_u8_u16(t.val[1]);                                         \
  }
#define dct_trn8_32(x, y)                                                      \
  {                                                                            \
    uint32x2x2_t t = vtrn_u32(vreinterpret_u32_u8(x), vreinterpret_u32_u8(y)); \
    x = vreinterpret_u8_u32(t.val[0]);                                         \
    y = vreinterpret_u8_u32(t.val[1]);                                         \
  }

    // sadly can't use interleaved stores here since we only write
    // 8 bytes to each scan line!

    // 8x8 8-bit transpose pass 1
    dct_trn8_8(p0, p1);
    dct_trn8_8(p2, p3);
    dct_trn8_8(p4, p5);
    dct_trn8_8(p6, p7);

    // pass 2
    dct_trn8_16(p0, p2);
    dct_trn8_16(p1, p3);
    dct_trn8_16(p4, p6);
    dct_trn8_16(p5, p7);

    // pass 3
    dct_trn8_32(p0, p4);
    dct_trn8_32(p1, p5);
    dct_trn8_32(p2, p6);
    dct_trn8_32(p3, p7);

    // store
    vst1_u8(out, p0);
    out += out_stride;
    vst1_u8(out, p1);
    out += out_stride;
    vst1_u8(out, p2);
    out += out_stride;
    vst1_u8(out, p3);
    out += out_stride;
    vst1_u8(out, p4);
    out += out_stride;
    vst1_u8(out, p5);
    out += out_stride;
    vst1_u8(out, p6);
    out += out_stride;
    vst1_u8(out, p7);

#undef dct_trn8_8
#undef dct_trn8_16
#undef dct_trn8_32
  }

#undef dct_long_mul
#undef dct_long_mac
#undef dct_widen
#undef dct_wadd
#undef dct_wsub
#undef dct_bfly32o
#undef dct_pass
}

#endif // STBI_NEON
#define image__MARKER_none 0xff
// if there's a pending marker from the entropy stream, return that
// otherwise, fetch from the stream and get a marker. if there's no
// marker, return 0xff, which is never a valid marker value
static ubyte image__get_marker(image__jpeg *j) {
  ubyte x;
  if (j->marker != image__MARKER_none) {
    x = j->marker;
    j->marker = image__MARKER_none;
    return x;
  }
  x = image__get8(j->s);
  if (x != 0xff)
    return image__MARKER_none;
  while (x == 0xff)
    x = image__get8(j->s); // consume repeated 0xff fill bytes
  return x;
}
// in each scan, we'll have scan_n components, and the order
// of the components is specified by order[]
#define image__RESTART(x) ((x) >= 0xd0 && (x) <= 0xd7)
// after a restart interval, image__jpeg_reset the entropy decoder and
// the dc prediction
static void image__jpeg_reset(image__jpeg *j) {
  j->code_bits = 0;
  j->code_buffer = 0;
  j->nomore = 0;
  j->img_comp[0].dc_pred = j->img_comp[1].dc_pred = j->img_comp[2].dc_pred = j->img_comp[3].dc_pred = 0;
  j->marker = image__MARKER_none;
  j->todo = j->restart_interval ? j->restart_interval : 0x7fffffff;
  j->eob_run = 0;
  // no more than 1<<31 MCUs if no restart_interal? that's plenty safe,
  // since we don't even allow 1<<30 pixels
}
static int image__parse_entropy_coded_data(image__jpeg *z) {
  image__jpeg_reset(z);
  if (!z->progressive) {
    if (z->scan_n == 1) {
      int i, j;
      STBI_SIMD_ALIGN(shrt, data[64]);
      int n = z->order[0];
      // non-interleaved data, we just need to process one block at a time,
      // in trivial scanline order
      // number of blocks to do just depends on how many actual "pixels" this
      // component has, independent of interleaved MCU blocking and such
      int w = (z->img_comp[n].x + 7) >> 3;
      int h = (z->img_comp[n].y + 7) >> 3;
      for (j = 0; j < h; ++j) {
        for (i = 0; i < w; ++i) {
          int ha = z->img_comp[n].ha;
          if (!image__jpeg_decode_block(z, data, z->huff_dc + z->img_comp[n].hd, z->huff_ac + ha, z->fast_ac[ha], n, z->dequant[z->img_comp[n].tq]))
            return 0;
          z->idct_block_kernel(z->img_comp[n].data + z->img_comp[n].w2 * j * 8 + i * 8, z->img_comp[n].w2, data);
          // every data block is an MCU, so countdown the restart interval
          if (--z->todo <= 0) {
            if (z->code_bits < 24)
              image__grow_buffer_unsafe(z);
            // if it's NOT a restart, then just bail, so we get corrupt data
            // rather than no data
            if (!image__RESTART(z->marker))
              return 1;
            image__jpeg_reset(z);
          }
        }
      }
      return 1;
    } else { // interleaved
      int i, j, k, x, y;
      STBI_SIMD_ALIGN(shrt, data[64]);
      for (j = 0; j < z->img_mcu_y; ++j) {
        for (i = 0; i < z->img_mcu_x; ++i) {
          // scan an interleaved mcu... process scan_n components in order
          for (k = 0; k < z->scan_n; ++k) {
            int n = z->order[k];
            // scan out an mcu's worth of this component; that's just determined
            // by the basic H and V specified for the component
            for (y = 0; y < z->img_comp[n].v; ++y) {
              for (x = 0; x < z->img_comp[n].h; ++x) {
                int x2 = (i * z->img_comp[n].h + x) * 8;
                int y2 = (j * z->img_comp[n].v + y) * 8;
                int ha = z->img_comp[n].ha;
                if (!image__jpeg_decode_block(z, data, z->huff_dc + z->img_comp[n].hd, z->huff_ac + ha, z->fast_ac[ha], n, z->dequant[z->img_comp[n].tq]))
                  return 0;
                z->idct_block_kernel(z->img_comp[n].data + z->img_comp[n].w2 * y2 + x2, z->img_comp[n].w2, data);
              }
            }
          }
          // after all interleaved components, that's an interleaved MCU,
          // so now count down the restart interval
          if (--z->todo <= 0) {
            if (z->code_bits < 24)
              image__grow_buffer_unsafe(z);
            if (!image__RESTART(z->marker))
              return 1;
            image__jpeg_reset(z);
          }
        }
      }
      return 1;
    }
  } else {
    if (z->scan_n == 1) {
      int i, j;
      int n = z->order[0];
      // non-interleaved data, we just need to process one block at a time,
      // in trivial scanline order
      // number of blocks to do just depends on how many actual "pixels" this
      // component has, independent of interleaved MCU blocking and such
      int w = (z->img_comp[n].x + 7) >> 3;
      int h = (z->img_comp[n].y + 7) >> 3;
      for (j = 0; j < h; ++j) {
        for (i = 0; i < w; ++i) {
          shrt *data = z->img_comp[n].coeff + 64 * (i + j * z->img_comp[n].coeff_w);
          if (z->spec_start == 0) {
            if (!image__jpeg_decode_block_prog_dc(z, data, &z->huff_dc[z->img_comp[n].hd], n))
              return 0;
          } else {
            int ha = z->img_comp[n].ha;
            if (!image__jpeg_decode_block_prog_ac(z, data, &z->huff_ac[ha], z->fast_ac[ha]))
              return 0;
          }
          // every data block is an MCU, so countdown the restart interval
          if (--z->todo <= 0) {
            if (z->code_bits < 24)
              image__grow_buffer_unsafe(z);
            if (!image__RESTART(z->marker))
              return 1;
            image__jpeg_reset(z);
          }
        }
      }
      return 1;
    } else { // interleaved
      int i, j, k, x, y;
      for (j = 0; j < z->img_mcu_y; ++j) {
        for (i = 0; i < z->img_mcu_x; ++i) {
          // scan an interleaved mcu... process scan_n components in order
          for (k = 0; k < z->scan_n; ++k) {
            int n = z->order[k];
            // scan out an mcu's worth of this component; that's just determined
            // by the basic H and V specified for the component
            for (y = 0; y < z->img_comp[n].v; ++y) {
              for (x = 0; x < z->img_comp[n].h; ++x) {
                int x2 = (i * z->img_comp[n].h + x);
                int y2 = (j * z->img_comp[n].v + y);
                shrt *data = z->img_comp[n].coeff + 64 * (x2 + y2 * z->img_comp[n].coeff_w);
                if (!image__jpeg_decode_block_prog_dc(z, data, &z->huff_dc[z->img_comp[n].hd], n))
                  return 0;
              }
            }
          }
          // after all interleaved components, that's an interleaved MCU,
          // so now count down the restart interval
          if (--z->todo <= 0) {
            if (z->code_bits < 24)
              image__grow_buffer_unsafe(z);
            if (!image__RESTART(z->marker))
              return 1;
            image__jpeg_reset(z);
          }
        }
      }
      return 1;
    }
  }
}
static void image__jpeg_dequantize(shrt *data, ushrt *dequant) {
  int i;
  for (i = 0; i < 64; ++i)
    data[i] *= dequant[i];
}
static void image__jpeg_finish(image__jpeg *z) {
  if (z->progressive) {
    // dequantize and idct the data
    int i, j, n;
    for (n = 0; n < z->s->img_n; ++n) {
      int w = (z->img_comp[n].x + 7) >> 3;
      int h = (z->img_comp[n].y + 7) >> 3;
      for (j = 0; j < h; ++j) {
        for (i = 0; i < w; ++i) {
          shrt *data = z->img_comp[n].coeff + 64 * (i + j * z->img_comp[n].coeff_w);
          image__jpeg_dequantize(data, z->dequant[z->img_comp[n].tq]);
          z->idct_block_kernel(z->img_comp[n].data + z->img_comp[n].w2 * j * 8 + i * 8, z->img_comp[n].w2, data);
        }
      }
    }
  }
}
static int image__process_marker(image__jpeg *z, int m) {
  int L;
  switch (m) {
  case image__MARKER_none: // no marker found
    return stb_set_error("expected marker : Corrupt JPEG");

  case 0xDD: // DRI - specify restart interval
    if (image__get16be(z->s) != 4)
      return stb_set_error("bad DRI len : Corrupt JPEG");
    z->restart_interval = image__get16be(z->s);
    return 1;

  case 0xDB: // DQT - define quantization table
    L = image__get16be(z->s) - 2;
    while (L > 0) {
      int q = image__get8(z->s);
      int p = q >> 4, sixteen = (p != 0);
      int t = q & 15, i;
      if (p != 0 && p != 1)
        return stb_set_error("bad DQT type : Corrupt JPEG");
      if (t > 3)
        return stb_set_error("bad DQT table : Corrupt JPEG");

      for (i = 0; i < 64; ++i)
        z->dequant[t][image__jpeg_dezigzag[i]] = (ushrt)(sixteen ? image__get16be(z->s) : image__get8(z->s));
      L -= (sixteen ? 129 : 65);
    }
    return L == 0;

  case 0xC4: // DHT - define huffman table
    L = image__get16be(z->s) - 2;
    while (L > 0) {
      ubyte *v;
      int sizes[16], i, n = 0;
      int q = image__get8(z->s);
      int tc = q >> 4;
      int th = q & 15;
      if (tc > 1 || th > 3)
        return stb_set_error("bad DHT header : Corrupt JPEG");
      for (i = 0; i < 16; ++i) {
        sizes[i] = image__get8(z->s);
        n += sizes[i];
      }
      if (n > 256)
        return stb_set_error("bad DHT header : Corrupt JPEG"); // Loop over i < n would write past end of values!
      L -= 17;
      if (tc == 0) {
        if (!image__build_huffman(z->huff_dc + th, sizes))
          return 0;
        v = z->huff_dc[th].values;
      } else {
        if (!image__build_huffman(z->huff_ac + th, sizes))
          return 0;
        v = z->huff_ac[th].values;
      }
      for (i = 0; i < n; ++i)
        v[i] = image__get8(z->s);
      if (tc != 0)
        image__build_fast_ac(z->fast_ac[th], z->huff_ac + th);
      L -= n;
    }
    return L == 0;
  }

  // check for comment block or APP blocks
  if ((m >= 0xE0 && m <= 0xEF) || m == 0xFE) {
    L = image__get16be(z->s);
    if (L < 2) {
      if (m == 0xFE)
        return stb_set_error("bad COM len : Corrupt JPEG");
      else
        return stb_set_error("bad APP len : Corrupt JPEG");
    }
    L -= 2;

    if (m == 0xE0 && L >= 5) { // JFIF APP0 segment
      static const ubyte tag[5] = {'J', 'F', 'I', 'F', '\0'};
      int ok = 1;
      int i;
      for (i = 0; i < 5; ++i)
        if (image__get8(z->s) != tag[i])
          ok = 0;
      L -= 5;
      if (ok)
        z->jfif = 1;
    } else if (m == 0xEE && L >= 12) { // Adobe APP14 segment
      static const ubyte tag[6] = {'A', 'd', 'o', 'b', 'e', '\0'};
      int ok = 1;
      int i;
      for (i = 0; i < 6; ++i)
        if (image__get8(z->s) != tag[i])
          ok = 0;
      L -= 6;
      if (ok) {
        image__get8(z->s);                            // version
        image__get16be(z->s);                         // flags0
        image__get16be(z->s);                         // flags1
        z->app14_color_transform = image__get8(z->s); // color transform
        L -= 6;
      }
    }

    image__skip(z->s, L);
    return 1;
  }

  return stb_set_error("unknown marker : Corrupt JPEG");
}

// after we see SOS
static int image__process_scan_header(image__jpeg *z) {
  int i;
  int Ls = image__get16be(z->s);
  z->scan_n = image__get8(z->s);
  if (z->scan_n < 1 || z->scan_n > 4 || z->scan_n > (int)z->s->img_n)
    return stb_set_error("bad SOS component count : Corrupt JPEG");
  if (Ls != 6 + 2 * z->scan_n)
    return stb_set_error("bad SOS len : Corrupt JPEG");
  for (i = 0; i < z->scan_n; ++i) {
    int id = image__get8(z->s), which;
    int q = image__get8(z->s);
    for (which = 0; which < z->s->img_n; ++which)
      if (z->img_comp[which].id == id)
        break;
    if (which == z->s->img_n)
      return 0; // no match
    z->img_comp[which].hd = q >> 4;
    if (z->img_comp[which].hd > 3)
      return stb_set_error("bad DC huff : Corrupt JPEG");
    z->img_comp[which].ha = q & 15;
    if (z->img_comp[which].ha > 3)
      return stb_set_error("bad AC huff : Corrupt JPEG");
    z->order[i] = which;
  }

  {
    int aa;
    z->spec_start = image__get8(z->s);
    z->spec_end = image__get8(z->s); // should be 63, but might be 0
    aa = image__get8(z->s);
    z->succ_high = (aa >> 4);
    z->succ_low = (aa & 15);
    if (z->progressive) {
      if (z->spec_start > 63 || z->spec_end > 63 || z->spec_start > z->spec_end || z->succ_high > 13 || z->succ_low > 13)
        return stb_set_error("bad SOS : Corrupt JPEG");
    } else {
      if (z->spec_start != 0)
        return stb_set_error("bad SOS : Corrupt JPEG");
      if (z->succ_high != 0 || z->succ_low != 0)
        return stb_set_error("bad SOS : Corrupt JPEG");
      z->spec_end = 63;
    }
  }

  return 1;
}
static int image__free_jpeg_components(image__jpeg *z, int ncomp, int why) {
  int i;
  for (i = 0; i < ncomp; ++i) {
    if (z->img_comp[i].raw_data) {
      util_memfree(z->img_comp[i].raw_data);
      z->img_comp[i].raw_data = NULL;
      z->img_comp[i].data = NULL;
    }
    if (z->img_comp[i].raw_coeff) {
      util_memfree(z->img_comp[i].raw_coeff);
      z->img_comp[i].raw_coeff = 0;
      z->img_comp[i].coeff = 0;
    }
    if (z->img_comp[i].linebuf) {
      util_memfree(z->img_comp[i].linebuf);
      z->img_comp[i].linebuf = NULL;
    }
  }
  return why;
}
static int image__process_frame_header(image__jpeg *z, int scan) {
  image__context *s = z->s;
  int Lf, p, i, q, h_max = 1, v_max = 1, c;
  Lf = image__get16be(s);
  if (Lf < 11)
    return stb_set_error("bad SOF len : Corrupt JPEG"); // JPEG
  p = image__get8(s);
  if (p != 8)
    return stb_set_error("only 8-bit : JPEG format not supported: 8-bit only"); // JPEG baseline
  s->img_y = image__get16be(s);
  if (s->img_y == 0)
    return stb_set_error("no header height : JPEG format not supported: delayed height"); // Legal, but we don't handle it--but neither does IJG
  s->img_x = image__get16be(s);
  if (s->img_x == 0)
    return stb_set_error("0 width : Corrupt JPEG"); // JPEG requires
  if (s->img_y > MAX_DIMENSIONS)
    return stb_set_error("Very large image (corrupt?)");
  if (s->img_x > MAX_DIMENSIONS)
    return stb_set_error("Very large image (corrupt?)");
  c = image__get8(s);
  if (c != 3 && c != 1 && c != 4)
    return stb_set_error("bad component count : Corrupt JPEG");
  s->img_n = c;
  for (i = 0; i < c; ++i) {
    z->img_comp[i].data = NULL;
    z->img_comp[i].linebuf = NULL;
  }

  if (Lf != 8 + 3 * s->img_n)
    return stb_set_error("bad SOF len : Corrupt JPEG");

  z->rgb = 0;
  for (i = 0; i < s->img_n; ++i) {
    static const ubyte rgb[3] = {'R', 'G', 'B'};
    z->img_comp[i].id = image__get8(s);
    if (s->img_n == 3 && z->img_comp[i].id == rgb[i])
      ++z->rgb;
    q = image__get8(s);
    z->img_comp[i].h = (q >> 4);
    if (!z->img_comp[i].h || z->img_comp[i].h > 4)
      return stb_set_error("bad H : Corrupt JPEG");
    z->img_comp[i].v = q & 15;
    if (!z->img_comp[i].v || z->img_comp[i].v > 4)
      return stb_set_error("bad V : Corrupt JPEG");
    z->img_comp[i].tq = image__get8(s);
    if (z->img_comp[i].tq > 3)
      return stb_set_error("bad TQ : Corrupt JPEG");
  }

  if (scan != image__SCAN_load)
    return 1;

  if (!image__validm3ad(s->img_x, s->img_y, s->img_n, 0))
    return stb_set_error("Image too large to decode");

  for (i = 0; i < s->img_n; ++i) {
    if (z->img_comp[i].h > h_max)
      h_max = z->img_comp[i].h;
    if (z->img_comp[i].v > v_max)
      v_max = z->img_comp[i].v;
  }

  // check that plane subsampling factors are integer ratios; our resamplers can't deal with fractional ratios
  // and I've never seen a non-corrupted JPEG file actually use them
  for (i = 0; i < s->img_n; ++i) {
    if (h_max % z->img_comp[i].h != 0)
      return stb_set_error("bad H : Corrupt JPEG");
    if (v_max % z->img_comp[i].v != 0)
      return stb_set_error("bad V : Corrupt JPEG");
  }

  // compute interleaved mcu info
  z->img_h_max = h_max;
  z->img_v_max = v_max;
  z->img_mcu_w = h_max * 8;
  z->img_mcu_h = v_max * 8;
  // these sizes can't be more than 17 bits
  z->img_mcu_x = (s->img_x + z->img_mcu_w - 1) / z->img_mcu_w;
  z->img_mcu_y = (s->img_y + z->img_mcu_h - 1) / z->img_mcu_h;

  for (i = 0; i < s->img_n; ++i) {
    // number of effective pixels (e.g. for non-interleaved MCU)
    z->img_comp[i].x = (s->img_x * z->img_comp[i].h + h_max - 1) / h_max;
    z->img_comp[i].y = (s->img_y * z->img_comp[i].v + v_max - 1) / v_max;
    // to simplify generation, we'll allocate enough memory to decode
    // the bogus oversized data from using interleaved MCUs and their
    // big blocks (e.g. a 16x16 iMCU on an image of width 33); we won't
    // discard the extra data until colorspace conversion
    //
    // img_mcu_x, img_mcu_y: <=17 bits; comp[i].h and .v are <=4 (checked earlier)
    // so these muls can't overflow with 32-bit ints (which we require)
    z->img_comp[i].w2 = z->img_mcu_x * z->img_comp[i].h * 8;
    z->img_comp[i].h2 = z->img_mcu_y * z->img_comp[i].v * 8;
    z->img_comp[i].coeff = 0;
    z->img_comp[i].raw_coeff = 0;
    z->img_comp[i].linebuf = NULL;
    z->img_comp[i].raw_data = image__malloc_mad2(z->img_comp[i].w2, z->img_comp[i].h2, 15);
    if (z->img_comp[i].raw_data == NULL)
      return image__free_jpeg_components(z, i + 1, stb_set_error("outofmem : Out of memory"));
    // align blocks for idct using mmx/sse
    z->img_comp[i].data = (ubyte *)(((iter)z->img_comp[i].raw_data + 15) & ~15);
    if (z->progressive) {
      // w2, h2 are multiples of 8 (see above)
      z->img_comp[i].coeff_w = z->img_comp[i].w2 / 8;
      z->img_comp[i].coeff_h = z->img_comp[i].h2 / 8;
      z->img_comp[i].raw_coeff = image__malloc_mad3(z->img_comp[i].w2, z->img_comp[i].h2, sizeof(shrt), 15);
      if (z->img_comp[i].raw_coeff == NULL)
        return image__free_jpeg_components(z, i + 1, stb_set_error("outofmem : Out of memory"));
      z->img_comp[i].coeff = (shrt *)(((iter)z->img_comp[i].raw_coeff + 15) & ~15);
    }
  }

  return 1;
}
// use comparisons since in some cases we handle more than one case (e.g. SOF)
#define image__DNL(x) ((x) == 0xdc)
#define image__SOI(x) ((x) == 0xd8)
#define image__EOI(x) ((x) == 0xd9)
#define image__SOF(x) ((x) == 0xc0 || (x) == 0xc1 || (x) == 0xc2)
#define image__SOS(x) ((x) == 0xda)
#define image__SOF_progressive(x) ((x) == 0xc2)
static int image__decode_jpeg_header(image__jpeg *z, int scan) {
  int m;
  z->jfif = 0;
  z->app14_color_transform = -1; // valid values are 0,1,2
  z->marker = image__MARKER_none; // initialize cached marker to empty
  m = image__get_marker(z);
  if (!image__SOI(m))
    return stb_set_error("no SOI : Corrupt JPEG");
  if (scan == image__SCAN_type)
    return 1;
  m = image__get_marker(z);
  while (!image__SOF(m)) {
    if (!image__process_marker(z, m))
      return 0;
    m = image__get_marker(z);
    while (m == image__MARKER_none) {
      // some files have extra padding after their blocks, so ok, we'll scan
      if (image__eof(z->s))
        return stb_set_error("no SOF : Corrupt JPEG");
      m = image__get_marker(z);
    }
  }
  z->progressive = image__SOF_progressive(m);
  if (!image__process_frame_header(z, scan))
    return 0;
  return 1;
}
static ubyte image__skip_jpeg_junk_at_end(image__jpeg *j) {
  // some JPEGs have junk at end, skip over it but if we find what looks
  // like a valid marker, resume there
  while (!image__eof(j->s)) {
    ubyte x = image__get8(j->s);
    while (x == 0xff) { // might be a marker
      if (image__eof(j->s))
        return image__MARKER_none;
      x = image__get8(j->s);
      if (x != 0x00 && x != 0xff) {
        // not a stuffed zero or lead-in to another marker, looks
        // like an actual marker, return it
        return x;
      }
      // stuffed zero has x=0 now which ends the loop, meaning we go
      // back to regular scan loop.
      // repeated 0xff keeps trying to read the next byte of the marker.
    }
  }
  return image__MARKER_none;
}
// decode image to YCbCr format
static int image__decode_jpeg_image(image__jpeg *j) {
  int m;
  for (m = 0; m < 4; m++) {
    j->img_comp[m].raw_data = NULL;
    j->img_comp[m].raw_coeff = NULL;
  }
  j->restart_interval = 0;
  if (!image__decode_jpeg_header(j, image__SCAN_load))
    return 0;
  m = image__get_marker(j);
  while (!image__EOI(m)) {
    if (image__SOS(m)) {
      if (!image__process_scan_header(j))
        return 0;
      if (!image__parse_entropy_coded_data(j))
        return 0;
      if (j->marker == image__MARKER_none) {
        j->marker = image__skip_jpeg_junk_at_end(j);
        // if we reach eof without hitting a marker, image__get_marker() below will fail and we'll eventually return 0
      }
      m = image__get_marker(j);
      if (image__RESTART(m))
        m = image__get_marker(j);
    } else if (image__DNL(m)) {
      int Ld = image__get16be(j->s);
      uint32 NL = image__get16be(j->s);
      if (Ld != 4)
        return stb_set_error("bad DNL len : Corrupt JPEG");
      if (NL != j->s->img_y)
        return stb_set_error("bad DNL height : Corrupt JPEG");
      m = image__get_marker(j);
    } else {
      if (!image__process_marker(j, m))
        return 1;
      m = image__get_marker(j);
    }
  }
  if (j->progressive)
    image__jpeg_finish(j);
  return 1;
}
// static jfif-centered resampling (across block boundaries)
typedef ubyte *(*resample_row_func)(ubyte *out, ubyte *in0, ubyte *in1,
                                      int w, int hs);
#define image__div4(x) ((ubyte)((x) >> 2))
static ubyte *resample_row_1(ubyte *out, ubyte *in_near, ubyte *in_far, int w, int hs) {
  UNUSED(out);
  UNUSED(in_far);
  UNUSED(w);
  UNUSED(hs);
  return in_near;
}
static ubyte *image__resample_row_v_2(ubyte *out, ubyte *in_near, ubyte *in_far, int w, int hs) {
  // need to generate two samples vertically for every one in input
  int i;
  UNUSED(hs);
  for (i = 0; i < w; ++i)
    out[i] = image__div4(3 * in_near[i] + in_far[i] + 2);
  return out;
}
static ubyte *image__resample_row_h_2(ubyte *out, ubyte *in_near, ubyte *in_far, int w, int hs) {
  // need to generate two samples horizontally for every one in input
  int i;
  ubyte *input = in_near;

  if (w == 1) {
    // if only one sample, can't do any interpolation
    out[0] = out[1] = input[0];
    return out;
  }

  out[0] = input[0];
  out[1] = image__div4(input[0] * 3 + input[1] + 2);
  for (i = 1; i < w - 1; ++i) {
    int n = 3 * input[i] + 2;
    out[i * 2 + 0] = image__div4(n + input[i - 1]);
    out[i * 2 + 1] = image__div4(n + input[i + 1]);
  }
  out[i * 2 + 0] = image__div4(input[w - 2] * 3 + input[w - 1] + 2);
  out[i * 2 + 1] = input[w - 1];

  UNUSED(in_far);
  UNUSED(hs);

  return out;
}
#define image__div16(x) ((ubyte)((x) >> 4))

static ubyte *image__resample_row_hv_2(ubyte *out, ubyte *in_near, ubyte *in_far, int w, int hs) {
  // need to generate 2x2 samples for every one in input
  int i, t0, t1;
  if (w == 1) {
    out[0] = out[1] = image__div4(3 * in_near[0] + in_far[0] + 2);
    return out;
  }

  t1 = 3 * in_near[0] + in_far[0];
  out[0] = image__div4(t1 + 2);
  for (i = 1; i < w; ++i) {
    t0 = t1;
    t1 = 3 * in_near[i] + in_far[i];
    out[i * 2 - 1] = image__div16(3 * t0 + t1 + 8);
    out[i * 2] = image__div16(3 * t1 + t0 + 8);
  }
  out[w * 2 - 1] = image__div4(t1 + 2);

  UNUSED(hs);

  return out;
}

#if defined(STBI_SSE2) || defined(STBI_NEON)
static ubyte *image__resample_row_hv_2_simd(ubyte *out, ubyte *in_near, ubyte *in_far, int w, int hs) {
  // need to generate 2x2 samples for every one in input
  int i = 0, t0, t1;

  if (w == 1) {
    out[0] = out[1] = image__div4(3 * in_near[0] + in_far[0] + 2);
    return out;
  }

  t1 = 3 * in_near[0] + in_far[0];
  // process groups of 8 pixels for as long as we can.
  // note we can't handle the last pixel in a row in this loop
  // because we need to handle the filter boundary conditions.
  for (; i < ((w - 1) & ~7); i += 8) {
#if defined(STBI_SSE2)
    // load and perform the vertical filtering pass
    // this uses 3*x + y = 4*x + (y - x)
    __m128i zero = _mm_setzero_si128();
    __m128i farb = _mm_loadl_epi64((__m128i *)(in_far + i));
    __m128i nearb = _mm_loadl_epi64((__m128i *)(in_near + i));
    __m128i farw = _mm_unpacklo_epi8(farb, zero);
    __m128i nearw = _mm_unpacklo_epi8(nearb, zero);
    __m128i diff = _mm_sub_epi16(farw, nearw);
    __m128i nears = _mm_slli_epi16(nearw, 2);
    __m128i curr = _mm_add_epi16(nears, diff); // current row

    // horizontal filter works the same based on shifted vers of current
    // row. "prev" is current row shifted right by 1 pixel; we need to
    // insert the previous pixel value (from t1).
    // "next" is current row shifted left by 1 pixel, with first pixel
    // of next block of 8 pixels added in.
    __m128i prv0 = _mm_slli_si128(curr, 2);
    __m128i nxt0 = _mm_srli_si128(curr, 2);
    __m128i prev = _mm_insert_epi16(prv0, t1, 0);
    __m128i next = _mm_insert_epi16(nxt0, 3 * in_near[i + 8] + in_far[i + 8], 7);

    // horizontal filter, polyphase implementation since it's convenient:
    // even pixels = 3*cur + prev = cur*4 + (prev - cur)
    // odd  pixels = 3*cur + next = cur*4 + (next - cur)
    // note the shared term.
    __m128i bias = _mm_set1_epi16(8);
    __m128i curs = _mm_slli_epi16(curr, 2);
    __m128i prvd = _mm_sub_epi16(prev, curr);
    __m128i nxtd = _mm_sub_epi16(next, curr);
    __m128i curb = _mm_add_epi16(curs, bias);
    __m128i even = _mm_add_epi16(prvd, curb);
    __m128i odd = _mm_add_epi16(nxtd, curb);

    // interleave even and odd pixels, then undo scaling.
    __m128i int0 = _mm_unpacklo_epi16(even, odd);
    __m128i int1 = _mm_unpackhi_epi16(even, odd);
    __m128i de0 = _mm_srli_epi16(int0, 4);
    __m128i de1 = _mm_srli_epi16(int1, 4);

    // pack and write output
    __m128i outv = _mm_packus_epi16(de0, de1);
    _mm_storeu_si128((__m128i *)(out + i * 2), outv);
#elif defined(STBI_NEON)
    // load and perform the vertical filtering pass
    // this uses 3*x + y = 4*x + (y - x)
    uint8x8_t farb = vld1_u8(in_far + i);
    uint8x8_t nearb = vld1_u8(in_near + i);
    int16x8_t diff = vreinterpretq_s16_u16(vsubl_u8(farb, nearb));
    int16x8_t nears = vreinterpretq_s16_u16(vshll_n_u8(nearb, 2));
    int16x8_t curr = vaddq_s16(nears, diff); // current row

    // horizontal filter works the same based on shifted vers of current
    // row. "prev" is current row shifted right by 1 pixel; we need to
    // insert the previous pixel value (from t1).
    // "next" is current row shifted left by 1 pixel, with first pixel
    // of next block of 8 pixels added in.
    int16x8_t prv0 = vextq_s16(curr, curr, 7);
    int16x8_t nxt0 = vextq_s16(curr, curr, 1);
    int16x8_t prev = vsetq_lane_s16(t1, prv0, 0);
    int16x8_t next = vsetq_lane_s16(3 * in_near[i + 8] + in_far[i + 8], nxt0, 7);

    // horizontal filter, polyphase implementation since it's convenient:
    // even pixels = 3*cur + prev = cur*4 + (prev - cur)
    // odd  pixels = 3*cur + next = cur*4 + (next - cur)
    // note the shared term.
    int16x8_t curs = vshlq_n_s16(curr, 2);
    int16x8_t prvd = vsubq_s16(prev, curr);
    int16x8_t nxtd = vsubq_s16(next, curr);
    int16x8_t even = vaddq_s16(curs, prvd);
    int16x8_t odd = vaddq_s16(curs, nxtd);

    // undo scaling and round, then store with even/odd phases interleaved
    uint8x8x2_t o;
    o.val[0] = vqrshrun_n_s16(even, 4);
    o.val[1] = vqrshrun_n_s16(odd, 4);
    vst2_u8(out + i * 2, o);
#endif

    // "previous" value for next iter
    t1 = 3 * in_near[i + 7] + in_far[i + 7];
  }

  t0 = t1;
  t1 = 3 * in_near[i] + in_far[i];
  out[i * 2] = image__div16(3 * t1 + t0 + 8);

  for (++i; i < w; ++i) {
    t0 = t1;
    t1 = 3 * in_near[i] + in_far[i];
    out[i * 2 - 1] = image__div16(3 * t0 + t1 + 8);
    out[i * 2] = image__div16(3 * t1 + t0 + 8);
  }
  out[w * 2 - 1] = image__div4(t1 + 2);

  UNUSED(hs);

  return out;
}
#endif

static ubyte *image__resample_row_generic(ubyte *out, ubyte *in_near, ubyte *in_far, int w, int hs) {
  // resample with nearest-neighbor
  int i, j;
  UNUSED(in_far);
  for (i = 0; i < w; ++i)
    for (j = 0; j < hs; ++j)
      out[i * hs + j] = in_near[i];
  return out;
}

// this is a reduced-precision calculation of YCbCr-to-RGB introduced
// to make sure the code produces the same results in both SIMD and scalar
#define image__float2fixed(x) (((int)((x) * 4096.0f + 0.5f)) << 8)
static void image__YCbCr_to_RGB_row(ubyte *out, const ubyte *y, const ubyte *pcb, const ubyte *pcr, int count, int step) {
  int i;
  for (i = 0; i < count; ++i) {
    int y_fixed = (y[i] << 20) + (1 << 19); // rounding
    int r, g, b;
    int cr = pcr[i] - 128;
    int cb = pcb[i] - 128;
    r = y_fixed + cr * image__float2fixed(1.40200f);
    g = y_fixed + (cr * -image__float2fixed(0.71414f)) + ((cb * -image__float2fixed(0.34414f)) & 0xffff0000);
    b = y_fixed + cb * image__float2fixed(1.77200f);
    r >>= 20;
    g >>= 20;
    b >>= 20;
    if ((unsigned)r > 255) {
      if (r < 0)
        r = 0;
      else
        r = 255;
    }
    if ((unsigned)g > 255) {
      if (g < 0)
        g = 0;
      else
        g = 255;
    }
    if ((unsigned)b > 255) {
      if (b < 0)
        b = 0;
      else
        b = 255;
    }
    out[0] = (ubyte)r;
    out[1] = (ubyte)g;
    out[2] = (ubyte)b;
    out[3] = 255;
    out += step;
  }
}

#if defined(STBI_SSE2) || defined(STBI_NEON)
static void image__YCbCr_to_RGB_simd(ubyte *out, ubyte const *y, ubyte const *pcb, ubyte const *pcr, int count, int step) {
  int i = 0;

#ifdef STBI_SSE2
  // step == 3 is pretty ugly on the final interleave, and i'm not convinced
  // it's useful in practice (you wouldn't use it for textures, for example).
  // so just accelerate step == 4 case.
  if (step == 4) {
    // this is a fairly straightforward implementation and not super-optimized.
    __m128i signflip = _mm_set1_epi8(-0x80);
    __m128i cr_const0 = _mm_set1_epi16((shrt)(1.40200f * 4096.0f + 0.5f));
    __m128i cr_const1 = _mm_set1_epi16(-(shrt)(0.71414f * 4096.0f + 0.5f));
    __m128i cb_const0 = _mm_set1_epi16(-(shrt)(0.34414f * 4096.0f + 0.5f));
    __m128i cb_const1 = _mm_set1_epi16((shrt)(1.77200f * 4096.0f + 0.5f));
    __m128i y_bias = _mm_set1_epi8((char)(ubyte)128);
    __m128i xw = _mm_set1_epi16(255); // alpha channel

    for (; i + 7 < count; i += 8) {
      // load
      __m128i y_bytes = _mm_loadl_epi64((__m128i *)(y + i));
      __m128i cr_bytes = _mm_loadl_epi64((__m128i *)(pcr + i));
      __m128i cb_bytes = _mm_loadl_epi64((__m128i *)(pcb + i));
      __m128i cr_biased = _mm_xor_si128(cr_bytes, signflip); // -128
      __m128i cb_biased = _mm_xor_si128(cb_bytes, signflip); // -128

      // unpack to shrt (and left-shift cr, cb by 8)
      __m128i yw = _mm_unpacklo_epi8(y_bias, y_bytes);
      __m128i crw = _mm_unpacklo_epi8(_mm_setzero_si128(), cr_biased);
      __m128i cbw = _mm_unpacklo_epi8(_mm_setzero_si128(), cb_biased);

      // color transform
      __m128i yws = _mm_srli_epi16(yw, 4);
      __m128i cr0 = _mm_mulhi_epi16(cr_const0, crw);
      __m128i cb0 = _mm_mulhi_epi16(cb_const0, cbw);
      __m128i cb1 = _mm_mulhi_epi16(cbw, cb_const1);
      __m128i cr1 = _mm_mulhi_epi16(crw, cr_const1);
      __m128i rws = _mm_add_epi16(cr0, yws);
      __m128i gwt = _mm_add_epi16(cb0, yws);
      __m128i bws = _mm_add_epi16(yws, cb1);
      __m128i gws = _mm_add_epi16(gwt, cr1);

      // descale
      __m128i rw = _mm_srai_epi16(rws, 4);
      __m128i bw = _mm_srai_epi16(bws, 4);
      __m128i gw = _mm_srai_epi16(gws, 4);

      // back to byte, set up for transpose
      __m128i brb = _mm_packus_epi16(rw, bw);
      __m128i gxb = _mm_packus_epi16(gw, xw);

      // transpose to interleave channels
      __m128i t0 = _mm_unpacklo_epi8(brb, gxb);
      __m128i t1 = _mm_unpackhi_epi8(brb, gxb);
      __m128i o0 = _mm_unpacklo_epi16(t0, t1);
      __m128i o1 = _mm_unpackhi_epi16(t0, t1);

      // store
      _mm_storeu_si128((__m128i *)(out + 0), o0);
      _mm_storeu_si128((__m128i *)(out + 16), o1);
      out += 32;
    }
  }
#endif

#ifdef STBI_NEON
  // in this version, step=3 support would be easy to add. but is there demand?
  if (step == 4) {
    // this is a fairly straightforward implementation and not super-optimized.
    uint8x8_t signflip = vdup_n_u8(0x80);
    int16x8_t cr_const0 = vdupq_n_s16((shrt)(1.40200f * 4096.0f + 0.5f));
    int16x8_t cr_const1 = vdupq_n_s16(-(shrt)(0.71414f * 4096.0f + 0.5f));
    int16x8_t cb_const0 = vdupq_n_s16(-(shrt)(0.34414f * 4096.0f + 0.5f));
    int16x8_t cb_const1 = vdupq_n_s16((shrt)(1.77200f * 4096.0f + 0.5f));

    for (; i + 7 < count; i += 8) {
      // load
      uint8x8_t y_bytes = vld1_u8(y + i);
      uint8x8_t cr_bytes = vld1_u8(pcr + i);
      uint8x8_t cb_bytes = vld1_u8(pcb + i);
      int8x8_t cr_biased = vreinterpret_s8_u8(vsub_u8(cr_bytes, signflip));
      int8x8_t cb_biased = vreinterpret_s8_u8(vsub_u8(cb_bytes, signflip));

      // expand to s16
      int16x8_t yws = vreinterpretq_s16_u16(vshll_n_u8(y_bytes, 4));
      int16x8_t crw = vshll_n_s8(cr_biased, 7);
      int16x8_t cbw = vshll_n_s8(cb_biased, 7);

      // color transform
      int16x8_t cr0 = vqdmulhq_s16(crw, cr_const0);
      int16x8_t cb0 = vqdmulhq_s16(cbw, cb_const0);
      int16x8_t cr1 = vqdmulhq_s16(crw, cr_const1);
      int16x8_t cb1 = vqdmulhq_s16(cbw, cb_const1);
      int16x8_t rws = vaddq_s16(yws, cr0);
      int16x8_t gws = vaddq_s16(vaddq_s16(yws, cb0), cr1);
      int16x8_t bws = vaddq_s16(yws, cb1);

      // undo scaling, round, convert to byte
      uint8x8x4_t o;
      o.val[0] = vqrshrun_n_s16(rws, 4);
      o.val[1] = vqrshrun_n_s16(gws, 4);
      o.val[2] = vqrshrun_n_s16(bws, 4);
      o.val[3] = vdup_n_u8(255);

      // store, interleaving r/g/b/a
      vst4_u8(out, o);
      out += 8 * 4;
    }
  }
#endif

  for (; i < count; ++i) {
    int y_fixed = (y[i] << 20) + (1 << 19); // rounding
    int r, g, b;
    int cr = pcr[i] - 128;
    int cb = pcb[i] - 128;
    r = y_fixed + cr * image__float2fixed(1.40200f);
    g = y_fixed + cr * -image__float2fixed(0.71414f) + ((cb * -image__float2fixed(0.34414f)) & 0xffff0000);
    b = y_fixed + cb * image__float2fixed(1.77200f);
    r >>= 20;
    g >>= 20;
    b >>= 20;
    if ((unsigned)r > 255) {
      if (r < 0)
        r = 0;
      else
        r = 255;
    }
    if ((unsigned)g > 255) {
      if (g < 0)
        g = 0;
      else
        g = 255;
    }
    if ((unsigned)b > 255) {
      if (b < 0)
        b = 0;
      else
        b = 255;
    }
    out[0] = (ubyte)r;
    out[1] = (ubyte)g;
    out[2] = (ubyte)b;
    out[3] = 255;
    out += step;
  }
}
#endif

// set up the kernels
static void image__setup_jpeg(image__jpeg *j) {
  j->idct_block_kernel = image__idct_block;
  j->YCbCr_to_RGB_kernel = image__YCbCr_to_RGB_row;
  j->resample_row_hv_2_kernel = image__resample_row_hv_2;

#ifdef STBI_SSE2
  if (image__sse2_available()) {
    j->idct_block_kernel = image__idct_simd;
    j->YCbCr_to_RGB_kernel = image__YCbCr_to_RGB_simd;
    j->resample_row_hv_2_kernel = image__resample_row_hv_2_simd;
  }
#endif

#ifdef STBI_NEON
  j->idct_block_kernel = image__idct_simd;
  j->YCbCr_to_RGB_kernel = image__YCbCr_to_RGB_simd;
  j->resample_row_hv_2_kernel = image__resample_row_hv_2_simd;
#endif
}
// clean up the temporary component buffers
static void image__cleanup_jpeg(image__jpeg *j) {
  image__free_jpeg_components(j, j->s->img_n, 0);
}
typedef struct {
  resample_row_func resample;
  ubyte *line0, *line1;
  int hs, vs;  // expansion factor in each axis
  int w_lores; // horizontal pixels pre-expansion
  int ystep;   // how far through vertical expansion we are
  int ypos;    // which pre-expansion row we're on
} image__resample;
// fast 0..255 * 0..255 => 0..255 rounded multiplication
static ubyte image__blinn_8x8(ubyte x, ubyte y) {
  uint t = x * y + 128;
  return (ubyte)((t + (t >> 8)) >> 8);
}
static ubyte *load_jpeg_image(image__jpeg *z, int *out_x, int *out_y, int *comp, int req_comp) {
  int n, decode_n, is_rgb;
  z->s->img_n = 0; // make image__cleanup_jpeg safe

  // validate req_comp
  if (req_comp < 0 || req_comp > 4)
    return (ubyte *)(iter)stb_set_error("bad req_comp : Internal error");

  // load a jpeg image from whichever source, but leave in YCbCr format
  if (!image__decode_jpeg_image(z)) {
    image__cleanup_jpeg(z);
    return NULL;
  }

  // determine actual number of components to generate
  n = req_comp ? req_comp : z->s->img_n >= 3 ? 3
                                             : 1;

  is_rgb = z->s->img_n == 3 && (z->rgb == 3 || (z->app14_color_transform == 0 && !z->jfif));

  if (z->s->img_n == 3 && n < 3 && !is_rgb)
    decode_n = 1;
  else
    decode_n = z->s->img_n;

  // nothing to do if no components requested; check this now to avoid
  // accessing uninitialized coutput[0] later
  if (decode_n <= 0) {
    image__cleanup_jpeg(z);
    return NULL;
  }

  // resample and color-convert
  {
    int k;
    uint i, j;
    ubyte *output;
    ubyte *coutput[4] = {NULL, NULL, NULL, NULL};

    image__resample res_comp[4];

    for (k = 0; k < decode_n; ++k) {
      image__resample *r = &res_comp[k];

      // allocate line buffer big enough for upsampling off the edges
      // with upsample factor of 4
      z->img_comp[k].linebuf = (ubyte *)util_malloc(z->s->img_x + 3);
      if (!z->img_comp[k].linebuf) {
        image__cleanup_jpeg(z);
        return (ubyte *)(iter)stb_set_error("outofmem : Out of memory");
      }

      r->hs = z->img_h_max / z->img_comp[k].h;
      r->vs = z->img_v_max / z->img_comp[k].v;
      r->ystep = r->vs >> 1;
      r->w_lores = (z->s->img_x + r->hs - 1) / r->hs;
      r->ypos = 0;
      r->line0 = r->line1 = z->img_comp[k].data;

      if (r->hs == 1 && r->vs == 1)
        r->resample = resample_row_1;
      else if (r->hs == 1 && r->vs == 2)
        r->resample = image__resample_row_v_2;
      else if (r->hs == 2 && r->vs == 1)
        r->resample = image__resample_row_h_2;
      else if (r->hs == 2 && r->vs == 2)
        r->resample = z->resample_row_hv_2_kernel;
      else
        r->resample = image__resample_row_generic;
    }

    // can't error after this so, this is safe
    output = (ubyte *)image__malloc_mad3(n, z->s->img_x, z->s->img_y, 1);
    if (!output) {
      image__cleanup_jpeg(z);
      return (ubyte *)(iter)stb_set_error("outofmem : Out of memory");
    }

    // now go ahead and resample
    for (j = 0; j < z->s->img_y; ++j) {
      ubyte *out = output + n * z->s->img_x * j;
      for (k = 0; k < decode_n; ++k) {
        image__resample *r = &res_comp[k];
        int y_bot = r->ystep >= (r->vs >> 1);
        coutput[k] = r->resample(z->img_comp[k].linebuf,
                                 y_bot ? r->line1 : r->line0,
                                 y_bot ? r->line0 : r->line1,
                                 r->w_lores, r->hs);
        if (++r->ystep >= r->vs) {
          r->ystep = 0;
          r->line0 = r->line1;
          if (++r->ypos < z->img_comp[k].y)
            r->line1 += z->img_comp[k].w2;
        }
      }
      if (n >= 3) {
        ubyte *y = coutput[0];
        if (z->s->img_n == 3) {
          if (is_rgb) {
            for (i = 0; i < z->s->img_x; ++i) {
              out[0] = y[i];
              out[1] = coutput[1][i];
              out[2] = coutput[2][i];
              out[3] = 255;
              out += n;
            }
          } else {
            z->YCbCr_to_RGB_kernel(out, y, coutput[1], coutput[2], z->s->img_x, n);
          }
        } else if (z->s->img_n == 4) {
          if (z->app14_color_transform == 0) { // CMYK
            for (i = 0; i < z->s->img_x; ++i) {
              ubyte m = coutput[3][i];
              out[0] = image__blinn_8x8(coutput[0][i], m);
              out[1] = image__blinn_8x8(coutput[1][i], m);
              out[2] = image__blinn_8x8(coutput[2][i], m);
              out[3] = 255;
              out += n;
            }
          } else if (z->app14_color_transform == 2) { // YCCK
            z->YCbCr_to_RGB_kernel(out, y, coutput[1], coutput[2], z->s->img_x, n);
            for (i = 0; i < z->s->img_x; ++i) {
              ubyte m = coutput[3][i];
              out[0] = image__blinn_8x8(255 - out[0], m);
              out[1] = image__blinn_8x8(255 - out[1], m);
              out[2] = image__blinn_8x8(255 - out[2], m);
              out += n;
            }
          } else { // YCbCr + alpha?  Ignore the fourth channel for now
            z->YCbCr_to_RGB_kernel(out, y, coutput[1], coutput[2], z->s->img_x, n);
          }
        } else
          for (i = 0; i < z->s->img_x; ++i) {
            out[0] = out[1] = out[2] = y[i];
            out[3] = 255; // not used if n==3
            out += n;
          }
      } else {
        if (is_rgb) {
          if (n == 1)
            for (i = 0; i < z->s->img_x; ++i)
              *out++ = image__compute_y(coutput[0][i], coutput[1][i], coutput[2][i]);
          else {
            for (i = 0; i < z->s->img_x; ++i, out += 2) {
              out[0] = image__compute_y(coutput[0][i], coutput[1][i], coutput[2][i]);
              out[1] = 255;
            }
          }
        } else if (z->s->img_n == 4 && z->app14_color_transform == 0) {
          for (i = 0; i < z->s->img_x; ++i) {
            ubyte m = coutput[3][i];
            ubyte r = image__blinn_8x8(coutput[0][i], m);
            ubyte g = image__blinn_8x8(coutput[1][i], m);
            ubyte b = image__blinn_8x8(coutput[2][i], m);
            out[0] = image__compute_y(r, g, b);
            out[1] = 255;
            out += n;
          }
        } else if (z->s->img_n == 4 && z->app14_color_transform == 2) {
          for (i = 0; i < z->s->img_x; ++i) {
            out[0] = image__blinn_8x8(255 - coutput[0][i], coutput[3][i]);
            out[1] = 255;
            out += n;
          }
        } else {
          ubyte *y = coutput[0];
          if (n == 1)
            for (i = 0; i < z->s->img_x; ++i)
              out[i] = y[i];
          else
            for (i = 0; i < z->s->img_x; ++i) {
              *out++ = y[i];
              *out++ = 255;
            }
        }
      }
    }
    image__cleanup_jpeg(z);
    *out_x = z->s->img_x;
    *out_y = z->s->img_y;
    if (comp)
      *comp = z->s->img_n >= 3 ? 3 : 1; // report original components, not output
    return output;
  }
}
static void *image__jpeg_load(image__context *s, int *x, int *y, int *comp, int req_comp, image__result_info *ri) {
  ubyte *result;
  image__jpeg *j = (image__jpeg *)util_malloc(sizeof(image__jpeg));
  if (!j)
    return (ubyte *)(iter)stb_set_error("outofmem : Out of memory");
  util_memset(j, 0, sizeof(image__jpeg));
  UNUSED(ri);
  j->s = s;
  image__setup_jpeg(j);
  result = load_jpeg_image(j, x, y, comp, req_comp);
  util_memfree(j);
  return result;
}
static int image__jpeg_test(image__context *s) {
  int r;
  image__jpeg *j = (image__jpeg *)util_malloc(sizeof(image__jpeg));
  if (!j)
    return stb_set_error("outofmem : Out of memory");
  util_memset(j, 0, sizeof(image__jpeg));
  j->s = s;
  image__setup_jpeg(j);
  r = image__decode_jpeg_header(j, image__SCAN_type);
  image__rewind(s);
  util_memfree(j);
  return r;
}
static int image__jpeg_info_raw(image__jpeg *j, int *x, int *y, int *comp) {
  if (!image__decode_jpeg_header(j, image__SCAN_header)) {
    image__rewind(j->s);
    return 0;
  }
  if (x)
    *x = j->s->img_x;
  if (y)
    *y = j->s->img_y;
  if (comp)
    *comp = j->s->img_n >= 3 ? 3 : 1;
  return 1;
}
static int image__jpeg_info(image__context *s, int *x, int *y, int *comp) {
  int result;
  image__jpeg *j = (image__jpeg *)(malloc(sizeof(image__jpeg)));
  if (!j)
    return stb_set_error("outofmem : Out of memory");
  util_memset(j, 0, sizeof(image__jpeg));
  j->s = s;
  result = image__jpeg_info_raw(j, x, y, comp);
  util_memfree(j);
  return result;
}

// public domain "baseline" PNG decoder   v0.10  Sean Barrett 2006-11-18
//    simple implementation
//      - only 8-bit samples
//      - no CRC checking
//      - allocates lots of intermediate memory
//        - avoids problem of streaming data between subsystems
//        - avoids explicit window management
//    performance
//      - uses stb_zlib, a PD zlib implementation with fast huffman decoding

enum {
  image__F_none = 0,
  image__F_sub = 1,
  image__F_up = 2,
  image__F_avg = 3,
  image__F_paeth = 4,
  // synthetic filter used for first scanline to avoid needing a dummy row of 0s
  image__F_avg_first
};
static ubyte first_row_filter[5] = {
  image__F_none,
  image__F_sub,
  image__F_none,
  image__F_avg_first,
  image__F_sub // Paeth with b=c=0 turns out to be equivalent to sub
};
static ubyte image__paeth(ubyte a, ubyte b, ubyte c) {
  ubyte thresh = c * 3 - a - b;
  ubyte lo = MIN(a,b);
  ubyte hi = MAX(a,b);
  return (thresh > lo) ? ((hi > thresh) ? c : lo) : hi;
}
static const ubyte image__depth_scale_table[9] = {0, 0xff, 0x55, 0, 0x11, 0, 0, 0, 0x01};
// adds an extra all-255 alpha channel
// dest == src is legal
// img_n must be 1 or 3
static void image__create_png_alpha_expand8(ubyte *dest, ubyte *src, uint32 x, iter chnl) {
  iter i, j, k;
  // must process data backwards since we allow dest==src
  for (i = x, j = chnl, k = chnl + 1; i--;) {
    dest[i * k + j] = 255;
  	while (j--)
      dest[i * k + j] = src[i * chnl + j];
  }
}
// create the png data from post-deflated data
static bool image__create_png_image_raw(image_bitmap *imb, ubyte *raw, iter raw_len, uint32 x, uint32 y, ubyte depth, ubyte color) {
  // @TODO support more bits
  int bytes = (depth == 16 ? 2 : 1);
  uint32 i, j, stride = x * imb->chnl * bytes;
  uint32 img_len, img_width_bytes;
  ubyte *filter_buf;
  int all_ok = 1;
  int k;
  int pixel_bytes = imb->inf.chnl * bytes;
  int width = x;

  imb->data = CAST(ubyte *)image__malloc_mad3(x, y, pixel_bytes, 0); // extra bytes to write off the end into
  if (!imb->data) {
    stb_set_error("outofmem : Out of memory");
    return false;
  }
  // note: error exits here don't need to clean up a->out individually,
  // image__do_png always does on error.
  if (!image__validm3ad(imb->inf.chnl, x, depth, 7))
    return stb_set_error("too large");
  img_width_bytes = (((imb->inf.chnl * x * depth) + 7) >> 3);
  if (!image__validmad(img_width_bytes, y, img_width_bytes))
    return stb_set_error("too large");
  img_len = (img_width_bytes + 1) * y;

  // we used to check for exact match between raw_len and img_len on non-interlaced PNGs,
  // but issue #276 reported a PNG in the wild that had extra data at the end (all zeros),
  // so just check for raw_len < img_len always.
  if (raw_len < img_len)
    return stb_set_error("not enough pixels");

  // Allocate two scan lines worth of filter workspace buffer.
  filter_buf = (ubyte *)image__malloc_mad2(img_width_bytes, 2, 0);
  if (!filter_buf)
    return stb_set_error("outofmem : Out of memory");

  // Filtering for low-bit-depth images
  if (depth < 8) {
    pixel_bytes = 1;
    width = img_width_bytes;
  }

  for (j = 0; j < y; ++j) {
    // cur/prior filter buffers alternate
    ubyte *cur = filter_buf + (j & 1) * img_width_bytes;
    ubyte *prior = filter_buf + (~j & 1) * img_width_bytes;
    ubyte *dest = a->out + stride * j;
    int nk = width * pixel_bytes;
    int filter = *raw++;

    // check filter type
    if (filter > 4) {
      all_ok = stb_set_error("invalid filter");
      break;
    }

    // if first row, use special filter that doesn't sample previous row
    if (j == 0)
      filter = first_row_filter[filter];

    // perform actual filtering
    switch (filter) {
    case image__F_none:
      util_memcpy(cur, raw, nk);
      break;
    case image__F_sub:
      util_memcpy(cur, raw, pixel_bytes);
      for (k = pixel_bytes; k < nk; ++k)
        cur[k] = image__BYTECAST(raw[k] + cur[k - pixel_bytes]);
      break;
    case image__F_up:
      for (k = 0; k < nk; ++k)
        cur[k] = image__BYTECAST(raw[k] + prior[k]);
      break;
    case image__F_avg:
      for (k = 0; k < pixel_bytes; ++k)
        cur[k] = image__BYTECAST(raw[k] + (prior[k] >> 1));
      for (k = pixel_bytes; k < nk; ++k)
        cur[k] = image__BYTECAST(raw[k] + ((prior[k] + cur[k - pixel_bytes]) >> 1));
      break;
    case image__F_paeth:
      for (k = 0; k < pixel_bytes; ++k)
        cur[k] = image__BYTECAST(raw[k] + prior[k]); // prior[k] == image__paeth(0,prior[k],0)
      for (k = pixel_bytes; k < nk; ++k)
        cur[k] = image__BYTECAST(raw[k] + image__paeth(cur[k - pixel_bytes], prior[k], prior[k - pixel_bytes]));
      break;
    case image__F_avg_first:
      util_memcpy(cur, raw, pixel_bytes);
      for (k = pixel_bytes; k < nk; ++k)
        cur[k] = image__BYTECAST(raw[k] + (cur[k - pixel_bytes] >> 1));
      break;
    }

    raw += nk;

    // expand decoded bits in cur to dest, also adding an extra alpha channel if desired
    if (depth < 8) {
      ubyte scale = (color == 0) ? image__depth_scale_table[depth] : 1; // scale grayscale values to 0..255 range
      ubyte *in = cur;
      ubyte *out = dest;
      ubyte inb = 0;
      uint32 nsmp = x * imb->inf.chnl;

      // expand bits to bytes first
      if (depth == 4) {
        for (i = 0; i < nsmp; ++i) {
          if ((i & 1) == 0)
            inb = *in++;
          *out++ = scale * (inb >> 4);
          inb <<= 4;
        }
      } else if (depth == 2) {
        for (i = 0; i < nsmp; ++i) {
          if ((i & 3) == 0)
            inb = *in++;
          *out++ = scale * (inb >> 6);
          inb <<= 2;
        }
      } else {
        ASSERT(depth == 1);
        for (i = 0; i < nsmp; ++i) {
          if ((i & 7) == 0)
            inb = *in++;
          *out++ = scale * (inb >> 7);
          inb <<= 1;
        }
      }

      // insert alpha=255 values if desired
      if (imb->inf.chnl != out_n)
        image__create_png_alpha_expand8(dest, dest, x, img_n);
    } else if (depth == 8) {
      if (img_n == out_n)
        util_memcpy(dest, cur, x * img_n);
      else
        image__create_png_alpha_expand8(dest, cur, x, img_n);
    } else if (depth == 16) {
      // convert the image data from big-endian to platform-native
      ushrt *dest16 = (ushrt *)dest;
      uint32 nsmp = x * img_n;

      if (img_n == out_n) {
        for (i = 0; i < nsmp; ++i, ++dest16, cur += 2)
          *dest16 = (cur[0] << 8) | cur[1];
      } else {
        ASSERT(img_n + 1 == out_n);
        if (img_n == 1) {
          for (i = 0; i < x; ++i, dest16 += 2, cur += 2) {
            dest16[0] = (cur[0] << 8) | cur[1];
            dest16[1] = 0xffff;
          }
        } else {
          ASSERT(img_n == 3);
          for (i = 0; i < x; ++i, dest16 += 4, cur += 6) {
            dest16[0] = (cur[0] << 8) | cur[1];
            dest16[1] = (cur[2] << 8) | cur[3];
            dest16[2] = (cur[4] << 8) | cur[5];
            dest16[3] = 0xffff;
          }
        }
      }
    }
  }

  util_memfree(filter_buf);
  if (!all_ok)
    return 0;
  return 1;
}
static int image__compute_transparency(image__png *z, ubyte tc[3], int out_n) {
  image__context *s = z->s;
  uint32 i, pixel_count = s->img_x * s->img_y;
  ubyte *p = z->out;

  // compute color-based transparency, assuming we've
  // already got 255 as the alpha value in the output
  ASSERT(out_n == 2 || out_n == 4);

  if (out_n == 2) {
    for (i = 0; i < pixel_count; ++i) {
      p[1] = (p[0] == tc[0] ? 0 : 255);
      p += 2;
    }
  } else {
    for (i = 0; i < pixel_count; ++i) {
      if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
        p[3] = 0;
      p += 4;
    }
  }
  return 1;
}
static int image__compute_transparency16(image__png *z, ushrt tc[3], int out_n) {
  image__context *s = z->s;
  uint32 i, pixel_count = s->img_x * s->img_y;
  ushrt *p = (ushrt *)z->out;

  // compute color-based transparency, assuming we've
  // already got 65535 as the alpha value in the output
  ASSERT(out_n == 2 || out_n == 4);

  if (out_n == 2) {
    for (i = 0; i < pixel_count; ++i) {
      p[1] = (p[0] == tc[0] ? 0 : 65535);
      p += 2;
    }
  } else {
    for (i = 0; i < pixel_count; ++i) {
      if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
        p[3] = 0;
      p += 4;
    }
  }
  return 1;
}
static int image__expand_png_palette(image__png *a, ubyte *palette, int UNUSED_ARG(len), int pal_img_n) {
  uint32 i, pixel_count = a->s->img_x * a->s->img_y;
  ubyte *p = (ubyte *)image__malloc_mad2(pixel_count, pal_img_n, 0), *temp_out, *orig = a->out;
  if (!p)
    return stb_set_error("Out of memory");

  // between here and util_memfree(out) below, exitting would leak
  temp_out = p;

  if (pal_img_n == 3) {
    for (i = 0; i < pixel_count; ++i) {
      int n = orig[i] * 4;
      p[0] = palette[n];
      p[1] = palette[n + 1];
      p[2] = palette[n + 2];
      p += 3;
    }
  } else {
    for (i = 0; i < pixel_count; ++i) {
      int n = orig[i] * 4;
      p[0] = palette[n];
      p[1] = palette[n + 1];
      p[2] = palette[n + 2];
      p[3] = palette[n + 3];
      p += 4;
    }
  }
  util_memfree(a->out);
  a->out = temp_out;
  return 1;
}
typedef struct {
	iter idata_len, palette_len;
	ubyte *idata, *palette, *trnsId;
	ubyte compress, color, interlace;
	bool isIphone;
} image__png;
static int image__png_parse(image__context *s, image_info *inf, image__png *png) {
  uint32 chunk_len, crc;
  iter i, j;
  int ret = -1;
#define PNG_APPLY(P,V) if (!png) png->##P = (V)
#define PNG_ERROR(s, ...) do {\
  stb_set_error("PNG: " (s), __VA_ARGS__);\
  goto image__png_parse_end;\
} while (0)
#define PNG_CHEAD(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define PNG_CHEAD_ARC(a, b, c, d) case PNG_CHEAD(a,b,c,d): {\
	if (png && !png->idata) PNG_ERROR("%x invalid order", CAST(int32)PNG_CHEAD(a,b,c,d));\
  hash_crc32_appends(&crc, image__readtemp(s, chunk_len), chunk_len);\
} break
  if (util_memcmp(PNG_SIGNATURE, image__readtemp(s, 8), 8)) {
    image__rewind(s);
    return 0;
  }
  {
	  ubyte *temp = image_readtemp(s, 25);
	  // get IHDR must be first otherwise error
	  // 13 + IHDR in big endian order
	  if (util_memcmp("\0\0\0\rIHDR", temp, 8)) PNG_ERROR("bad header");
	  inf->w = *CAST(uint32*)(temp + 9);
	  inf->h = *CAST(uint32*)(temp + 13);
	  if (!inf->w || !inf->h || (inf->w > MAX_DIMENSIONS) || (inf->h > MAX_DIMENSIONS)) PNG_ERROR("Image size is too large");
	  inf->bpc = temp[17];
	  PNG_APPLY(color, temp[18])
#define PNG_TYPE(T,B) case (((T) << 8) | (B))
	  switch (*CAST(ushrt*)(temp + 17)) {
	    PNG_TYPE(0,16): PNG_TYPE(0,8):
	      inf->chnl = 1;
	      break;
	    PNG_TYPE(2,16): PNG_TYPE(2,8):
	    PNG_TYPE(3,8): 
	      inf->chnl = 3;
	      break;
	    PNG_TYPE(4,8): PNG_TYPE(4,16):
	      inf->chnl = 2;
	      break;
	    PNG_TYPE(6,8): PNG_TYPE(6,16):
	      inf->chnl = 4;
	      break;
	    PNG_TYPE(0,4): PNG_TYPE(0,2): PNG_TYPE(0,1):
	    PNG_TYPE(3,4): PNG_TYPE(3,2): PNG_TYPE(3,1):
	      PNG_ERROR("idk parser bit less than 8");
	    default:
	      PNG_ERROR("bad ctype");
	  }
#undef PNG_TYPE
	  if (temp[19] || temp[20] || (temp[21] > 1))
	    PNG_ERROR("bad parameter");
	  PNG_APPLY(compression, temp[19]);
	  PNG_APPLY(filter		 , temp[20]);
	  PNG_APPLY(interlace	 , temp[21]);
	  // crc validator
	  hash_crc32_start(&crc);
	  hash_crc32_appends(&crc, temp, temp + 4, 13);
	  hash_crc32_end(&crc);
	  // either validator or actual crc need to flip
	  crc = imath_flip32(crc);
	  if (util_memcmp(&crc, temp + 22, 4)) PNG_ERROR("invalid crc IHDR");
  }
  for (bool loop = true;loop;) {
    chunk_len = image__get32be(s);
    hash_crc32_start(&crc);
    hash_crc32_appends(&crc, image__readtemp(s, 4), 4);
    switch (s->buffer.ui) {
	    case PNG_CHEAD('P', 'L', 'T', 'E'):
	    	if (png) {
		      if (png->color == 2 || png->color == 6) {
		      	// not used to quatized sugested color! skip it
		      	hash_crc32_appends(&crc, image__readtemp(s, chunk_len), chunk_len);
		      } else if (png->idata || png->color != 3 || png->palette || (chunk_len > 256 * 3) || (chunk_len % 3)) {
		        PNG_ERROR("invalid PLTE chunk");
		      } else {
		      	png->palette_len = chunk_len / 3;
			      png->palette = CAST(ubyte*)util_malloc(png->palette_len);
			      for (i = 0; i < (png->palette_len * 4); i += 4) {
				      if (3 != image__read(s, png->palette + i, 3))
				        PNG_ERROR("fail read palette chunk data at %zu", i);
			      	hash_crc32_appends(&crc, png->palette + i, 3);
				      png->palette[i + 3] = 0xff;
			      }
		      }
	    	} else hash_crc32_appends(&crc, image__readtemp(s, chunk_len), chunk_len);
	      break;
	    case PNG_CHEAD('t', 'R', 'N', 'S'):
      	++inf->chnl;
      	if (png) {
	      	if (png->idata) PNG_ERROR("tRNS after IDAT");
	      	switch (png->color) {
		      	case 0:
		        	png->trnsId = CAST(ubyte*)util_malloc(2);
		        	image__read(s, png->trnsId, 2);
			      	hash_crc32_appends(&crc, png->trnsId, 2);
		      		break;
		      	case 2:
		        	png->trnsId = CAST(ubyte*)util_malloc(6);
		        	image__read(s, png->trnsId, 6);
			      	hash_crc32_appends(&crc, png->trnsId, 6);
		      		break;
		      	case 3:
		        	if (chunk_len > png->palette_len)
		        		PNG_ERROR("tRNS provide more transparent than palette");
		        	for (i = 0; i < (chunk_len * 4); i += 4) {
		        		png->palette[i + 3] = image__get8(s);
			      		hash_crc32_append(&crc, png->palette[i + 3]);
		        	}
		      		break;
		      	default:
		      		PNG_ERROR("prohibit tRNS on color type 4 & 6");
	      	}
        } else hash_crc32_appends(&crc, image__readtemp(s, chunk_len), chunk_len);
	      break;
	    case PNG_CHEAD('I', 'D', 'A', 'T'):
	    	if (png) {
		      if ((png->color == 3) && !png->palette) PNG_ERROR("PLTE not defined for indexed color type");
		      png->idata = CAST(ubyte*)util_realloc(png->idata, png->idata_len + chunk_len);
		      if (chunk_len != image__read(s, png->idata + png->idata_len, chunk_len))
		        PNG_ERROR("fail to allocate idata %zu bytes", chunk_len + png->idata_len);
		      hash_crc32_appends(&crc, image__readtemp(s, chunk_len), chunk_len);
		      png->idata_len += chunk_len;
	    	} else hash_crc32_appends(&crc, image__readtemp(s, chunk_len), chunk_len);
	      break;
	    case PNG_CHEAD('I', 'E', 'N', 'D'):
	      if (png && (!(png->idata) || (png->color == 3 && !(png->palette))))
	        PNG_ERROR("data chunk is lost");
	      loop = false;
	      break;
	    // optional case (archilary)
	    case PNG_CHEAD_ARC('b','K','G','D');
	    case PNG_CHEAD_ARC('c','H','R','M');
	    case PNG_CHEAD_ARC('g','A','M','A');
	    case PNG_CHEAD_ARC('s','B','I','T');
	    case PNG_CHEAD_ARC('h','I','S','T');
	    case PNG_CHEAD_ARC('p','H','Y','s');
	    // skip
	    case PNG_CHEAD('C', 'g', 'B', 'I'):
	      PNG_APPLY(isIphone, true);
	    case PNG_CHEAD('t','I','M','E'):
	    case PNG_CHEAD('t','E','X','t'):
	    case PNG_CHEAD('z','T','X','t'):
	      hash_crc32_appends(&crc, image__readtemp(s, chunk_len), chunk_len);
	      break;
	    // error case
	    case PNG_CHEAD('I', 'H', 'D', 'R'):
	    default:
	      PNG_ERROR("invalid chunk");
    }
    hash_crc32_end(&crc);
    if (crc != image__get32be(s)) PNG_ERROR("crc validator wrong!");
  }
  // file should be clean
  ret = 1;
image__png_parse_end:
	if (png && ret == -1) {
	  if (png->palette) util_memfree(palette);
	  if (png->idata) util_memfree(idata);
	  if (png->trnsId) util_memfree(trbsId);
	}
  return ret;
#undef PNG_CHEAD_ARC
#undef PNG_CHEAD
#undef PNG_APPLY
#undef PNG_ERROR
}
static int image__png_info(image__context *s, image_info *inf) {
	return image__png_parse(s, inf, NULL);
}
static int image__png_load(image__context *s, image_bitmap *imb) {
  image__png png;
	int ret = image__png_parse(s, &(imb->inf), &png);
  if (ret < 1) return ret;
  ret = -1;
#define PNG_ERROR(...) stb_set_error(__VA_ARGS__); goto image__png_load_end
  // decode zlib
  {
  	// initial guess for decoded data size to avoid unnecessary reallocs
	  uint32 bpl = (imb->inf.w * imb->inf.bpc + 7) / 8; // bytes per line, per component
	  iter raw_len = bpl * imb->inf.h * imb->inf.chnl /* pixels */ + imb->inf.h /* filter */;
	  ubyte *expanded = CAST(ubyte*)zlib_decode_malloc_guesssize_headerflag(CAST(byte*)png.idata, png.idata_len, &raw_len, !png.isIphone);
	  if (!expanded) goto image__png_load_end;
	  util_memfree(png.idata);
	  png.idata = expanded;
	  png.idata_len = raw_len;
  }
  // create image
  {
	  ubyte *image_data = png.idata;
	  int image_data_len = png.idata_len;
	  int x,y;
	  iter out_bytes = imb->inf.chnl * imb->inf.bpc/8;
	  if (!png->interlaced) return image__create_png_image_raw(imb, image_data, image_data_len, imb->inf.chnl, imb->inf.w, imb->inf.h, imb->inf.bpc, png.color);
	  // de-interlacing
	  ubyte *final = CAST(ubyte *)image__malloc_mad4(imb->inf.w, imb->inf.h, imb->inf.chnl, imb->inf.bpc, 0);
	  if (!final) PNG_ERROR("Out of memory");
	  for (iter p = 0; p < 7; ++p) {
	    static const int orig[] = {0, 0, 4, 0, 2, 0, 1, 0};
	    static const int spc[]  = {8, 8, 8, 4, 4, 2, 2, 1};
	    x = (imb->inf.w - orig[p + 1] + spc[p + 1] - 1) / spc[p + 1];
	    y = (imb->inf.h - orig[  p  ] + spc[  p  ] - 1) / spc[  p  ];
	    if (x && y) {
	      uint32 img_len = ((((imb->inf.chnl * x * imb->inf.bpc) + 7) >> 3) + 1) * y;
	      if (!image__create_png_image_raw(imb, image_data, image_data_len, imb->inf.chnl, x, y, imb->inf.bpc, png.color)) {
	        util_memfree(final);
	        return 0;
	      }
	      for (j = 0; j < y; ++j) {
	        for (i = 0; i < x; ++i) {
	          int out_y = j * spc[  p  ] + orig[  p  ];
	          int out_x = i * spc[p + 1] + orig[p + 1];
	          iter v = out_y * imb->inf.w * out_bytes + out_x * out_bytes;
	          iter w = (j * x + i) * out_bytes;
	          util_memcpy(final + v, imb->data + w, out_bytes);
	        }
	      }
	      util_memfree(imb->data);
	      image_data += img_len;
	      image_data_len -= img_len;
	    }
	  }
	  a->out = final;
  }
  if (has_trans) {
    if (z->depth == 16) {
      if (!image__compute_transparency16(z, tc16, s->img_out_n))
        return 0;
    } else {
      if (!image__compute_transparency(z, tc, s->img_out_n))
        return 0;
    }
  }
  if (is_iphone && imb->inf.chnl > 2) {
  	const iter bypc = imb->inf.bpc / 8;
  	const iter bpp = imb->inf.chnl * bytes;
	  for (i = 0; i < (imb->inf.w * imb->inf.h * bpp); i += bpp) {
	    util_memswap(imb->data + i, imb->data + i + (2 * bypc), bypc); // bgr to rgb
#ifdef UNPREMULTIPLY
	  	if (imb->inf.chnl == 4)
		  	for (j = 0; j < (3 * bypc); j += bypc)
		  		imb->data[i + j] = (imb->data[i + j] * 255 + (imb->data[i + 3] >> 1)) / imb->data[i + 3];
#endif // UNPREMULTIPLY
  	}
  }
  if (pal_img_n) {
    // pal_img_n == 3 or 4
    s->img_n = pal_img_n; // record the actual colors we had
    s->img_out_n = pal_img_n;
    if (req_comp >= 3)
      s->img_out_n = req_comp;
    if (!image__expand_png_palette(z, palette, pal_len, s->img_out_n))
      return 0;
  }
  ret = 1;
image__png_load_end:
	if (png->transId) util_memfree(png->transId);
	if (png->palette) util_memfree(png->palette);
	if (png->idata) util_memfree(png->idata);
  return ret;
#undef PNG_ERROR
}

// Microsoft/Windows BMP image

static int image__bmp_test_raw(image__context *s) {
  int r;
  int sz;
  if (image__get8(s) != 'B')
    return 0;
  if (image__get8(s) != 'M')
    return 0;
  image__get32le(s); // discard 32bit filesize
  image__get16le(s); // discard 16bit reserved
  image__get16le(s); // discard 16bit reserved
  image__get32le(s); // discard 32bit data offset
  sz = image__get32le(s);
  r = (sz == 12 || sz == 40 || sz == 56 || sz == 108 || sz == 124);
  return r;
}
static int image__bmp_test(image__context *s) {
  int r = image__bmp_test_raw(s);
  image__rewind(s);
  return r;
}
// returns 0..31 for the highest set bit
static int image__high_bit(uint z) {
  int n = 0;
  if (z == 0)
    return -1;
  if (z >= 0x10000) {
    n += 16;
    z >>= 16;
  }
  if (z >= 0x00100) {
    n += 8;
    z >>= 8;
  }
  if (z >= 0x00010) {
    n += 4;
    z >>= 4;
  }
  if (z >= 0x00004) {
    n += 2;
    z >>= 2;
  }
  if (z >= 0x00002) {
    n += 1; /* >>=  1;*/
  }
  return n;
}
static int image__bitcount(uint a) {
  a = (a & 0x55555555) + ((a >> 1) & 0x55555555); // max 2
  a = (a & 0x33333333) + ((a >> 2) & 0x33333333); // max 4
  a = (a + (a >> 4)) & 0x0f0f0f0f;                // max 8 per 4, now 8 bits
  a = (a + (a >> 8));                             // max 16 per 8 bits
  a = (a + (a >> 16));                            // max 32 per 8 bits
  return a & 0xff;
}
// extract an arbitrarily-aligned N-bit value (N=bits)
// from v, and then make it 8-bits long and fractionally
// extend it to full full range.
static int image__shiftsigned(uint v, int shift, int bits) {
  static uint mul_table[9] = {
    0,
    0xff /*0b11111111*/,
    0x55 /*0b01010101*/,
    0x49 /*0b01001001*/,
    0x11 /*0b00010001*/,
    0x21 /*0b00100001*/,
    0x41 /*0b01000001*/,
    0x81 /*0b10000001*/,
    0x01 /*0b00000001*/,
  };
  static uint shift_table[9] = {
    0,
    0,
    0,
    1,
    0,
    2,
    4,
    6,
    0,
  };
  if (shift < 0)
    v <<= -shift;
  else
    v >>= shift;
  ASSERT(v < 256);
  v >>= (8 - bits);
  ASSERT(bits >= 0 && bits <= 8);
  return (int)((unsigned)v * mul_table[bits]) >> shift_table[bits];
}
typedef struct {
  int bpp, offset, hsz;
  uint mr, mg, mb, ma, all_a;
  int extra_read;
} image__bmp_data;
static int image__bmp_set_mask_defaults(image__bmp_data *info, int compress) {
  // BI_BITFIELDS specifies masks explicitly, don't override
  if (compress == 3)
    return 1;

  if (compress == 0) {
    if (info->bpp == 16) {
      info->mr = 31u << 10;
      info->mg = 31u << 5;
      info->mb = 31u << 0;
    } else if (info->bpp == 32) {
      info->mr = 0xffu << 16;
      info->mg = 0xffu << 8;
      info->mb = 0xffu << 0;
      info->ma = 0xffu << 24;
      info->all_a = 0; // if all_a is 0 at end, then we loaded alpha channel but it was all 0
    } else {
      // otherwise, use defaults, which is all-0
      info->mr = info->mg = info->mb = info->ma = 0;
    }
    return 1;
  }
  return 0; // error
}
static void *image__bmp_parse_header(image__context *s, image__bmp_data *info) {
  int hsz;
  if (image__get8(s) != 'B' || image__get8(s) != 'M')
    return (ubyte *)(iter)stb_set_error("not BMP : Corrupt BMP");
  image__get32le(s); // discard filesize
  image__get16le(s); // discard reserved
  image__get16le(s); // discard reserved
  info->offset = image__get32le(s);
  info->hsz = hsz = image__get32le(s);
  info->mr = info->mg = info->mb = info->ma = 0;
  info->extra_read = 14;

  if (info->offset < 0)
    return (ubyte *)(iter)stb_set_error("bad BMP : bad BMP");

  if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108 && hsz != 124)
    return (ubyte *)(iter)stb_set_error("unknown BMP : BMP type not supported: unknown");
  if (hsz == 12) {
    s->img_x = image__get16le(s);
    s->img_y = image__get16le(s);
  } else {
    s->img_x = image__get32le(s);
    s->img_y = image__get32le(s);
  }
  if (image__get16le(s) != 1)
    return (ubyte *)(iter)stb_set_error("bad BMP : bad BMP");
  info->bpp = image__get16le(s);
  if (hsz != 12) {
    int compress = image__get32le(s);
    if (compress == 1 || compress == 2)
      return (ubyte *)(iter)stb_set_error("BMP RLE : BMP type not supported: RLE");
    if (compress >= 4)
      return (ubyte *)(iter)stb_set_error("BMP JPEG/PNG : BMP type not supported: unsupported compression"); // this includes PNG/JPEG modes
    if (compress == 3 && info->bpp != 16 && info->bpp != 32)
      return (ubyte *)(iter)stb_set_error("bad BMP : bad BMP"); // bitfields requires 16 or 32 bits/pixel
    image__get32le(s);                                                 // discard sizeof
    image__get32le(s);                                                 // discard hres
    image__get32le(s);                                                 // discard vres
    image__get32le(s);                                                 // discard colorsused
    image__get32le(s);                                                 // discard max important
    if (hsz == 40 || hsz == 56) {
      if (hsz == 56) {
        image__get32le(s);
        image__get32le(s);
        image__get32le(s);
        image__get32le(s);
      }
      if (info->bpp == 16 || info->bpp == 32) {
        if (compress == 0) {
          image__bmp_set_mask_defaults(info, compress);
        } else if (compress == 3) {
          info->mr = image__get32le(s);
          info->mg = image__get32le(s);
          info->mb = image__get32le(s);
          info->extra_read += 12;
          // not documented, but generated by photoshop and handled by mspaint
          if (info->mr == info->mg && info->mg == info->mb) {
            // ?!?!?
            return (ubyte *)(iter)stb_set_error("bad BMP : bad BMP");
          }
        } else
          return (ubyte *)(iter)stb_set_error("bad BMP : bad BMP");
      }
    } else {
      // V4/V5 header
      int i;
      if (hsz != 108 && hsz != 124)
        return (ubyte *)(iter)stb_set_error("bad BMP : bad BMP");
      info->mr = image__get32le(s);
      info->mg = image__get32le(s);
      info->mb = image__get32le(s);
      info->ma = image__get32le(s);
      if (compress != 3) // override mr/mg/mb unless in BI_BITFIELDS mode, as per docs
        image__bmp_set_mask_defaults(info, compress);
      image__get32le(s); // discard color space
      for (i = 0; i < 12; ++i)
        image__get32le(s); // discard color space parameters
      if (hsz == 124) {
        image__get32le(s); // discard rendering intent
        image__get32le(s); // discard offset of profile data
        image__get32le(s); // discard size of profile data
        image__get32le(s); // discard reserved
      }
    }
  }
  return (void *)1;
}
static void *image__bmp_load(image__context *s, int *x, int *y, int *comp, int req_comp, image__result_info *ri) {
  ubyte *out;
  uint mr = 0, mg = 0, mb = 0, ma = 0, all_a;
  ubyte pal[256][4];
  int psize = 0, i, j, width;
  int flip_vertically, pad, target;
  image__bmp_data info;
  UNUSED(ri);

  info.all_a = 255;
  if (image__bmp_parse_header(s, &info) == NULL)
    return NULL; // error code already set

  flip_vertically = ((int)s->img_y) > 0;
  s->img_y = abs((int)s->img_y);

  if (s->img_y > MAX_DIMENSIONS)
    return (ubyte *)(iter)stb_set_error("Very large image (corrupt?)");
  if (s->img_x > MAX_DIMENSIONS)
    return (ubyte *)(iter)stb_set_error("Very large image (corrupt?)");

  mr = info.mr;
  mg = info.mg;
  mb = info.mb;
  ma = info.ma;
  all_a = info.all_a;

  if (info.hsz == 12) {
    if (info.bpp < 24)
      psize = (info.offset - info.extra_read - 24) / 3;
  } else {
    if (info.bpp < 16)
      psize = (info.offset - info.extra_read - info.hsz) >> 2;
  }
  if (psize == 0) {
    // accept some number of extra bytes after the header, but if the offset points either to before
    // the header ends or implies a large amount of extra data, reject the file as malformed
    int bytes_read_so_far = s->callback_already_read + (int)(s->img_buffer - s->img_buffer_original);
    int header_limit = 1024;        // max we actually read is below 256 bytes currently.
    int extra_data_limit = 256 * 4; // what ordinarily goes here is a palette; 256 entries*4 bytes is its max size.
    if (bytes_read_so_far <= 0 || bytes_read_so_far > header_limit) {
      return (ubyte *)(iter)stb_set_error("bad header : Corrupt BMP");
    }
    // we established that bytes_read_so_far is positive and sensible.
    // the first half of this test rejects offsets that are either too small positives, or
    // negative, and guarantees that info.offset >= bytes_read_so_far > 0. this in turn
    // ensures the number computed in the second half of the test can't overflow.
    if (info.offset < bytes_read_so_far || info.offset - bytes_read_so_far > extra_data_limit) {
      return (ubyte *)(iter)stb_set_error("bad offset : Corrupt BMP");
    } else {
      image__skip(s, info.offset - bytes_read_so_far);
    }
  }

  if (info.bpp == 24 && ma == 0xff000000)
    s->img_n = 3;
  else
    s->img_n = ma ? 4 : 3;
  if (req_comp && req_comp >= 3) // we can directly decode 3 or 4
    target = req_comp;
  else
    target = s->img_n; // if they want monochrome, we'll post-convert

  // sanity-check size
  if (!image__validm3ad(target, s->img_x, s->img_y, 0))
    return (ubyte *)(iter)stb_set_error("Corrupt BMP");

  out = (ubyte *)image__malloc_mad3(target, s->img_x, s->img_y, 0);
  if (!out)
    return (ubyte *)(iter)stb_set_error("outofmem : Out of memory");
  if (info.bpp < 16) {
    int z = 0;
    if (psize == 0 || psize > 256) {
      util_memfree(out);
      return (ubyte *)(iter)stb_set_error("invalid : Corrupt BMP");
    }
    for (i = 0; i < psize; ++i) {
      pal[i][2] = image__get8(s);
      pal[i][1] = image__get8(s);
      pal[i][0] = image__get8(s);
      if (info.hsz != 12)
        image__get8(s);
      pal[i][3] = 255;
    }
    image__skip(s, info.offset - info.extra_read - info.hsz - psize * (info.hsz == 12 ? 3 : 4));
    if (info.bpp == 1)
      width = (s->img_x + 7) >> 3;
    else if (info.bpp == 4)
      width = (s->img_x + 1) >> 1;
    else if (info.bpp == 8)
      width = s->img_x;
    else {
      util_memfree(out);
      return (ubyte *)(iter)stb_set_error("bad bpp : Corrupt BMP");
    }
    pad = (-width) & 3;
    if (info.bpp == 1) {
      for (j = 0; j < (int)s->img_y; ++j) {
        int bit_offset = 7, v = image__get8(s);
        for (i = 0; i < (int)s->img_x; ++i) {
          int color = (v >> bit_offset) & 0x1;
          out[z++] = pal[color][0];
          out[z++] = pal[color][1];
          out[z++] = pal[color][2];
          if (target == 4)
            out[z++] = 255;
          if (i + 1 == (int)s->img_x)
            break;
          if ((--bit_offset) < 0) {
            bit_offset = 7;
            v = image__get8(s);
          }
        }
        image__skip(s, pad);
      }
    } else {
      for (j = 0; j < (int)s->img_y; ++j) {
        for (i = 0; i < (int)s->img_x; i += 2) {
          int v = image__get8(s), v2 = 0;
          if (info.bpp == 4) {
            v2 = v & 15;
            v >>= 4;
          }
          out[z++] = pal[v][0];
          out[z++] = pal[v][1];
          out[z++] = pal[v][2];
          if (target == 4)
            out[z++] = 255;
          if (i + 1 == (int)s->img_x)
            break;
          v = (info.bpp == 8) ? image__get8(s) : v2;
          out[z++] = pal[v][0];
          out[z++] = pal[v][1];
          out[z++] = pal[v][2];
          if (target == 4)
            out[z++] = 255;
        }
        image__skip(s, pad);
      }
    }
  } else {
    int rshift = 0, gshift = 0, bshift = 0, ashift = 0, rcount = 0, gcount = 0, bcount = 0, acount = 0;
    int z = 0;
    int easy = 0;
    image__skip(s, info.offset - info.extra_read - info.hsz);
    if (info.bpp == 24)
      width = 3 * s->img_x;
    else if (info.bpp == 16)
      width = 2 * s->img_x;
    else /* bpp = 32 and pad = 0 */
      width = 0;
    pad = (-width) & 3;
    if (info.bpp == 24) {
      easy = 1;
    } else if (info.bpp == 32) {
      if (mb == 0xff && mg == 0xff00 && mr == 0x00ff0000 && ma == 0xff000000)
        easy = 2;
    }
    if (!easy) {
      if (!mr || !mg || !mb) {
        util_memfree(out);
        return (ubyte *)(iter)stb_set_error("bad masks : Corrupt BMP");
      }
      // right shift amt to put high bit in position #7
      rshift = image__high_bit(mr) - 7;
      rcount = image__bitcount(mr);
      gshift = image__high_bit(mg) - 7;
      gcount = image__bitcount(mg);
      bshift = image__high_bit(mb) - 7;
      bcount = image__bitcount(mb);
      ashift = image__high_bit(ma) - 7;
      acount = image__bitcount(ma);
      if (rcount > 8 || gcount > 8 || bcount > 8 || acount > 8) {
        util_memfree(out);
        return (ubyte *)(iter)stb_set_error("bad masks : Corrupt BMP");
      }
    }
    for (j = 0; j < (int)s->img_y; ++j) {
      if (easy) {
        for (i = 0; i < (int)s->img_x; ++i) {
          ubyte a;
          out[z + 2] = image__get8(s);
          out[z + 1] = image__get8(s);
          out[z + 0] = image__get8(s);
          z += 3;
          a = (easy == 2 ? image__get8(s) : 255);
          all_a |= a;
          if (target == 4)
            out[z++] = a;
        }
      } else {
        int bpp = info.bpp;
        for (i = 0; i < (int)s->img_x; ++i) {
          uint32 v = (bpp == 16 ? (uint32)image__get16le(s) : image__get32le(s));
          uint a;
          out[z++] = image__BYTECAST(image__shiftsigned(v & mr, rshift, rcount));
          out[z++] = image__BYTECAST(image__shiftsigned(v & mg, gshift, gcount));
          out[z++] = image__BYTECAST(image__shiftsigned(v & mb, bshift, bcount));
          a = (ma ? image__shiftsigned(v & ma, ashift, acount) : 255);
          all_a |= a;
          if (target == 4)
            out[z++] = image__BYTECAST(a);
        }
      }
      image__skip(s, pad);
    }
  }

  // if alpha channel is all 0s, replace with all 255s
  if (target == 4 && all_a == 0)
    for (i = 4 * s->img_x * s->img_y - 1; i >= 0; i -= 4)
      out[i] = 255;

  if (flip_vertically) {
    ubyte t;
    for (j = 0; j < (int)s->img_y >> 1; ++j) {
      ubyte *p1 = out + j * s->img_x * target;
      ubyte *p2 = out + (s->img_y - 1 - j) * s->img_x * target;
      for (i = 0; i < (int)s->img_x * target; ++i) {
        t = p1[i];
        p1[i] = p2[i];
        p2[i] = t;
      }
    }
  }

  if (req_comp && req_comp != target) {
    out = image__convert_format(out, target, req_comp, s->img_x, s->img_y);
    if (out == NULL)
      return out; // image__convert_format frees input on failure
  }

  *x = s->img_x;
  *y = s->img_y;
  if (comp)
    *comp = s->img_n;
  return out;
}

// Targa Truevision - TGA
// by Jonathan Dummer

static int image__tga_info(image__context *s, image_info *inf) {
  //  read in the TGA header stuff
  ubyte *temp = image__readtemp(s, 17);
  ubyte tga_indexed = temp[1];
  int tga_image_type = temp[2];
  int tga_palette_bits = temp[7];
  inf->w = *CAST(ushrt*)(temp + 12);
  inf->h = *CAST(ushrt*)(temp + 14);
  ubyte bits_per_pixel = temp[16];
  if ((tga_indexed > 1) || //   only RGB or indexed allowed
      (tga_indexed && (
        (tga_image_type != 1 && tga_image_type != 9) ||
        (tga_palette_bits != 8 && tga_palette_bits != 15 && tga_palette_bits != 16 && tga_palette_bits != 24 && tga_palette_bits != 32) ||
        (bits_per_pixel != 8 && bits_per_pixel != 16)
      ) ||
      (tga_image_type != 2 && tga_image_type != 3 && tga_image_type != 10 && tga_image_type != 11) ||
      (bits_per_pixel != 8 && bits_per_pixel != 15 && bits_per_pixel != 16 && bits_per_pixel != 24 && bits_per_pixel != 32) ||
      (inf->w > MAX_DIMENSIONS) ||
      (inf->h > MAX_DIMENSIONS)) {
    image__rewind(s);
    return 0;         // colortype 1 demands image type 1 or 9
  }
  // only RGB or RGBA (incl. 16bit) or grey allowed
  switch (tga_indexed ? tga_palette_bits : bits_per_pixel) {
    case 8: inf->chnl = 1; break;
    case 16:
      if (tga_indexed && tga_image_type == 3) inf->chnl = 2;
    case 15:
    case 24: inf->chnl = 3; break;
    case 32: inf->chnl = 4; break;
    default:
      image__rewind(s);
      return 0;
  }
  inf->bpc = 8; // stored channel info into 8 bits
  return 1;
}
// read 16bit value and convert to 24bit RGB
static void image__tga_read_rgb16(image__context *s, ubyte *out) {
  ushrt px = CAST(ushrt)image__get16le(s);
  ushrt fiveBitMask = 31;
  // we have 3 channels with 5bits each
  int r = (px >> 10) & fiveBitMask;
  int g = (px >> 5) & fiveBitMask;
  int b = px & fiveBitMask;
  // Note that this saves the data in RGB(A) order, so it doesn't need to be swapped later
  out[0] = (ubyte)((r * 255) / 31);
  out[1] = (ubyte)((g * 255) / 31);
  out[2] = (ubyte)((b * 255) / 31);

  // some people claim that the most significant bit might be used for alpha
  // (possibly if an alpha-bit is set in the "image descriptor byte")
  // but that only made 16bit test images completely translucent..
  // so let's treat all 15 and 16bit TGAs as RGB with no alpha.
}
static int image__tga_load(image__context *s, image_bitmap *imb) {
  int ret = image__tga_info(s, &(imb->data));
  if (ret < 1) return ret;
  //  read in the TGA header stuff
  // temporary by tga info 17 bytes
  ubyte *temp = s->buffer.ub;
  ubyte tga_offset = temp[0];
  ubyte tga_indexed = temp[1];
  ubyte tga_image_type = temp[2];
  int tga_palette_start = *CAST(ushrt*)(temp + 3);
  int tga_palette_len = *CAST(ushrt*)(temp + 5);
  ubyte tga_palette_bits = temp[7];
  // int tga_x_origin = *CAST(ushrt*)(temp + 8);
  // int tga_y_origin = *CAST(ushrt)(temp + 10);
  ubyte bits_per_pixel = temp[16];
  int tga_inverted = image__get8(s);
  // int tga_alpha_bits = tga_inverted & 15; // the 4 lowest bits - unused (useless?)
  tga_inverted = 1 - ((tga_inverted >> 5) & 1);
  //   do a tiny bit of precessing
  bool tga_is_RLE = false;
  if (tga_image_type >= 8) {
    tga_image_type -= 8;
    tga_is_RLE = true;
  }
  int RLE_count = 0;
  int RLE_repeating = 0;
  bool read_next_pixel = true;
  //   If I'm paletted, then I'll use the number of bits from the palette
  bool tga_rgb16 = (tga_indexed ? tga_palette_bits : bits_per_pixel) == 15;
  imb->data = CAST(ubyte *)image__malloc_mad3(bitmap->data.w, bitmap->data.h, imb->data.chnl, 0);
  if (!imb->data) {
    stb_set_error("outofmem : Out of memory");
    return -1;
  }
  ubyte *tga_palette = NULL;
  // processing data
  // skip to the data's starting position (offset usually = 0)
  image__skip(s, tga_offset);
  iter i, j;
  ubyte raw_data[4] = {0};
  if (!tga_indexed && !tga_is_RLE && !tga_rgb16) {
    for (i = 0; i < bitmap->data.h; ++i) {
      int row = tga_inverted ? bitmap->data.h - i - 1 : i;
      ubyte *tga_row = imb->data + row * bitmap->data.w * imb->data.chnl;
      image__read(s, tga_row, bitmap->data.w * imb->data.chnl);
    }
  } else {
    //   do I need to load a palette?
    if (tga_indexed) {
      if (tga_palette_len == 0) { /* you have to have at least one entry! */
        util_memfree(imb->data);
        stb_set_error("bad palette : Corrupt TGA");
        return -1;
      }
      //   any data to skip? (offset usually = 0)
      image__skip(s, tga_palette_start);
      //   load the palette
      tga_palette = CAST(ubyte *)image__malloc_mad2(tga_palette_len, imb->data.chnl, 0);
      if (!tga_palette) {
        util_memfree(imb->data);
        stb_set_error("outofmem : Out of memory");
        return -1;
      }
      if (tga_rgb16) {
        ubyte *pal_entry = tga_palette;
        ASSERT(imb->data.chnl == 3);
        for (i = 0; i < tga_palette_len; ++i) {
          image__tga_read_rgb16(s, pal_entry);
          pal_entry += imb->data.chnl;
        }
      } else if (!image__read(s, tga_palette, tga_palette_len * imb->data.chnl)) {
        util_memfree(imb->data);
        util_memfree(tga_palette);
        stb_set_error("bad palette : Corrupt TGA");
        return -1;
      }
    }
    //   load the data
    for (i = 0; i < bitmap->data.w * bitmap->data.h; ++i) {
      //   if I'm in RLE mode, do I need to get a RLE png chunk?
      if (tga_is_RLE) {
        if (RLE_count == 0) {
          //   yep, get the next byte as a RLE command
          int RLE_cmd = image__get8(s);
          RLE_count = 1 + (RLE_cmd & 127);
          RLE_repeating = RLE_cmd >> 7;
          read_next_pixel = true;
        } else if (!RLE_repeating) {
          read_next_pixel = true;
        }
      } else {
        read_next_pixel = true;
      }
      //   OK, if I need to read a pixel, do it now
      if (read_next_pixel) {
        //   load however much data we did have
        if (tga_indexed) {
          // read in index, then perform the lookup
          int pal_idx = (bits_per_pixel == 8) ? image__get8(s) : image__get16le(s);
          if (pal_idx >= tga_palette_len) {
            // invalid index
            pal_idx = 0;
          }
          pal_idx *= imb->data.chnl;
          util_memcpy(imb->data + (i * imb->data.chnl), tga_palette + pal_idx, imb->data.chnl);
        } else if (tga_rgb16) {
          ASSERT(imb->data.chnl == 3);
          image__tga_read_rgb16(s, imb->data + (i * imb->data.chnl));
        } else {
          image__read(s, imb->data + (i * imb->data.chnl), imb->data.chnl);
        }
        //   clear the reading flag for the next pixel
        read_next_pixel = false;
      } // end of reading a pixel
      //   in case we're in RLE mode, keep counting down
      --RLE_count;
    }
    //   do I need to invert the image?
    if (tga_inverted) {
      for (j = 0; j * 2 < bitmap->data.h; ++j) {
        int index1 = j * bitmap->data.w * imb->data.chnl;
        int index2 = (bitmap->data.h - 1 - j) * bitmap->data.w * imb->data.chnl;
        for (i = bitmap->data.w * imb->data.chnl; i > 0; --i) {
          ubyte temp = imb->data[index1];
          imb->data[index1] = imb->data[index2];
          imb->data[index2] = temp;
          ++index1;
          ++index2;
        }
      }
    }
    //   clear my palette, if I had one
    if (tga_palette != NULL) {
      util_memfree(tga_palette);
    }
  }

  // swap RGB - if the source data was RGB16, it already is in the right order
  if (imb->data.chnl >= 3 && !tga_rgb16) {
    ubyte *tga_pixel = imb->data;
    for (i = 0; i < bitmap->data.w * bitmap->data.h; ++i) {
      util_memflip(tga_pixel, 3);
      tga_pixel += imb->data.chnl;
    }
  }
  //   the things I do to get rid of an error message, and yet keep
  //   Microsoft's C compilers happy... [8^(
  tga_palette_start = tga_palette_len = tga_palette_bits = 0;
  UNUSED(tga_palette_start);
  //   OK, done
  return 1;
}

// *************************************************************************************************
// Photoshop PSD loader -- PD by Thatcher Ulrich, integration by Nicolas Schulz, tweaked by STB

static int image__psd_test(image__context *s) {
  int r = (image__get32be(s) == 0x38425053);
  image__rewind(s);
  return r;
}
static int image__psd_decode_rle(image__context *s, ubyte *p, int pixelCount) {
  int count, nleft, len;

  count = 0;
  while ((nleft = pixelCount - count) > 0) {
    len = image__get8(s);
    if (len == 128) {
      // No-op.
    } else if (len < 128) {
      // Copy next len+1 bytes literally.
      len++;
      if (len > nleft)
        return 0; // corrupt data
      count += len;
      while (len) {
        *p = image__get8(s);
        p += 4;
        len--;
      }
    } else if (len > 128) {
      ubyte val;
      // Next -len+1 bytes in the dest are replicated from next source byte.
      // (Interpret len as a negative 8-bit int.)
      len = 257 - len;
      if (len > nleft)
        return 0; // corrupt data
      val = image__get8(s);
      count += len;
      while (len) {
        *p = val;
        p += 4;
        len--;
      }
    }
  }

  return 1;
}
static void *image__psd_load(image__context *s, int *x, int *y, int *comp, int req_comp, image__result_info *ri, int bpc) {
  int pixelCount;
  int channelCount, compression;
  int channel, i;
  int bitdepth;
  int w, h;
  ubyte *out;
  UNUSED(ri);

  // Check identifier
  if (image__get32be(s) != 0x38425053) // "8BPS"
    return (ubyte *)(iter)stb_set_error("not PSD : Corrupt PSD image");

  // Check file type version.
  if (image__get16be(s) != 1)
    return (ubyte *)(iter)stb_set_error("wrong version : Unsupported version of PSD image");

  // Skip 6 reserved bytes.
  image__skip(s, 6);

  // Read the number of channels (R, G, B, A, etc).
  channelCount = image__get16be(s);
  if (channelCount < 0 || channelCount > 16)
    return (ubyte *)(iter)stb_set_error("wrong channel count : Unsupported number of channels in PSD image");

  // Read the rows and columns of the image.
  h = image__get32be(s);
  w = image__get32be(s);

  if (h > MAX_DIMENSIONS)
    return (ubyte *)(iter)stb_set_error("Very large image (corrupt?)");
  if (w > MAX_DIMENSIONS)
    return (ubyte *)(iter)stb_set_error("Very large image (corrupt?)");

  // Make sure the depth is 8 bits.
  bitdepth = image__get16be(s);
  if (bitdepth != 8 && bitdepth != 16)
    return (ubyte *)(iter)stb_set_error("unsupported bit depth : PSD bit depth is not 8 or 16 bit");

  // Make sure the color mode is RGB.
  // Valid options are:
  //   0: Bitmap
  //   1: Grayscale
  //   2: Indexed color
  //   3: RGB color
  //   4: CMYK color
  //   7: Multichannel
  //   8: Duotone
  //   9: Lab color
  if (image__get16be(s) != 3)
    return (ubyte *)(iter)stb_set_error("wrong color format : PSD is not in RGB color format");

  // Skip the Mode Data.  (It's the palette for indexed color; other info for other modes.)
  image__skip(s, image__get32be(s));

  // Skip the image resources.  (resolution, pen tool paths, etc)
  image__skip(s, image__get32be(s));

  // Skip the reserved data.
  image__skip(s, image__get32be(s));

  // Find out if the data is compressed.
  // Known values:
  //   0: no compression
  //   1: RLE compressed
  compression = image__get16be(s);
  if (compression > 1)
    return (ubyte *)(iter)stb_set_error("bad compression : PSD has an unknown compression format");

  // Check size
  if (!image__validm3ad(4, w, h, 0))
    return (ubyte *)(iter)stb_set_error("Corrupt PSD");

  // Create the destination image.

  if (!compression && bitdepth == 16 && bpc == 16) {
    out = (ubyte *)image__malloc_mad3(8, w, h, 0);
    ri->bits_per_channel = 16;
  } else
    out = (ubyte *)util_malloc(4 * w * h);

  if (!out)
    return (ubyte *)(iter)stb_set_error("outofmem : Out of memory");
  pixelCount = w * h;

  // Initialize the data to zero.
  // util_memset( out, 0, pixelCount * 4 );

  // Finally, the image data.
  if (compression) {
    // RLE as used by .PSD and .TIFF
    // Loop until you get the number of unpacked bytes you are expecting:
    //     Read the next source byte into n.
    //     If n is between 0 and 127 inclusive, copy the next n+1 bytes literally.
    //     Else if n is between -127 and -1 inclusive, copy the next byte -n+1 times.
    //     Else if n is 128, noop.
    // Endloop

    // The RLE-compressed data is preceded by a 2-byte data count for each row in the data,
    // which we're going to just skip.
    image__skip(s, h * channelCount * 2);

    // Read the RLE data by channel.
    for (channel = 0; channel < 4; channel++) {
      ubyte *p;

      p = out + channel;
      if (channel >= channelCount) {
        // Fill this channel with default data.
        for (i = 0; i < pixelCount; i++, p += 4)
          *p = (channel == 3 ? 255 : 0);
      } else {
        // Read the RLE data.
        if (!image__psd_decode_rle(s, p, pixelCount)) {
          util_memfree(out);
          return (ubyte *)(iter)stb_set_error("corrupt : bad RLE data");
        }
      }
    }

  } else {
    // We're at the raw image data.  It's each channel in order (Red, Green, Blue, Alpha, ...)
    // where each channel consists of an 8-bit (or 16-bit) value for each pixel in the image.

    // Read the data by channel.
    for (channel = 0; channel < 4; channel++) {
      if (channel >= channelCount) {
        // Fill this channel with default data.
        if (bitdepth == 16 && bpc == 16) {
          ushrt *q = ((ushrt *)out) + channel;
          ushrt val = channel == 3 ? 65535 : 0;
          for (i = 0; i < pixelCount; i++, q += 4)
            *q = val;
        } else {
          ubyte *p = out + channel;
          ubyte val = channel == 3 ? 255 : 0;
          for (i = 0; i < pixelCount; i++, p += 4)
            *p = val;
        }
      } else {
        if (ri->bits_per_channel == 16) { // output bpc
          ushrt *q = ((ushrt *)out) + channel;
          for (i = 0; i < pixelCount; i++, q += 4)
            *q = (ushrt)image__get16be(s);
        } else {
          ubyte *p = out + channel;
          if (bitdepth == 16) { // input bpc
            for (i = 0; i < pixelCount; i++, p += 4)
              *p = (ubyte)(image__get16be(s) >> 8);
          } else {
            for (i = 0; i < pixelCount; i++, p += 4)
              *p = image__get8(s);
          }
        }
      }
    }
  }

  // remove weird white matte from PSD
  if (channelCount >= 4) {
    if (ri->bits_per_channel == 16) {
      for (i = 0; i < w * h; ++i) {
        ushrt *pixel = (ushrt *)out + 4 * i;
        if (pixel[3] != 0 && pixel[3] != 65535) {
          float a = pixel[3] / 65535.0f;
          float ra = 1.0f / a;
          float inv_a = 65535.0f * (1 - ra);
          pixel[0] = (ushrt)(pixel[0] * ra + inv_a);
          pixel[1] = (ushrt)(pixel[1] * ra + inv_a);
          pixel[2] = (ushrt)(pixel[2] * ra + inv_a);
        }
      }
    } else {
      for (i = 0; i < w * h; ++i) {
        ubyte *pixel = out + 4 * i;
        if (pixel[3] != 0 && pixel[3] != 255) {
          float a = pixel[3] / 255.0f;
          float ra = 1.0f / a;
          float inv_a = 255.0f * (1 - ra);
          pixel[0] = (ubyte)(pixel[0] * ra + inv_a);
          pixel[1] = (ubyte)(pixel[1] * ra + inv_a);
          pixel[2] = (ubyte)(pixel[2] * ra + inv_a);
        }
      }
    }
  }

  // convert to desired output format
  if (req_comp && req_comp != 4) {
    if (ri->bits_per_channel == 16)
      out = (ubyte *)image__convert_format16((ushrt *)out, 4, req_comp, w, h);
    else
      out = image__convert_format(out, 4, req_comp, w, h);
    if (out == NULL)
      return out; // image__convert_format frees input on failure
  }

  if (comp)
    *comp = 4;
  *y = h;
  *x = w;

  return out;
}

// *************************************************************************************************
// Softimage PIC loader
// by Tom Seddon
//
// See http://softimage.wiki.softimage.com/index.php/INFO:_PIC_file_format
// See http://ozviz.wasp.uwa.edu.au/~pbourke/dataformats/softimagepic/

static int image__pic_is4(image__context *s, const char *str) {
  int i;
  for (i = 0; i < 4; ++i)
    if (image__get8(s) != (ubyte)str[i])
      return 0;

  return 1;
}
static int image__pic_test_core(image__context *s) {
  int i;

  if (!image__pic_is4(s, "\x53\x80\xF6\x34"))
    return 0;

  for (i = 0; i < 84; ++i)
    image__get8(s);

  if (!image__pic_is4(s, "PICT"))
    return 0;

  return 1;
}
typedef struct {
  ubyte size, type, channel;
} image__pic_packet;
static ubyte *image__readval(image__context *s, int channel, ubyte *dest) {
  int mask = 0x80, i;

  for (i = 0; i < 4; ++i, mask >>= 1) {
    if (channel & mask) {
      if (image__eof(s))
        return (ubyte *)(iter)stb_set_error("bad file : PIC file too shrt");
      dest[i] = image__get8(s);
    }
  }

  return dest;
}
static void image__copyval(int channel, ubyte *dest, const ubyte *src) {
  int mask = 0x80, i;

  for (i = 0; i < 4; ++i, mask >>= 1)
    if (channel & mask)
      dest[i] = src[i];
}
static ubyte *image__pic_load_core(image__context *s, int width, int height, int *comp, ubyte *result) {
  int act_comp = 0, num_packets = 0, y, chained;
  image__pic_packet packets[10];

  // this will (should...) cater for even some bizarre stuff like having data
  // for the same channel in multiple packets.
  do {
    image__pic_packet *packet;

    if (num_packets == sizeof(packets) / sizeof(packets[0]))
      return (ubyte *)(iter)stb_set_error("bad format : too many packets");

    packet = &packets[num_packets++];

    chained = image__get8(s);
    packet->size = image__get8(s);
    packet->type = image__get8(s);
    packet->channel = image__get8(s);

    act_comp |= packet->channel;

    if (image__eof(s))
      return (ubyte *)(iter)stb_set_error("bad file : file too shrt (reading packets)");
    if (packet->size != 8)
      return (ubyte *)(iter)stb_set_error("bad format : packet isn't 8bpp");
  } while (chained);

  *comp = (act_comp & 0x10 ? 4 : 3); // has alpha channel?

  for (y = 0; y < height; ++y) {
    int packet_idx;

    for (packet_idx = 0; packet_idx < num_packets; ++packet_idx) {
      image__pic_packet *packet = &packets[packet_idx];
      ubyte *dest = result + y * width * 4;

      switch (packet->type) {
      default:
        return (ubyte *)(iter)stb_set_error("bad format : packet has bad compression type");

      case 0: { // uncompressed
        int x;

        for (x = 0; x < width; ++x, dest += 4)
          if (!image__readval(s, packet->channel, dest))
            return 0;
        break;
      }

      case 1: // Pure RLE
      {
        int left = width, i;

        while (left > 0) {
          ubyte count, value[4];

          count = image__get8(s);
          if (image__eof(s))
            return (ubyte *)(iter)stb_set_error("bad file : file too shrt (pure read count)");

          if (count > left)
            count = (ubyte)left;

          if (!image__readval(s, packet->channel, value))
            return 0;

          for (i = 0; i < count; ++i, dest += 4)
            image__copyval(packet->channel, dest, value);
          left -= count;
        }
      } break;

      case 2: { // Mixed RLE
        int left = width;
        while (left > 0) {
          int count = image__get8(s), i;
          if (image__eof(s))
            return (ubyte *)(iter)stb_set_error("bad file : file too shrt (mixed read count)");

          if (count >= 128) { // Repeated
            ubyte value[4];

            if (count == 128)
              count = image__get16be(s);
            else
              count -= 127;
            if (count > left)
              return (ubyte *)(iter)stb_set_error("bad file : scanline overrun");

            if (!image__readval(s, packet->channel, value))
              return 0;

            for (i = 0; i < count; ++i, dest += 4)
              image__copyval(packet->channel, dest, value);
          } else { // Raw
            ++count;
            if (count > left)
              return (ubyte *)(iter)stb_set_error("bad file : scanline overrun");

            for (i = 0; i < count; ++i, dest += 4)
              if (!image__readval(s, packet->channel, dest))
                return 0;
          }
          left -= count;
        }
        break;
      }
      }
    }
  }

  return result;
}
static void *image__pic_load(image__context *s, int *px, int *py, int *comp, int req_comp, image__result_info *ri) {
  ubyte *result;
  int i, x, y, internal_comp;
  UNUSED(ri);

  if (!comp)
    comp = &internal_comp;

  for (i = 0; i < 92; ++i)
    image__get8(s);

  x = image__get16be(s);
  y = image__get16be(s);

  if (y > MAX_DIMENSIONS)
    return (ubyte *)(iter)stb_set_error("Very large image (corrupt?)");
  if (x > MAX_DIMENSIONS)
    return (ubyte *)(iter)stb_set_error("Very large image (corrupt?)");

  if (image__eof(s))
    return (ubyte *)(iter)stb_set_error("bad file : file too shrt (pic header)");
  if (!image__validm3ad(x, y, 4, 0))
    return (ubyte *)(iter)stb_set_error("PIC image too large to decode");

  image__get32be(s); // skip `ratio'
  image__get16be(s); // skip `fields'
  image__get16be(s); // skip `pad'

  // intermediate buffer is RGBA
  result = (ubyte *)image__malloc_mad3(x, y, 4, 0);
  if (!result)
    return (ubyte *)(iter)stb_set_error("outofmem : Out of memory");
  util_memset(result, 0xff, x * y * 4);

  if (!image__pic_load_core(s, x, y, comp, result)) {
    util_memfree(result);
    result = 0;
  }
  *px = x;
  *py = y;
  if (req_comp == 0)
    req_comp = *comp;
  result = image__convert_format(result, 4, req_comp, x, y);

  return result;
}
static int image__pic_test(image__context *s) {
  int r = image__pic_test_core(s);
  image__rewind(s);
  return r;
}

// *************************************************************************************************
// GIF loader -- public domain by Jean-Marc Lienher -- simplified/shrunk by stb

typedef struct {
  shrt prefix;
  ubyte first;
  ubyte suffix;
} image__gif_lzw;
typedef struct {
  int w, h;
  ubyte *out;        // output buffer (always 4 components)
  ubyte *background; // The current "background" as far as a gif is concerned
  ubyte *history;
  int flags, bgindex, ratio, transparent, eflags;
  ubyte pal[256][4];
  ubyte lpal[256][4];
  image__gif_lzw codes[8192];
  ubyte *color_table;
  int parse, step;
  int lflags;
  int start_x, start_y;
  int max_x, max_y;
  int cur_x, cur_y;
  int line_size;
  int delay;
} image__gif;
static int image__gif_test_raw(image__context *s) {
  int sz;
  if (image__get8(s) != 'G' || image__get8(s) != 'I' || image__get8(s) != 'F' || image__get8(s) != '8')
    return 0;
  sz = image__get8(s);
  if (sz != '9' && sz != '7')
    return 0;
  if (image__get8(s) != 'a')
    return 0;
  return 1;
}
static int image__gif_test(image__context *s) {
  int r = image__gif_test_raw(s);
  image__rewind(s);
  return r;
}
static void image__gif_parse_colortable(image__context *s, ubyte pal[256][4], int num_entries, int transp) {
  int i;
  for (i = 0; i < num_entries; ++i) {
    pal[i][2] = image__get8(s);
    pal[i][1] = image__get8(s);
    pal[i][0] = image__get8(s);
    pal[i][3] = transp == i ? 0 : 255;
  }
}
static int image__gif_header(image__context *s, image__gif *g, int *comp, int is_info) {
  ubyte version;
  if (image__get8(s) != 'G' || image__get8(s) != 'I' || image__get8(s) != 'F' || image__get8(s) != '8')
    return stb_set_error("not GIF : Corrupt GIF");

  version = image__get8(s);
  if (version != '7' && version != '9')
    return stb_set_error("not GIF : Corrupt GIF");
  if (image__get8(s) != 'a')
    return stb_set_error("not GIF : Corrupt GIF");

  g->w = image__get16le(s);
  g->h = image__get16le(s);
  g->flags = image__get8(s);
  g->bgindex = image__get8(s);
  g->ratio = image__get8(s);
  g->transparent = -1;

  if (g->w > MAX_DIMENSIONS)
    return stb_set_error("Very large image (corrupt?)");
  if (g->h > MAX_DIMENSIONS)
    return stb_set_error("Very large image (corrupt?)");

  if (comp != 0)
    *comp = 4; // can't actually tell whether it's 3 or 4 until we parse the comments

  if (is_info)
    return 1;

  if (g->flags & 0x80)
    image__gif_parse_colortable(s, g->pal, 2 << (g->flags & 7), -1);

  return 1;
}
static int image__gif_info_raw(image__context *s, int *x, int *y, int *comp) {
  image__gif *g = (image__gif *)util_malloc(sizeof(image__gif));
  if (!g)
    return stb_set_error("outofmem : Out of memory");
  if (!image__gif_header(s, g, comp, 1)) {
    util_memfree(g);
    image__rewind(s);
    return 0;
  }
  if (x)
    *x = g->w;
  if (y)
    *y = g->h;
  util_memfree(g);
  return 1;
}
static void image__out_gif_code(image__gif *g, ushrt code) {
  ubyte *p, *c;
  int idx;

  // recurse to decode the prefixes, since the linked-list is backwards,
  // and working backwards through an interleaved image would be nasty
  if (g->codes[code].prefix >= 0)
    image__out_gif_code(g, g->codes[code].prefix);

  if (g->cur_y >= g->max_y)
    return;

  idx = g->cur_x + g->cur_y;
  p = &g->out[idx];
  g->history[idx / 4] = 1;

  c = &g->color_table[g->codes[code].suffix * 4];
  if (c[3] > 128) { // don't render transparent pixels;
    p[0] = c[2];
    p[1] = c[1];
    p[2] = c[0];
    p[3] = c[3];
  }
  g->cur_x += 4;

  if (g->cur_x >= g->max_x) {
    g->cur_x = g->start_x;
    g->cur_y += g->step;

    while (g->cur_y >= g->max_y && g->parse > 0) {
      g->step = (1 << g->parse) * g->line_size;
      g->cur_y = g->start_y + (g->step >> 1);
      --g->parse;
    }
  }
}
static ubyte *image__process_gif_raster(image__context *s, image__gif *g) {
  ubyte lzw_cs;
  int32 len, init_code;
  uint32 first;
  int32 codesize, codemask, avail, oldcode, bits, valid_bits, clear;
  image__gif_lzw *p;

  lzw_cs = image__get8(s);
  if (lzw_cs > 12)
    return NULL;
  clear = 1 << lzw_cs;
  first = 1;
  codesize = lzw_cs + 1;
  codemask = (1 << codesize) - 1;
  bits = 0;
  valid_bits = 0;
  for (init_code = 0; init_code < clear; init_code++) {
    g->codes[init_code].prefix = -1;
    g->codes[init_code].first = (ubyte)init_code;
    g->codes[init_code].suffix = (ubyte)init_code;
  }

  // support no starting clear code
  avail = clear + 2;
  oldcode = -1;

  len = 0;
  for (;;) {
    if (valid_bits < codesize) {
      if (len == 0) {
        len = image__get8(s); // start new block
        if (len == 0)
          return g->out;
      }
      --len;
      bits |= (int32)image__get8(s) << valid_bits;
      valid_bits += 8;
    } else {
      int32 code = bits & codemask;
      bits >>= codesize;
      valid_bits -= codesize;
      // @OPTIMIZE: is there some way we can accelerate the non-clear path?
      if (code == clear) { // clear code
        codesize = lzw_cs + 1;
        codemask = (1 << codesize) - 1;
        avail = clear + 2;
        oldcode = -1;
        first = 0;
      } else if (code == clear + 1) { // end of stream code
        image__skip(s, len);
        while ((len = image__get8(s)) > 0)
          image__skip(s, len);
        return g->out;
      } else if (code <= avail) {
        if (first) {
          return (ubyte *)(iter)stb_set_error("no clear code : Corrupt GIF");
        }

        if (oldcode >= 0) {
          p = &g->codes[avail++];
          if (avail > 8192) {
            return (ubyte *)(iter)stb_set_error("too many codes : Corrupt GIF");
          }

          p->prefix = (shrt)oldcode;
          p->first = g->codes[oldcode].first;
          p->suffix = (code == avail) ? p->first : g->codes[code].first;
        } else if (code == avail)
          return (ubyte *)(iter)stb_set_error("illegal code in raster : Corrupt GIF");

        image__out_gif_code(g, (ushrt)code);

        if ((avail & codemask) == 0 && avail <= 0x0FFF) {
          codesize++;
          codemask = (1 << codesize) - 1;
        }

        oldcode = code;
      } else {
        return (ubyte *)(iter)stb_set_error("illegal code in raster : Corrupt GIF");
      }
    }
  }
}
// this function is designed to support animated gifs, although stb_image doesn't support it
// two back is the image from two frames ago, used for a very specific disposal format
static ubyte *image__gif_load_next(image__context *s, image__gif *g, int *comp, int req_comp, ubyte *two_back) {
  int dispose;
  int first_frame;
  int pi;
  int pcount;
  UNUSED(req_comp);

  // on first frame, any non-written pixels get the background colour (non-transparent)
  first_frame = 0;
  if (g->out == 0) {
    if (!image__gif_header(s, g, comp, 0))
      return 0;
    if (!image__validm3ad(4, g->w, g->h, 0))
      return (ubyte *)(iter)stb_set_error("GIF image is too large");
    pcount = g->w * g->h;
    g->out = (ubyte *)util_malloc(4 * pcount);
    g->background = (ubyte *)util_malloc(4 * pcount);
    g->history = (ubyte *)util_malloc(pcount);
    if (!g->out || !g->background || !g->history)
      return (ubyte *)(iter)stb_set_error("outofmem : Out of memory");

    // image is treated as "transparent" at the start - ie, nothing overwrites the current background;
    // background colour is only used for pixels that are not rendered first frame, after that "background"
    // color refers to the color that was there the previous frame.
    util_memset(g->out, 0x00, 4 * pcount);
    util_memset(g->background, 0x00, 4 * pcount); // state of the background (starts transparent)
    util_memset(g->history, 0x00, pcount);        // pixels that were affected previous frame
    first_frame = 1;
  } else {
    // second frame - how do we dispose of the previous one?
    dispose = (g->eflags & 0x1C) >> 2;
    pcount = g->w * g->h;

    if ((dispose == 3) && (two_back == 0)) {
      dispose = 2; // if I don't have an image to revert back to, default to the old background
    }

    if (dispose == 3) { // use previous graphic
      for (pi = 0; pi < pcount; ++pi) {
        if (g->history[pi]) {
          util_memcpy(&g->out[pi * 4], &two_back[pi * 4], 4);
        }
      }
    } else if (dispose == 2) {
      // restore what was changed last frame to background before that frame;
      for (pi = 0; pi < pcount; ++pi) {
        if (g->history[pi]) {
          util_memcpy(&g->out[pi * 4], &g->background[pi * 4], 4);
        }
      }
    } else {
      // This is a non-disposal case eithe way, so just
      // leave the pixels as is, and they will become the new background
      // 1: do not dispose
      // 0:  not specified.
    }

    // background is what out is after the undoing of the previou frame;
    util_memcpy(g->background, g->out, 4 * g->w * g->h);
  }

  // clear my history;
  util_memset(g->history, 0x00, g->w * g->h); // pixels that were affected previous frame

  for (;;) {
    int tag = image__get8(s);
    switch (tag) {
    case 0x2C: /* Image Descriptor */
    {
      int32 x, y, w, h;
      ubyte *o;

      x = image__get16le(s);
      y = image__get16le(s);
      w = image__get16le(s);
      h = image__get16le(s);
      if (((x + w) > (g->w)) || ((y + h) > (g->h)))
        return (ubyte *)(iter)stb_set_error("bad Image Descriptor : Corrupt GIF");

      g->line_size = g->w * 4;
      g->start_x = x * 4;
      g->start_y = y * g->line_size;
      g->max_x = g->start_x + w * 4;
      g->max_y = g->start_y + h * g->line_size;
      g->cur_x = g->start_x;
      g->cur_y = g->start_y;

      // if the width of the specified rectangle is 0, that means
      // we may not see *any* pixels or the image is malformed;
      // to make sure this is caught, move the current y down to
      // max_y (which is what out_gif_code checks).
      if (w == 0)
        g->cur_y = g->max_y;

      g->lflags = image__get8(s);

      if (g->lflags & 0x40) {
        g->step = 8 * g->line_size; // first interlaced spacing
        g->parse = 3;
      } else {
        g->step = g->line_size;
        g->parse = 0;
      }

      if (g->lflags & 0x80) {
        image__gif_parse_colortable(s, g->lpal, 2 << (g->lflags & 7), g->eflags & 0x01 ? g->transparent : -1);
        g->color_table = (ubyte *)g->lpal;
      } else if (g->flags & 0x80) {
        g->color_table = (ubyte *)g->pal;
      } else
        return (ubyte *)(iter)stb_set_error("missing color table : Corrupt GIF");

      o = image__process_gif_raster(s, g);
      if (!o)
        return NULL;

      // if this was the first frame,
      pcount = g->w * g->h;
      if (first_frame && (g->bgindex > 0)) {
        // if first frame, any pixel not drawn to gets the background color
        for (pi = 0; pi < pcount; ++pi) {
          if (g->history[pi] == 0) {
            g->pal[g->bgindex][3] = 255; // just in case it was made transparent, undo that; It will be reset next frame if need be;
            util_memcpy(&g->out[pi * 4], &g->pal[g->bgindex], 4);
          }
        }
      }

      return o;
    }

    case 0x21: // Comment Extension.
    {
      int len;
      int ext = image__get8(s);
      if (ext == 0xF9) { // Graphic Control Extension.
        len = image__get8(s);
        if (len == 4) {
          g->eflags = image__get8(s);
          g->delay = 10 * image__get16le(s); // delay - 1/100th of a second, saving as 1/1000ths.

          // unset old transparent
          if (g->transparent >= 0) {
            g->pal[g->transparent][3] = 255;
          }
          if (g->eflags & 0x01) {
            g->transparent = image__get8(s);
            if (g->transparent >= 0) {
              g->pal[g->transparent][3] = 0;
            }
          } else {
            // don't need transparent
            image__skip(s, 1);
            g->transparent = -1;
          }
        } else {
          image__skip(s, len);
          break;
        }
      }
      while ((len = image__get8(s)) != 0) {
        image__skip(s, len);
      }
      break;
    }

    case 0x3B:             // gif stream termination code
      return (ubyte *)s; // using '1' causes warning on some compilers

    default:
      return (ubyte *)(iter)stb_set_error("unknown code : Corrupt GIF");
    }
  }
}
static void *image__read_gif_main_outofmem(image__gif *g, ubyte *out, int **delays) {
  util_memfree(g->out);
  util_memfree(g->history);
  util_memfree(g->background);

  if (out)
    util_memfree(out);
  if (delays && *delays)
    util_memfree(*delays);
  return (ubyte *)(iter)stb_set_error("outofmem : Out of memory");
}
static void *image__read_gif_main(image__context *s, int **delays, int *x, int *y, int *z, int *comp, int req_comp) {
  if (image__gif_test(s)) {
    int layers = 0;
    ubyte *u = 0;
    ubyte *out = 0;
    ubyte *two_back = 0;
    image__gif g;
    int stride;
    int out_size = 0;
    int delays_size = 0;

    UNUSED(out_size);
    UNUSED(delays_size);

    util_memset(&g, 0, sizeof(g));
    if (delays) {
      *delays = 0;
    }

    do {
      u = image__gif_load_next(s, &g, comp, req_comp, two_back);
      if (u == (ubyte *)s)
        u = 0; // end of animated gif marker

      if (u) {
        *x = g.w;
        *y = g.h;
        ++layers;
        stride = g.w * g.h * 4;

        if (out) {
          void *tmp = (ubyte *)util_realloc(out, layers * stride);
          if (!tmp)
            return image__read_gif_main_outofmem(&g, out, delays);
          else {
            out = (ubyte *)tmp;
            out_size = layers * stride;
          }

          if (delays) {
            int *new_delays = (int *)util_realloc(*delays, sizeof(int) * layers);
            if (!new_delays)
              return image__read_gif_main_outofmem(&g, out, delays);
            *delays = new_delays;
            delays_size = layers * sizeof(int);
          }
        } else {
          out = (ubyte *)util_malloc(layers * stride);
          if (!out)
            return image__read_gif_main_outofmem(&g, out, delays);
          out_size = layers * stride;
          if (delays) {
            *delays = (int *)util_malloc(layers * sizeof(int));
            if (!*delays)
              return image__read_gif_main_outofmem(&g, out, delays);
            delays_size = layers * sizeof(int);
          }
        }
        util_memcpy(out + ((layers - 1) * stride), u, stride);
        if (layers >= 2) {
          two_back = out - 2 * stride;
        }

        if (delays) {
          (*delays)[layers - 1U] = g.delay;
        }
      }
    } while (u != 0);

    // free temp buffer;
    util_memfree(g.out);
    util_memfree(g.history);
    util_memfree(g.background);

    // do the final conversion after loading everything;
    if (req_comp && req_comp != 4)
      out = image__convert_format(out, 4, req_comp, layers * g.w, g.h);

    *z = layers;
    return out;
  } else {
    return (ubyte *)(iter)stb_set_error("not GIF : Image was not as a gif type.");
  }
}
static void *image__gif_load(image__context *s, int *x, int *y, int *comp, int req_comp, image__result_info *ri) {
  ubyte *u = 0;
  image__gif g;
  util_memset(&g, 0, sizeof(g));
  UNUSED(ri);

  u = image__gif_load_next(s, &g, comp, req_comp, 0);
  if (u == (ubyte *)s)
    u = 0; // end of animated gif marker
  if (u) {
    *x = g.w;
    *y = g.h;

    // moved conversion to after successful load so that the same
    // can be done for multiple frames.
    if (req_comp && req_comp != 4)
      u = image__convert_format(u, 4, req_comp, g.w, g.h);
  } else if (g.out) {
    // if there was an error and we allocated an image buffer, free it!
    util_memfree(g.out);
  }

  // free buffers needed for multiple frame loading;
  util_memfree(g.history);
  util_memfree(g.background);

  return u;
}
static int image__gif_info(image__context *s, int *x, int *y, int *comp) {
  return image__gif_info_raw(s, x, y, comp);
}

// *************************************************************************************************
// Radiance RGBE HDR loader
// originally by Nicolas Schulz
static int image__hdr_test_core(image__context *s, const char *signature) {
  int i;
  for (i = 0; signature[i]; ++i)
    if (image__get8(s) != signature[i])
      return 0;
  image__rewind(s);
  return 1;
}
static int image__hdr_test(image__context *s) {
  int r = image__hdr_test_core(s, "#?RADIANCE\n");
  image__rewind(s);
  if (!r) {
    r = image__hdr_test_core(s, "#?RGBE\n");
    image__rewind(s);
  }
  return r;
}
#define image__HDR_BUFLEN 1024
static char *image__hdr_gettoken(image__context *z, char *buffer) {
  int len = 0;
  char c = '\0';

  c = (char)image__get8(z);

  while (!image__eof(z) && c != '\n') {
    buffer[len++] = c;
    if (len == image__HDR_BUFLEN - 1) {
      // flush to end of line
      while (!image__eof(z) && image__get8(z) != '\n')
        ;
      break;
    }
    c = (char)image__get8(z);
  }

  buffer[len] = 0;
  return buffer;
}
static void image__hdr_convert(float *output, ubyte *input, int req_comp) {
  if (input[3] != 0) {
    float f1;
    // Exponent
    f1 = (float)imath_ldexp(1.0f, input[3] - (int)(128 + 8));
    if (req_comp <= 2)
      output[0] = (input[0] + input[1] + input[2]) * f1 / 3;
    else {
      output[0] = input[0] * f1;
      output[1] = input[1] * f1;
      output[2] = input[2] * f1;
    }
    if (req_comp == 2)
      output[1] = 1;
    if (req_comp == 4)
      output[3] = 1;
  } else {
    switch (req_comp) {
    case 4:
      output[3] = 1; /* fallthrough */
    case 3:
      output[0] = output[1] = output[2] = 0;
      break;
    case 2:
      output[1] = 1; /* fallthrough */
    case 1:
      output[0] = 0;
      break;
    }
  }
}
static float *image__hdr_load(image__context *s, int *x, int *y, int *comp, int req_comp, image__result_info *ri) {
  char buffer[image__HDR_BUFLEN];
  char *token;
  int valid = 0;
  int width, height;
  ubyte *scanline;
  float *hdr_data;
  int len;
  ubyte count, value;
  int i, j, k, c1, c2, z;
  const char *headerToken;
  UNUSED(ri);

  // Check identifier
  headerToken = image__hdr_gettoken(s, buffer);
  if (strcmp(headerToken, "#?RADIANCE") != 0 && strcmp(headerToken, "#?RGBE") != 0)
    return (float *)(iter)stb_set_error("not HDR : Corrupt HDR image");

  // Parse header
  for (;;) {
    token = image__hdr_gettoken(s, buffer);
    if (token[0] == 0)
      break;
    if (strcmp(token, "FORMAT=32-bit_rle_rgbe") == 0)
      valid = 1;
  }

  if (!valid)
    return (float *)(iter)stb_set_error("unsupported format : Unsupported HDR format");

  // Parse width and height
  // can't use sscanf() if we're not using stdio!
  token = image__hdr_gettoken(s, buffer);
  if (strncmp(token, "-Y ", 3))
    return (float *)(iter)stb_set_error("unsupported data layout : Unsupported HDR format");
  token += 3;
  height = (int)strtol(token, &token, 10);
  while (*token == ' ')
    ++token;
  if (strncmp(token, "+X ", 3))
    return (float *)(iter)stb_set_error("unsupported data layout : Unsupported HDR format");
  token += 3;
  width = (int)strtol(token, NULL, 10);

  if (height > MAX_DIMENSIONS)
    return (float *)(iter)stb_set_error("Very large image (corrupt?)");
  if (width > MAX_DIMENSIONS)
    return (float *)(iter)stb_set_error("Very large image (corrupt?)");

  *x = width;
  *y = height;

  if (comp)
    *comp = 3;
  if (req_comp == 0)
    req_comp = 3;

  if (!image__validm4ad(width, height, req_comp, sizeof(float), 0))
    return (float *)(iter)stb_set_error("HDR image is too large");

  // Read data
  hdr_data = (float *)image__malloc_mad4(width, height, req_comp, sizeof(float), 0);
  if (!hdr_data)
    return (float *)(iter)stb_set_error("outofmem : Out of memory");

  // Load image data
  // image data is stored as some number of sca
  if (width < 8 || width >= 32768) {
    // Read flat data
    for (j = 0; j < height; ++j) {
      for (i = 0; i < width; ++i) {
        ubyte rgbe[4];
      main_decode_loop:
        image__read(s, rgbe, 4);
        image__hdr_convert(hdr_data + j * width * req_comp + i * req_comp, rgbe, req_comp);
      }
    }
  } else {
    // Read RLE-encoded data
    scanline = NULL;

    for (j = 0; j < height; ++j) {
      c1 = image__get8(s);
      c2 = image__get8(s);
      len = image__get8(s);
      if (c1 != 2 || c2 != 2 || (len & 0x80)) {
        // not run-length encoded, so we have to actually use THIS data as a decoded
        // pixel (note this can't be a valid pixel--one of RGB must be >= 128)
        ubyte rgbe[4];
        rgbe[0] = (ubyte)c1;
        rgbe[1] = (ubyte)c2;
        rgbe[2] = (ubyte)len;
        rgbe[3] = (ubyte)image__get8(s);
        image__hdr_convert(hdr_data, rgbe, req_comp);
        i = 1;
        j = 0;
        util_memfree(scanline);
        goto main_decode_loop; // yes, this makes no sense
      }
      len <<= 8;
      len |= image__get8(s);
      if (len != width) {
        util_memfree(hdr_data);
        util_memfree(scanline);
        return (float *)(iter)stb_set_error("invalid decoded scanline length : corrupt HDR");
      }
      if (scanline == NULL) {
        scanline = (ubyte *)image__malloc_mad2(width, 4, 0);
        if (!scanline) {
          util_memfree(hdr_data);
          return (float *)(iter)stb_set_error("outofmem : Out of memory");
        }
      }

      for (k = 0; k < 4; ++k) {
        int nleft;
        i = 0;
        while ((nleft = width - i) > 0) {
          count = image__get8(s);
          if (count > 128) {
            // Run
            value = image__get8(s);
            count -= 128;
            if ((count == 0) || (count > nleft)) {
              util_memfree(hdr_data);
              util_memfree(scanline);
              return (float *)(iter)stb_set_error("corrupt : bad RLE data in HDR");
            }
            for (z = 0; z < count; ++z)
              scanline[i++ * 4 + k] = value;
          } else {
            // Dump
            if ((count == 0) || (count > nleft)) {
              util_memfree(hdr_data);
              util_memfree(scanline);
              return (float *)(iter)stb_set_error("corrupt : bad RLE data in HDR");
            }
            for (z = 0; z < count; ++z)
              scanline[i++ * 4 + k] = image__get8(s);
          }
        }
      }
      for (i = 0; i < width; ++i)
        image__hdr_convert(hdr_data + (j * width + i) * req_comp, scanline + i * 4, req_comp);
    }
    if (scanline)
      util_memfree(scanline);
  }

  return hdr_data;
}
static int image__hdr_info(image__context *s, int *x, int *y, int *comp) {
  char buffer[image__HDR_BUFLEN];
  char *token;
  int valid = 0;
  int dummy;

  if (!x)
    x = &dummy;
  if (!y)
    y = &dummy;
  if (!comp)
    comp = &dummy;

  if (image__hdr_test(s) == 0) {
    image__rewind(s);
    return 0;
  }

  for (;;) {
    token = image__hdr_gettoken(s, buffer);
    if (token[0] == 0)
      break;
    if (strcmp(token, "FORMAT=32-bit_rle_rgbe") == 0)
      valid = 1;
  }

  if (!valid) {
    image__rewind(s);
    return 0;
  }
  token = image__hdr_gettoken(s, buffer);
  if (strncmp(token, "-Y ", 3)) {
    image__rewind(s);
    return 0;
  }
  token += 3;
  *y = (int)strtol(token, &token, 10);
  while (*token == ' ')
    ++token;
  if (strncmp(token, "+X ", 3)) {
    image__rewind(s);
    return 0;
  }
  token += 3;
  *x = (int)strtol(token, NULL, 10);
  *comp = 3;
  return 1;
}
static int image__bmp_info(image__context *s, int *x, int *y, int *comp) {
  void *p;
  image__bmp_data info;

  info.all_a = 255;
  p = image__bmp_parse_header(s, &info);
  if (p == NULL) {
    image__rewind(s);
    return 0;
  }
  if (x)
    *x = s->img_x;
  if (y)
    *y = s->img_y;
  if (comp) {
    if (info.bpp == 24 && info.ma == 0xff000000)
      *comp = 3;
    else
      *comp = info.ma ? 4 : 3;
  }
  return 1;
}
static int image__psd_info(image__context *s, int *x, int *y, int *comp) {
  int channelCount, dummy, depth;
  if (!x)
    x = &dummy;
  if (!y)
    y = &dummy;
  if (!comp)
    comp = &dummy;
  if (image__get32be(s) != 0x38425053) {
    image__rewind(s);
    return 0;
  }
  if (image__get16be(s) != 1) {
    image__rewind(s);
    return 0;
  }
  image__skip(s, 6);
  channelCount = image__get16be(s);
  if (channelCount < 0 || channelCount > 16) {
    image__rewind(s);
    return 0;
  }
  *y = image__get32be(s);
  *x = image__get32be(s);
  depth = image__get16be(s);
  if (depth != 8 && depth != 16) {
    image__rewind(s);
    return 0;
  }
  if (image__get16be(s) != 3) {
    image__rewind(s);
    return 0;
  }
  *comp = 4;
  return 1;
}
static int image__pic_info(image__context *s, int *x, int *y, int *comp) {
  int act_comp = 0, num_packets = 0, chained, dummy;
  image__pic_packet packets[10];

  if (!x)
    x = &dummy;
  if (!y)
    y = &dummy;
  if (!comp)
    comp = &dummy;

  if (!image__pic_is4(s, "\x53\x80\xF6\x34")) {
    image__rewind(s);
    return 0;
  }

  image__skip(s, 88);

  *x = image__get16be(s);
  *y = image__get16be(s);
  if (image__eof(s)) {
    image__rewind(s);
    return 0;
  }
  if ((*x) != 0 && (1 << 28) / (*x) < (*y)) {
    image__rewind(s);
    return 0;
  }

  image__skip(s, 8);

  do {
    image__pic_packet *packet;

    if (num_packets == sizeof(packets) / sizeof(packets[0]))
      return 0;

    packet = &packets[num_packets++];
    chained = image__get8(s);
    packet->size = image__get8(s);
    packet->type = image__get8(s);
    packet->channel = image__get8(s);
    act_comp |= packet->channel;

    if (image__eof(s)) {
      image__rewind(s);
      return 0;
    }
    if (packet->size != 8) {
      image__rewind(s);
      return 0;
    }
  } while (chained);

  *comp = (act_comp & 0x10 ? 4 : 3);

  return 1;
}

// *************************************************************************************************
// Portable Gray Map and Portable Pixel Map loader
// by Ken Miller
//
// PGM: http://netpbm.sourceforge.net/doc/pgm.html
// PPM: http://netpbm.sourceforge.net/doc/ppm.html
//
// Known limitations:
//    Does not support comments in the header section
//    Does not support ASCII image data (formats P2 and P3)
static inline bool image__pnm_isspace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}
static void image__pnm_skip_whitespace(image__context *s, char *c) {
  for (;;) {
    while (!image__eof(s) && image__pnm_isspace(*c))
      *c = (char)image__get8(s);
    if (image__eof(s) || *c != '#')
      break;
    while (!image__eof(s) && *c != '\n' && *c != '\r')
      *c = (char)image__get8(s);
  }
}
static inline bool image__pnm_isdigit(char c) {
  return c >= '0' && c <= '9';
}
static int image__pnm_getinteger(image__context *s, char *c) {
  int value = 0;
  while (!image__eof(s) && image__pnm_isdigit(*c)) {
    value = value * 10 + (*c - '0');
    *c = (char)image__get8(s);
    if ((value > 214748364) || (value == 214748364 && *c > '7')) {
      stb_set_error("integer parse overflow : Parsing an integer in the PPM header overflowed a 32-bit int");
      return 0;
    }
  }
  return value;
}
static int image__pnm_info(image__context *s, image_info *inf) {
  byte *temp = CAST(byte*)image__readtemp(s, 2);
  if (temp[0] != 'P' || (temp[1] != '5' && temp[1] != '6')) {
    image__rewind(s);
    return 0;
  }
  inf->chnl = (temp[1] == '6') ? 3 : 1; // '5' is 1-component .pgm; '6' is 3-component .ppm
  char c = (char)image__get8(s);
  image__pnm_skip_whitespace(s, &c);
  inf->w = image__pnm_getinteger(s, &c); // read width
  image__pnm_skip_whitespace(s, &c);
  inf->h = image__pnm_getinteger(s, &c); // read height
  image__pnm_skip_whitespace(s, &c);
  int maxv = image__pnm_getinteger(s, &c); // read max value
  if (!inf->w || !inf->h || (maxv > 65535)) {
    stb_set_error("invalid parameter : PPM image header had zero or overflowing value");
    return -1;
  } else if (maxv > 255) inf->bpc = 16;
  else inf->bpc = 8;
}
static int image__pnm_load(image__context *s, image_bitmap *imb) {
  int ret = image__pnm_info(s, &(imb->inf));
  if (ret < 1) return ret;
  uint64 len = imb->inf.w * imb->inf.h * imb->inf.chnl * (imb->inf.bpc / 8);
  imb->data = CAST(ubyte *)image__malloc_mad4(imb->inf.chnl, imb->inf.w, imb->inf.h, imb->inf.bpc / 8, 0);
  if (!imb->data) {
    stb_set_error("outofmem : Out of memory");
    return -1;
  }
  if (len != image__read(s, imb->data, len)) {
    util_memfree(out);
    stb_set_error("bad PNM : PNM file truncated");
    return -1;
  }
  return 1;
}


// memory function
typedef struct {
  int read, len;
  const ubyte *data;
} image_mem_buffer;
static iter image__mem_read(void *usr, void *dst, iter n){
  image_mem_buffer *imb = CAST(image_mem_buffer*)usr;
  iter read = MIN(n, (imb->len - iimb->read));
  util_memcpy(dst, imb->data + imb->read, read);
  imb->read += read;
  return read;
}
static void image__mem_skip(void *usr, int n){
  image_mem_buffer *imb = CAST(image_mem_buffer*)usr;
  imb->read = MAX(0, MIN(imb->read + n,imb->len));
}
static void image__mem_rewind(void *usr){
  (CAST(image_mem_buffer*)usr)->read = 0;
}
static bool image__mem_eof(void *usr){
  image_mem_buffer *imb = CAST(image_mem_buffer*)usr;
  return imb->read < imb->len;
}
// file function
static iter image__io_read(void *user, void *data, iter size) {
  return file_read(data, size, (FILE *)user);
}
static void image__io_skip(void *user, int n) {
  file_seek(n, (FILE *)user);
}
static void image__io_rewind(void *user) {
  file_rewind((FILE *)user);
}
static bool image__io_eof(void *user) {
  return file_eof((FILE *)user);
}

image_info image_readinfo(char const *filename) {
  image_info ret = {0};
  FILE *f = file_open(filename, "wb");
  if (f) {
    ret = image_readinfo_from_func(CAST(void *)f, image__io_read, image__io_skip, image__io_eof);
    file_close(f);
  } else {
    stb_set_error("Fail to open file!");
  }
  return ret;
}
image_info image_readinfo_from_mem(const image_file imgf) {
  image_mem_buffer imb = {
    .data = imgf.data,
    .len = imgf.len,
    .read = 0,
  };
  return image_readinfo_from_func(CLIT(image_read_func){CAST(void *)&imb, image__mem_read, image__mem_skip, image__mem_rewind, image__mem_eof});
}
image_info image_readinfo_from_func(image_read_func read) {
  image__context s = {
    .user = read.user,
    .read = read.read,
    .skip = read.skip,
    .rewind = read.rewind,
    .eof = read.eof,
  };
  image_info ret = {0};
  typedef int image_inforeader(image__context*,image_info*);
  static const image_inforeader all_image_inforeader[] = {
    image__png_info,
    image__bmp_info,
    image__gif_info,
    image__psd_info,
    image__pic_info,
    // then the formats that can end up attempting to load with just 1 or 2
    // bytes matching expectations; these are prone to false positives, so
    // try them later
    image__jpeg_info,
    image__pnm_info,
    image__hdr_info,
    // test tga last because it's a crappy test!
    image__tga_info,
  };
  int result = 0;
  for (iter i = 0; !result && (i < STACK_ARR_LEN(all_image_inforeader)); ++i)
    result = all_image_inforeader[i](&s, &ret);
  if (!result) stb_set_error("unknown image type : Image not of any known type, or corrupt");
  return ret;
}

image_bitmap image_read_opt(char const *filename, image_opt opt) {
  image_bitmap ret = {0};
  FILE *f = file_open(filename, "wb");
  if (f) {
    ret = image_read_from_func_opt(CAST(void *)f, image__io_read, image__io_skip, image__io_eof, opt);
    file_close(f);
  } else {
    stb_set_error("Fail to open file!");
  }
  return ret;
}
image_bitmap image_read_from_mem_opt(const image_file imgf, image_opt opt) {
  image_mem_buffer imb = {
    .data = imgf.data,
    .len = imgf.len,
    .read = 0,
  };
  return image_read_from_func_opt(CLIT(image_read_func){CAST(void *)&imb, image__mem_read, image__mem_skip, image__mem_rewind, image__mem_eof}, opt);
}
image_bitmap image_read_from_func_opt(image_read_func read, image_opt opt) {
  image__context s = {
    .user = read.user,
    .read = read.read,
    .skip = read.skip,
    .rewind = read.rewind,
    .eof = read.eof,
  };
  image_bitmap ret = {0};
  // test the formats with a very explicit header first (at least a FOURCC
  // or distinctive magic number first)
  int result = 0;
  typedef int image_reader(image__context*,image_bitmap*);
  static const image_reader all_image_reader[] = {
    image__png_load,
    image__bmp_load,
    image__gif_load,
    image__psd_load,
    image__pic_load,
    // then the formats that can end up attempting to load with just 1 or 2
    // bytes matching expectations; these are prone to false positives, so
    // try them later
    image__jpeg_load,
    
    image__pnm_load,
    image__hdr_load,
    // test tga last because it's a crappy test!
    image__tga_load,
  };
  for (iter i = 0; !result && (i < STACK_ARR_LEN(all_image_reader)); ++i)
    result = all_image_reader[i](&s, &ret);
  if (result == 0) stb_set_error("unknown image type");
  else if (result == -1) stb_set_error("Corupt image data");
  return ret;
}

void image_file_free(image_file *fl) {
  util_memfree(fl->data);
  util_memset(fl, 0, sizeof(image_file));
}
void image_bitmap_free(image_bitmap *bp) {
  util_memfree(bp->data);
  util_memset(bp, 0, sizeof(image_bitmap));
}
