/* *****************************************************************************
 * image.c v0.0.0000
 * 
 * image codec
 * mostly stolen from http://nothings.org/stb
 * 
 * 
 * *****************************************************************************/

#include "codec/image.h"
#include "codec/hash.h"

#define MAX_BUFFER 1024
#define PRINT_WRN(...) printf(YELLOW"[WRN] " RESET __VA_ARGS__)
// global setting
static image_pref_flag enabled_flags = 0;


#define INFO_DO(X) do { if ((X)) {\
  *i = IMAGE_FAILURE_WORK; \
  goto png_loadinfo_end; \
} } while(0)
static image_info image__png_loadinfo(image_funct fct, image_cout *i) {
  image_info inf = {0};
  uint32 crc;
  *i = IMAGE_BAD_SIGN;
  
  byte buffer[MAX_BUFFER];
  INFO_DO(fct.read(fct.user, buffer, 8) < 8);
  if (util_memcmp(s,"\x89PNG\r\n\x1A\n", 8))
    goto png_loadinfo_end;
  // start crc header
  hash_crc32_start(&crc);
  INFO_DO(fct.read(fct.user, buffer, 8) != 8);
  if (util_memcmp(s,"\0\0\0\rIHDR", 8))
    goto png_loadinfo_end;
  hash_crc32_appends(&crc, "IHDR", 4);
  INFO_DO(fct.read(fct.user, buffer, 13) != 13);
  hash_crc32_appends(&crc, buffer, 13);
  {
    uint32 *bf = CAST(uint32*)buffer;
    inf.width  = imath_flip32(bf[0]);
    inf.height = imath_flip32(bf[1]);
  }
  inf.bits_channel = buffer[8];
  int color     = buffer[09],
      compres   = buffer[10],
      filter    = buffer[11],
      interlace = buffer[12];
  switch (color) {
    // grayscale
    case 0:
      if (inf.bits_channel != 1 && inf.bits_channel != 2 && inf.bits_channel != 4 && inf.bits_channel != 8 && inf.bits_channel != 16)
        goto png_loadinfo_end;
      inf.channel = 1;
      break;
    // indexed
    case 3: // depends pallete rgb and chunk alpha
      if (inf.bits_channel != 1 && inf.bits_channel != 2 && inf.bits_channel != 4 && inf.bits_channel != 8)
        goto png_loadinfo_end;
      inf.channel = color;
      break;
    // truecolor
    case 2: case 4: case 6:
      if (inf.bits_channel != 8 && inf.bits_channel != 16)
        goto png_loadinfo_end;
      inf.channel = color - 2;
      if (!inf.channel) inf.channel = 3;
      break;
    default: 
      goto png_loadinfo_end;
  }
  {
    INFO_DO(fct.read(fct.user, buffer, 4) != 4);
    hash_crc32_end(&crc);
    uint32 *checksum = CAST(uint32*)buffer;
    *checksum = imath_flip32(*checksum);
    if (crc != *checksum)
      PRINT_WRN("Image: png header checksum wrong!");
  }
  
  
  if (color == 3) {
    uint32 chunk_len, chunk_type;
    uint32 *b32;
    int loop = 1;
    do {
      INFO_DO(fct.read(fct.user, buffer, 8) != 8);
      b32 = CAST(uint32*)buffer;
      chunk_len = imath_flip32(b32[0]);
      chunk_type = b32[1];
      hash_crc32_start(&crc);
      hash_crc32_appends(&crc, &chunk_type, 4);
      if (chunk_len) INFO_DO(fct.read(fct.user, buffer, chunk_len) != chunk_len);
#define CASE(A,B,C,D) case CAST(uint32)(A | (B << 8) | (C << 16) | (D << 24))
      switch (chunk_type) {
        CASE('I','E','N','D'): {
          if (chunk_len) goto png_loadinfo_end;
          loop = 0;
        } break;
        CASE('I','H','D','R'):
          goto png_loadinfo_end;
        default: // ignored unknown chunk
          break;
      }
      INFO_DO(fct.read(fct.user, buffer, 4) != 4);
      hash_crc32_appends(&crc, buffer, chunk_len);
      hash_crc32_end(&crc);
      b32 = CAST(uint32*)buffer;
      *b32 = imath_flip32(*b32);
      if (crc != *b32)
        PRINT_WRN("Image: png checksum wrong!");
#undef CASE
    } while (loop);
  }
  *i = IMAGE_SUCCESS;
png_loadinfo_end:
  return inf;
}

