#include "qoi_codec.hpp"

union qoi_t {
	struct{
		uint8_t r,g,b,a;
	} rgba;
	uint32_t clr;
};

bool operator==(qoi_t a, qoi_t b) {
	return a.clr == b.clr;
}

#include <cstring>

#define QOI_OP_INDEX 0x00 /* 00xxxxxx */
#define QOI_OP_DIFF 0x40  /* 01xxxxxx */
#define QOI_OP_LUMA 0x80  /* 10xxxxxx */
#define QOI_OP_RUN 0xc0   /* 11xxxxxx */
#define QOI_OP_RGB 0xfe   /* 11111110 */
#define QOI_OP_RGBA 0xff  /* 11111111 */

#define QOI_MASK_1 0x3f /* 00111111 */
#define QOI_MASK_2 0xc0 /* 11000000 */

#define QOI_COLOR_HASH(C) (C.rgba.r * 3 + C.rgba.g * 5 + C.rgba.b * 7 + C.rgba.a * 11) % 64
#define QOI_MAGIC                                  \
  (((uint32_t)'q') << 24 | ((uint32_t)'o') << 16 | \
   ((uint32_t)'i') << 8 | ((uint32_t)'f'))

static const uint8_t qoi_padding[8] = {0, 0, 0, 0, 0, 0, 0, 1};

const codec_data qoi_encode (codec_data const &in) {
  codec_data out;
  qoi_t index[64]{}; // store code library = 0x3f => 0x11_1111b

  uint8_t run = 0; // store run_len code
  qoi_t px_prev {.clr = 0xff000000 }, px = px_prev;
  
  uint8_t bd; // store to out as byte, 32bit
  
	for (codec_data::reader ro = in.begin_read (); ro.left () || run;) {
		
    ro >> px.clr;
    
    if (((px.clr == px_prev.clr) && (++run == 62)) || !ro.left ()) {
  	// run length codec
    	bd = QOI_OP_RUN | (run - 1);
    	out << bd;
      run = 0;
      continue;
    } else if (run) {
  	// run_length end
      bd = QOI_OP_RUN | (run - 1);
    	out << bd;
      run = 0;
    }
      
    uint8_t index_pos = QOI_COLOR_HASH (px);
    if (index[index_pos] == px) {
    // write down to code library index
      bd = QOI_OP_INDEX | index_pos;
      out << bd;
    } else {
    // write down to library index
      index[index_pos] = px;
      // check alpha
      if (px.rgba.a == px_prev.rgba.a) {
        signed char vr = px.rgba.r - px_prev.rgba.r;
        signed char vg = px.rgba.g - px_prev.rgba.g;
        signed char vb = px.rgba.b - px_prev.rgba.b;

        signed char vg_r = vr - vg;
        signed char vg_b = vb - vg;

        if (
            vr > -3 && vr < 2 &&
            vg > -3 && vg < 2 &&
            vb > -3 && vb < 2) {
          bd = QOI_OP_DIFF | (vr + 2) << 4 | (vg + 2) << 2 | (vb + 2);
          out << bd;
        } else if (
            vg_r > -9 && vg_r < 8 &&
            vg > -33 && vg < 32 &&
            vg_b > -9 && vg_b < 8) {
          bd = QOI_OP_LUMA | (vg + 32);
          out << bd;
          bd = (vg_r + 8) << 4 | (vg_b + 8);
          out << bd;
        } else {
          bd = QOI_OP_RGB;
          out << bd;
          bd = px.rgba.r;
          out << bd;
          bd = px.rgba.g;
          out << bd;
          bd = px.rgba.b;
          out << bd;
        }
      } else {
      	// write full code
        bd = QOI_OP_RGBA;
        out << bd;
        out << px.clr;
      }
    }
  	px_prev = px;
  }
  return out;
}

const codec_data qoi_decode (codec_data const &in) {
	codec_data out;
	qoi_t index[64]{}; // store code library = 0x3f => 0x11_1111b

	qoi_t px {.clr = 0xff000000};
	uint8_t run, b1;
	for (codec_data::reader ro = in.begin_read (); ro.left ();) {
    ro >> b1;
    if (b1 == QOI_OP_RGB) {
      ro >> px.rgba.r;
      ro >> px.rgba.g;
      ro >> px.rgba.b;
    } else if (b1 == QOI_OP_RGBA) {
    	ro >> px.clr;
    } else if ((b1 & QOI_MASK_2) == QOI_OP_INDEX) {
      px = index[b1 & QOI_MASK_1];
    } else if ((b1 & QOI_MASK_2) == QOI_OP_DIFF) {
      px.rgba.r += ((b1 >> 4) & 0x03) - 2;
      px.rgba.g += ((b1 >> 2) & 0x03) - 2;
      px.rgba.b += (b1 & 0x03) - 2;
    } else if ((b1 & QOI_MASK_2) == QOI_OP_LUMA) {
      int vg = (b1 & QOI_MASK_1) - 0x20;
      ro >> b1;
      px.rgba.g += vg;
      px.rgba.b += vg - 8 + (b1 & 0x0f);
      b1 >>= 4;
      px.rgba.r += vg - 8 + (b1 & 0x0f);
    } else if ((b1 & QOI_MASK_2) == QOI_OP_RUN) {
      run = (b1 & QOI_MASK_1);
    }
    index[QOI_COLOR_HASH (px)] = px;
    
    do {
    	out << px.clr;
    } while (--run);
  }

  return out;
}

