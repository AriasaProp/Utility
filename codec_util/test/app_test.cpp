#include "codec_util.hpp"

#include "huffman_codec.hpp"

#include "clock_adjustment.hpp"
#include "random.hpp"

#include <iostream>
#include <cstdint>

struct test_result {
	bool success;
	unsigned long time_encode, time_decode; // ms
	double comp_ratio; // %
};

const test_result test_codec(const char *name, const codec_data &in, const codec_data(*encode)(const codec_data &), const codec_data(*decode)(const codec_data &)) {
	test_result r;
	profiling::clock_adjustment clck = profiling::clock_adjustment(name);
	//encoding data
	const codec_data encode_result = encode(in);
	r.time_encode = clck.get_clock(profiling::clock_adjustment::period::microseconds);
	//decoding data
	const codec_data decode_result = decode(encode_result);
	r.time_decode = clck.get_clock(profiling::clock_adjustment::period::microseconds);
	//compare
	r.success = decode_result == in;
	r.comp_ratio = 100.00 - 100.00 * double(encode_result.size_in_bit/in.size_in_bit);
	return r;
}

#define TRY 100

int main (int argv, char *args[]) {
  try {
	  for (unsigned tries = 0; tries < TRY; ++tries) {
	  	codec_data cd;
	  	uint32_t max = random_uint32();
	  	while ((max > 1800) && (max & 31)) {
	  		max = (max & 0xffff) + (max >> 16);
	  	}
  		// try make random data
	  	for (unsigned dat = 0; dat < max; dat += 32)
	  		cd << random_uint32();
	  	std::cout << cd << std::endl;
	  	//huffman
	  	/*
	  	{
	  		test_result huffman test_codec("HUFFMAN Codec",cd, huffman_endcode, huffman_decode);
	  	}
	  	*/
	  }
  } catch (const char *err) {
  	std::cout << "Error :" << err << std::endl;
  	
  	return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}