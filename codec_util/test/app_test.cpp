#include "codec_util.hpp"
#include "huffman_codec.hpp"
#include "qoi_codec.hpp"

#include "clock_adjustment.hpp"

#include <cstdint>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <utility>
#include <vector>

struct test_result {
  std::string name;
  bool success;
  unsigned long time_encode, time_decode; // ms
  double comp_ratio;                      // %
  void print () {
    std::cout << name << " : " << (success ? "success" : "fail") << " encode: " << time_encode << " ms, decode: " << time_decode << " ms, compress ratio: " << comp_ratio << " %" << std::endl;
  }
};

const test_result test_codec (std::pair<std::string, codec_data> data_n, std::pair<std::string, std::pair<const codec_data (*) (codec_data const &), const codec_data (*) (codec_data const &)>> codec_n) {
  test_result r;
  r.name = "data(" + data_n.first + "), codec(" + codec_n.first + ")";
  profiling::clock_adjustment clck = profiling::clock_adjustment (r.name.c_str ());
  // encoding data
  const codec_data encode_result = codec_n.second.first (data_n.second);
  r.time_encode = clck.get_clock (profiling::clock_adjustment::period::microseconds);
  // decoding data
  const codec_data decode_result = codec_n.second.second (encode_result);
  r.time_decode = clck.get_clock (profiling::clock_adjustment::period::microseconds);
  // compare
  r.success = decode_result == data_n.second;
  r.comp_ratio = 100.00 - 100.00 * double (encode_result.size_bit ()) / double (data_n.second.size_bit ());
  return r;
}

#define TRY 5
constexpr size_t CODEC_SIZE = 2048 * 2048;

int main (int argv, char *args[]) {
  try {
    std::vector<test_result> rss;
    {
      std::random_device rd;
      std::uniform_int_distribution<uint32_t> clr (0x0, 0xffffffff);
      // make codec variations
      std::map<std::string, std::pair<const codec_data (*) (codec_data const &), const codec_data (*) (codec_data const &)>> codec_var{
          {"Huffman", {huffman_encode, huffman_decode}}, {"Huffman2", {huffman_encode, huffman_decode}},
          {"QOI", {qoi_encode, qoi_decode}}, {"QOI2", {qoi_encode, qoi_decode}},
      	
      };
      // do codec
      {
        uint32_t rdmA[10];
        for (size_t i = 0; i < 10; ++i) {
          rdmA[i] = clr (rd);
        }
        {
          std::pair<std::string, codec_data> d{
              "var10_a",
              codec_data (CODEC_SIZE << 2)};
          for (size_t i = 0; i < CODEC_SIZE; ++i)
            d.second << rdmA[clr (rd) % 10];
          for (std::pair<std::string, std::pair<const codec_data (*) (codec_data const &), const codec_data (*) (codec_data const &)>> cod : codec_var) {
            rss.push_back (test_codec (d, cod));
          }
        }
        {
          std::pair<std::string, codec_data> d{
              "var10_b",
              codec_data (CODEC_SIZE << 2)};
          for (size_t i = 0; i < CODEC_SIZE; ++i)
            d.second << rdmA[clr (rd) % 10];
          for (std::pair<std::string, std::pair<const codec_data (*) (codec_data const &), const codec_data (*) (codec_data const &)>> cod : codec_var) {
            rss.push_back (test_codec (d, cod));
          }
        }
      }
      {
        uint32_t rdmA[100];
        for (size_t i = 0; i < 100; ++i) {
          rdmA[i] = clr (rd);
        }
        {
          std::pair<std::string, codec_data> d{
              "var100_a",
              codec_data (CODEC_SIZE << 2)};
          for (size_t i = 0; i < CODEC_SIZE; ++i)
            d.second << rdmA[clr (rd) % 10];
          for (std::pair<std::string, std::pair<const codec_data (*) (codec_data const &), const codec_data (*) (codec_data const &)>> cod : codec_var) {
            rss.push_back (test_codec (d, cod));
          }
        }
        {
          std::pair<std::string, codec_data> d{
              "var100_b",
              codec_data (CODEC_SIZE << 2)};
          for (size_t i = 0; i < CODEC_SIZE; ++i)
            d.second << rdmA[clr (rd) % 10];
          for (std::pair<std::string, std::pair<const codec_data (*) (codec_data const &), const codec_data (*) (codec_data const &)>> cod : codec_var) {
            rss.push_back (test_codec (d, cod));
          }
        }
      }
      {
        std::pair<std::string, codec_data> d{
            "var_noise",
            codec_data (CODEC_SIZE << 2)};
        for (size_t i = 0; i < CODEC_SIZE; ++i)
          d.second << clr (rd);
        for (std::pair<std::string, std::pair<const codec_data (*) (codec_data const &), const codec_data (*) (codec_data const &)>> cod : codec_var) {
          rss.push_back (test_codec (d, cod));
        }
      }
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