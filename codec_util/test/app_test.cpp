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
  void print () {
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
  r.comp_ratio = 100.00 - 100.00 * double (encode_result.size_bit ()) / double (in.size_bit ());
  return r;
}

#define TRY 5
#define CODEC_SIZE 80

int main (int argv, char *args[]) {
  /*
  {
    unsigned char ct;
    codec_data cd;
    cd << false << false;
    cd << (ct = 0xba);
    cd << true << false;
    cd << (ct = 0x2b);
    cd << false << true;
    cd << (ct = 0x9c);
    cd << true << true;
    cd << (ct = 0xff);
    codec_data::reader ro = cd.begin_read ();
    bool a, b;
    ro >> a >> b >> ct;
    std::cout << int (a | (b << 1)) << " " << std::hex << int (ct) << std::endl;
    ro >> a >> b >> ct;
    std::cout << int (a | (b << 1)) << " " << std::hex << int (ct) << std::endl;
    ro >> a >> b >> ct;
    std::cout << int (a | (b << 1)) << " " << std::hex << int (ct) << std::endl;
    ro >> a >> b >> ct;
    std::cout << int (a | (b << 1)) << " " << std::hex << int (ct) << std::endl;
  }
  */
  try {
    std::vector<test_result> rss;
    std::random_device rd;
    std::uniform_int_distribution<uint32_t> clr (0x0, 0xffffffff);
    uint32_t rdmA[10] {
      clr(rd),
      clr(rd),
      clr(rd),
      clr(rd),
      clr(rd),
      clr(rd),
      clr(rd),
      clr(rd),
      clr(rd),
      clr(rd)
    };
    for (size_t i = 0; i < TRY; ++i) {
      codec_data cd (CODEC_SIZE << 2);
      // try make random data
      for (size_t j = 0; j < CODEC_SIZE; ++j) {
        cd << rdmA[clr (rd) % 10];
      }
      rss.push_back (test_codec ("huffman", cd, huffman_encode, huffman_decode));
    }

    for (test_result rs : rss) {
      rs.print ();
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