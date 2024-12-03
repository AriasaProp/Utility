#include "no_codec.hpp"


const codec_data no_encode (codec_data const &in) {
	codec_data out;
	uint32_t px;
  for (codec_data::reader ro = in.begin_read (); ro.left ();) {
  	ro >> px;
  	out << px;
  }
	return out;
}

const codec_data no_decode (codec_data const &in) {
	codec_data out;
	uint32_t px;
  for (codec_data::reader ro = in.begin_read (); ro.left ();) {
  	ro >> px;
  	out << px;
  }
	return out;
}