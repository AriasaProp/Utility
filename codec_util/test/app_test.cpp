#include "codec_util.hpp"

#include "clock_adjustment.hpp"
#include "huffman_codec.hpp"
#include <random>
#include <vector>

#include <cstdint>
#include <iomanip>
#include <iostream>

struct test_result {
  std::string name;
  bool success;
  unsigned long time_encode, time_decode; // ms
  double comp_ratio;                      // %
  ~test_result () {
    std::cout << name << " : " << (success ? "success" : "fail") << " encode: " << time_encode << " ms, decode: " << time_decode << " ms, compress ratio: " << comp_ratio << " %" << std::endl;
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

#define TRY 3
#define CODEC_SIZE 8
// 4194304

int main (int argv, char *args[]) {
  try {
    std::vector<test_result> rss;
    std::random_device rd;
    std::uniform_int_distribution<uint32_t> clr (0x0, 0xffffffff);
    for (size_t i = 0; i < TRY; ++i) {
      codec_data cd (CODEC_SIZE << 2);
      // try make random data
      for (size_t j = 0; j < CODEC_SIZE; ++j) {
        uint32_t rndclr = clr (rd);
        cd << rndclr;
        std::cout << std::hex << rndclr << " ";
      }
      std::cout << std::dec << std::endl;
      std::cout << cd << std::endl;
      test_result rs = test_codec ("huffman", cd, huffman_encode, huffman_decode);
      rss.push_back (rs);
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