#define LOAD_DO(X) do { if ((X)) {\
  *i = IMAGE_FAILURE_WORK; \
  image_free(&img); \
  return img; \
} } while(0)
static image image__png_load(image_funct fct, image_cout *i) {
  image img = {0};
  LOAD_DO(fct.read(fct.user, buffer, 8) < 8);
  if (util_memcmp(s,"\137PNG\r\n\26\n", 8)) {
    *i = IMAGE_BAD_SIGN;
  }
  #define PNG_CHUNK(a,b,c,d)  (((unsigned) (a) << 24) + ((unsigned) (b) << 16) + ((unsigned) (c) << 8) + (unsigned) (d))
  for (;;) {
    stbi__pngchunk c = stbi__get_chunk_header(s);
    switch (c.type) {
       case PNG_CHUNK('I','H','D','R'): {
          int comp,filter;
          if (!first) return stbi__err("multiple IHDR","Corrupt PNG");
          first = 0;
          if (c.length != 13) return stbi__err("bad IHDR len","Corrupt PNG");
          s->img_x = stbi__get32be(s);
          s->img_y = stbi__get32be(s);
          if (s->img_y > STBI_MAX_DIMENSIONS) return stbi__err("too large","Very large image (corrupt?)");
          if (s->img_x > STBI_MAX_DIMENSIONS) return stbi__err("too large","Very large image (corrupt?)");
          z->depth = stbi__get8(s);  if (z->depth != 1 && z->depth != 2 && z->depth != 4 && z->depth != 8 && z->depth != 16)  return stbi__err("1/2/4/8/16-bit only","PNG not supported: 1/2/4/8/16-bit only");
          color = stbi__get8(s);  if (color > 6)         return stbi__err("bad ctype","Corrupt PNG");
          if (color == 3 && z->depth == 16)                  return stbi__err("bad ctype","Corrupt PNG");
          if (color == 3) pal_img_n = 3; else if (color & 1) return stbi__err("bad ctype","Corrupt PNG");
          comp  = stbi__get8(s);  if (comp) return stbi__err("bad comp method","Corrupt PNG");
          filter= stbi__get8(s);  if (filter) return stbi__err("bad filter method","Corrupt PNG");
          interlace = stbi__get8(s); if (interlace>1) return stbi__err("bad interlace method","Corrupt PNG");
          if (!s->img_x || !s->img_y) return stbi__err("0-pixel image","Corrupt PNG");
          if (!pal_img_n) {
             s->img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
             if ((1 << 30) / s->img_x / s->img_n < s->img_y) return stbi__err("too large", "Image too large to decode");
          } else {
             // if paletted, then pal_n is our final components, and
             // img_n is # components to decompress/filter.
             s->img_n = 1;
             if ((1 << 30) / s->img_x / 4 < s->img_y) return stbi__err("too large","Corrupt PNG");
          }
          // even with SCAN_header, have to scan to see if we have a tRNS
          break;
       }
       case PNG_CHUNK('C','g','B','I'):
          is_iphone = 1;
          stbi__skip(s, c.length);
          break;
  
       case PNG_CHUNK('P','L','T','E'):  {
          if (first) return stbi__err("first not IHDR", "Corrupt PNG");
          if (c.length > 256*3) return stbi__err("invalid PLTE","Corrupt PNG");
          pal_len = c.length / 3;
          if (pal_len * 3 != c.length) return stbi__err("invalid PLTE","Corrupt PNG");
          for (i=0; i < pal_len; ++i) {
             palette[i*4+0] = stbi__get8(s);
             palette[i*4+1] = stbi__get8(s);
             palette[i*4+2] = stbi__get8(s);
             palette[i*4+3] = 255;
          }
          break;
       }
  
       case PNG_CHUNK('t','R','N','S'): {
          if (first) return stbi__err("first not IHDR", "Corrupt PNG");
          if (z->idata) return stbi__err("tRNS after IDAT","Corrupt PNG");
          if (pal_img_n) {
             if (scan == STBI__SCAN_header) { s->img_n = 4; return 1; }
             if (pal_len == 0) return stbi__err("tRNS before PLTE","Corrupt PNG");
             if (c.length > pal_len) return stbi__err("bad tRNS len","Corrupt PNG");
             pal_img_n = 4;
             for (i=0; i < c.length; ++i)
                palette[i*4+3] = stbi__get8(s);
          } else {
             if (!(s->img_n & 1)) return stbi__err("tRNS with alpha","Corrupt PNG");
             if (c.length != (uint32) s->img_n*2) return stbi__err("bad tRNS len","Corrupt PNG");
             has_trans = 1;
             // non-paletted with tRNS = constant alpha. if header-scanning, we can stop now.
             if (scan == STBI__SCAN_header) { ++s->img_n; return 1; }
             if (z->depth == 16) {
                for (k = 0; k < s->img_n && k < 3; ++k) // extra loop test to suppress false GCC warning
                   tc16[k] = (uint16)stbi__get16be(s); // copy the values as-is
             } else {
                for (k = 0; k < s->img_n && k < 3; ++k)
                   tc[k] = (ubyte)(stbi__get16be(s) & 255) * stbi__depth_scale_table[z->depth]; // non 8-bit images will be larger
             }
          }
          break;
       }
  
       case PNG_CHUNK('I','D','A','T'): {
          if (first) return stbi__err("first not IHDR", "Corrupt PNG");
          if (pal_img_n && !pal_len) return stbi__err("no PLTE","Corrupt PNG");
          if (scan == STBI__SCAN_header) {
             // header scan definitely stops at first IDAT
             if (pal_img_n)
                s->img_n = pal_img_n;
             return 1;
          }
          if (c.length > (1u << 30)) return stbi__err("IDAT size limit", "IDAT section larger than 2^30 bytes");
          if ((int)(ioff + c.length) < (int)ioff) return 0;
          if (ioff + c.length > idata_limit) {
             uint32 idata_limit_old = idata_limit;
             ubyte *p;
             if (idata_limit == 0) idata_limit = c.length > 4096 ? c.length : 4096;
             while (ioff + c.length > idata_limit)
                idata_limit *= 2;
             STBI_NOTUSED(idata_limit_old);
             p = (ubyte *) STBI_REALLOC_SIZED(z->idata, idata_limit_old, idata_limit); if (p == NULL) return stbi__err("outofmem", "Out of memory");
             z->idata = p;
          }
          if (!stbi__getn(s, z->idata+ioff,c.length)) return stbi__err("outofdata","Corrupt PNG");
          ioff += c.length;
          break;
       }
  
       case PNG_CHUNK('I','E','N','D'): {
          uint32 raw_len, bpl;
          if (first) return stbi__err("first not IHDR", "Corrupt PNG");
          if (scan != STBI__SCAN_load) return 1;
          if (z->idata == NULL) return stbi__err("no IDAT","Corrupt PNG");
          // initial guess for decoded data size to avoid unnecessary reallocs
          bpl = (s->img_x * z->depth + 7) / 8; // bytes per line, per component
          raw_len = bpl * s->img_y * s->img_n /* pixels */ + s->img_y /* filter mode per row */;
          z->expanded = (ubyte *) stbi_zlib_decode_malloc_guesssize_headerflag((char *) z->idata, ioff, raw_len, (int *) &raw_len, !is_iphone);
          if (z->expanded == NULL) return 0; // zlib should set error
          STBI_FREE(z->idata); z->idata = NULL;
          if ((req_comp == s->img_n+1 && req_comp != 3 && !pal_img_n) || has_trans)
             s->img_out_n = s->img_n+1;
          else
             s->img_out_n = s->img_n;
          if (!stbi__create_png_image(z, z->expanded, raw_len, s->img_out_n, z->depth, color, interlace)) return 0;
          if (has_trans) {
             if (z->depth == 16) {
                if (!stbi__compute_transparency16(z, tc16, s->img_out_n)) return 0;
             } else {
                if (!stbi__compute_transparency(z, tc, s->img_out_n)) return 0;
             }
          }
          if (is_iphone && stbi__de_iphone_flag && s->img_out_n > 2)
             stbi__de_iphone(z);
          if (pal_img_n) {
             // pal_img_n == 3 or 4
             s->img_n = pal_img_n; // record the actual colors we had
             s->img_out_n = pal_img_n;
             if (req_comp >= 3) s->img_out_n = req_comp;
             if (!stbi__expand_png_palette(z, palette, pal_len, s->img_out_n))
                return 0;
          } else if (has_trans) {
             // non-paletted image with tRNS -> source image has (constant) alpha
             ++s->img_n;
          }
          STBI_FREE(z->expanded); z->expanded = NULL;
          // end of PNG chunk, read and skip CRC
          stbi__get32be(s);
          return 1;
       }
  
       default:
          // if critical, fail
          if (first) return stbi__err("first not IHDR", "Corrupt PNG");
          if ((c.type & (1 << 29)) == 0) {
             #ifndef STBI_NO_FAILURE_STRINGS
             // not threadsafe
             static char invalid_chunk[] = "XXXX PNG chunk not known";
             invalid_chunk[0] = STBI__BYTECAST(c.type >> 24);
             invalid_chunk[1] = STBI__BYTECAST(c.type >> 16);
             invalid_chunk[2] = STBI__BYTECAST(c.type >>  8);
             invalid_chunk[3] = STBI__BYTECAST(c.type >>  0);
             #endif
             return stbi__err(invalid_chunk, "PNG not supported: unknown PNG chunk type");
          }
          stbi__skip(s, c.length);
          break;
    }
    // end of PNG chunk, read and skip CRC
    stbi__get32be(s);
  }
  #undef PNG_CHUNK
  return img;
}


