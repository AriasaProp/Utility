#include "codec_util.hpp"

#include "clock_adjustment.hpp"
#include "random.hpp"

#include <cstdint>
#include <iostream>
/*
struct test_result {
  bool success;
  unsigned long time_encode, time_decode; // ms
  double comp_ratio;                      // %
  ~test_result() {
    //
  }
};

const test_result test_codec (const char *name, const codec_data &in, const codec_data (*encode) (codec_data const &), const codec_data (*decode) (codec_data const &)) {
  test_result r;
  profiling::clock_adjustment clck = profiling::clock_adjustment (name);
  // encoding data
  const codec_data encode_result = encode (in);
  r.time_encode = clck.get_clock (profiling::clock_adjustment::period::microseconds);
  // decoding data
  const codec_data decode_result = decode (encode_result);
  r.time_decode = clck.get_clock (profiling::clock_adjustment::period::microseconds);
  // compare
  r.success = decode_result == in;
  r.comp_ratio = 100.00 - 100.00 * double (encode_result.size_bit () / in.size_bit ());
  return r;
}
*/
#define TRY 100

int main (int argv, char *args[]) {
  try {
    for (unsigned tries = 0; tries < TRY; ++tries) {
      codec_data cd;
      uint32_t max = random_uint32 ();
      while ((max > 1800) && (max & 31)) {
        max = (max & 0xffff) + (max >> 16);
      }
      std::cout << "Hello wolrd! "<< max << std::endl;
      
      /*
      // try make random data
      for (unsigned dat = 0; dat < max; dat += 32)
        cd << random_uint32 ();
      std::cout << cd << std::endl;
      */
    }
  } catch (const char *err) {
    std::cout << "Error : " << err << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cout << "Error : uncaught" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}