// mem function
static inline void im_restart(void *u) {
  void **user = CAST(void**)u;
  user[0] = user[1];
}
static inline int im_read(void *u, void *d, iter n) {
  return file_read(d, 1, n, CAST(FILE*)u);
}
static inline int im_seek(void *u, iter n) {
  void **user = CAST(void**)u;
  if (((CAST(byte*)user[0]) += n) >= CAST(byte*)user[2])
    user[0] = user[2];
  return 0;
}
static inline int im_write(void *u,const void *d, iter n) {
  void **user = CAST(void**)u;
  iter c = CAST(byte*)user[2] - CAST(byte*)user[1];
  user[0] = user[1] = util_realloc(user[1], c + n);
  if (!user[0]) return -1;
  util_memcpy(CAST(byte*)user[0] + c, d, n);
  user[2] = user[0] = CAST(byte*)user[1] + c + n;
  return 0;
}
static inline int im_eof(void *u) {
  void **user = CAST(void**)u;
  return user[0] >= user[2];
}
// file function
static inline void if_restart(void *u) {
  file_rewind(CAST(FILE*)u);
}
static inline int if_read(void *u, void *d, iter n) {
  return file_read(d, 1, n, CAST(FILE*)u);
}
static inline int if_seek(void *u, iter n) {
  return file_seek(n, CAST(FILE*)u);
}
static inline int if_write(void *u,const void *d, iter n) {
  return file_write(d, 1, n, CAST(FILE*)u);
}
static inline int if_eof(void *u) {
  return file_eof(CAST(FILE*)u);
}

// surface
inline void image_switch_pref (const image_pref_flag f) {
  enabled_flags ^= f;
}

static void *image__load_core(image__context *s, int *x, int *y, int *comp, int req_comp, image__result_info *ri, int bpc) {
  image__png_load(s,x,y,comp,req_comp, ri);
  image__bmp_load(s,x,y,comp,req_comp, ri);
  image__psd_load(s,x,y,comp,req_comp, ri, bpc);
  image__pic_load(s,x,y,comp,req_comp, ri);
  image__jpeg_load(s,x,y,comp,req_comp, ri);
  image__pnm_load(s,x,y,comp,req_comp, ri);
  image__hdr_to_ldr(image__hdr_load(s, x,y,comp,req_comp, ri), *x, *y, req_comp ? req_comp : *comp);
  image__tga_load(s,x,y,comp,req_comp, ri);
  return image__errpuc("unknown image type", "Image not of any known type, or corrupt");
}

image_info image_loadinfo (image_funct fct, image_cout *i) {
  image_info inf = {0};
  image_cout r = image__load_core(fct);
  if (stbi__jpeg_info(s, x, y, comp)) return 1;
  if (stbi__png_info(s, x, y, comp))  return 1;
  if (stbi__gif_info(s, x, y, comp))  return 1;
  if (stbi__bmp_info(s, x, y, comp))  return 1;
  if (stbi__psd_info(s, x, y, comp))  return 1;
  if (stbi__pic_info(s, x, y, comp))  return 1;
  if (stbi__pnm_info(s, x, y, comp))  return 1;
  if (stbi__hdr_info(s, x, y, comp))  return 1;
  if (stbi__tga_info(s, x, y, comp))  return 1;
  return stbi__err("unknown image type", "Image not of any known type, or corrupt");
}
  image_info inf = {0};
  image_cout r = IMAGE_SUCCESS;
  inf = image__png_loadinfo(fct, &r);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  inf = image__bmp_loadinfo(s,x,y,comp,req_comp, ri);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  inf = image__psd_loadinfo(s,x,y,comp,req_comp, ri, bpc);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  inf = image__pic_loadinfo(s,x,y,comp,req_comp, ri);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  inf = image__jpeg_loadinfo(s,x,y,comp,req_comp, ri);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  inf = image__pnm_loadinfo(s,x,y,comp,req_comp, ri);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  inf = image__hdr_to_ldr(image__hdr_load(s, x,y,comp,req_comp, ri), *x, *y, req_comp ? req_comp : *comp);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  r = IMAGE_SUCCESS;
  inf = image__tga_loadinfo(s,x,y,comp,req_comp, ri);
loadimage_end:
  if (i) *i = r;
  return inf;
}
image_info image_loadinfo_mem (void*, iter, image_cout*);
image_info image_loadinfo_file (const char*, image_cout*);

image image_loadimage (image_funct fct, image_cout *i) {
  image img = {0};
  image_cout r = IMAGE_SUCCESS;
  img = image__png_load(fct, &r);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  fct.restart(fct.user);
  img = image__bmp_load(s,x,y,comp,req_comp, ri);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  fct.restart(fct.user);
  img = image__psd_load(s,x,y,comp,req_comp, ri, bpc);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  fct.restart(fct.user);
  img = image__pic_load(s,x,y,comp,req_comp, ri);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  fct.restart(fct.user);
  img = image__jpeg_load(s,x,y,comp,req_comp, ri);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  fct.restart(fct.user);
  img = image__pnm_load(s,x,y,comp,req_comp, ri);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  fct.restart(fct.user);
  img = image__hdr_to_ldr(image__hdr_load(s, x,y,comp,req_comp, ri), *x, *y, req_comp ? req_comp : *comp);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  fct.restart(fct.user);
  img = image__tga_load(s,x,y,comp,req_comp, ri);
  if (r != IMAGE_BAD_SIGN) goto loadimage_end;
  r = IMAGE_SUCCESS;
loadimage_end:
  if (i) *i = r;
  return img;
}
image image_loadimage_mem (void *data, iter n, image_cout *i) {
  image img = {0};
  image_cout r = IMAGE_SUCCESS;
  void *user[3] = {data,data,((CAST(byte*)data)+n)};
  image_funct F = {
    .user = user, 
    .restart = im_restart,
    .read = im_read,
    .write = im_write,
    .seek = im_seek,
    .eof = im_eof,
  };
  img = image_loadimage(F, &r);
  if (!i) *i = r;
  return img;
}
image image_loadimage_file (const char *filename, image_cout *i) {
  image img = {0};
  image_cout r = IMAGE_SUCCESS;
  FILE *file = file_open(filename, "w");
  if (file) {
    image_funct F = {
      .user = file,
      .restart = if_restart,
      .read = if_read,
      .write = if_write,
      .seek = if_seek,
      .eof = if_eof,
    };
    img = image_loadimage(F, &r);
    file_close(file);
  } else r = IMAGE_FAILURE_FILE;
  if (!i) *i = r;
  return img;
}

image_cout image_write (image_funct, const image);
image_cout image_write_mem (void*, iter, const image);
image_cout image_write_file (const char*, const image);

void image_free(image *x) {
  util_memfree(x->image_data);
  util_memset(x, 0, sizeof(image));
}
