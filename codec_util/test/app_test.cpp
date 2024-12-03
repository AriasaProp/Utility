#include "codec_util.hpp"
#include "huffman_codec.hpp"
#include "no_codec.hpp"
#include "qoi_codec.hpp"

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <utility>
#include <vector>

struct test_result {
  std::string name, data;
  bool success;
  unsigned long long time_encode, time_decode; // time report ns
  double comp_ratio;                           // %
};

const test_result test_codec (std::pair<std::string, codec_data> data_n, std::pair<std::string, std::pair<const codec_data (*) (codec_data const &), const codec_data (*) (codec_data const &)>> codec_n) {
  test_result r;
  r.name = codec_n.first;
  r.data = data_n.first;
  // encoding data
  std::chrono::time_point<std::chrono::high_resolution_clock> startc = std::chrono::high_resolution_clock::now ();
  const codec_data encode_result = codec_n.second.first (data_n.second);
  r.time_encode = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::high_resolution_clock::now () - startc).count ();
  // decoding data
  startc = std::chrono::high_resolution_clock::now ();
  const codec_data decode_result = codec_n.second.second (encode_result);
  r.time_decode = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::high_resolution_clock::now () - startc).count ();
  // compare
  r.success = decode_result == data_n.second;
  r.comp_ratio = 100.00 - 100.00 * double (encode_result.size_bit ()) / double (data_n.second.size_bit ());
  return r;
}

#define TRY 5
constexpr size_t CODEC_SIZE = 2048 * 2048;

int main (int argv, char *args[]) {
  std::cout << "Start Codec Test" << std::endl;
  try {
    std::vector<test_result> rss;
    {
      std::random_device rd;
      std::uniform_int_distribution<uint32_t> clr (0x0, 0xffffffff);
      // make codec variations
      std::map<std::string, std::pair<const codec_data (*) (codec_data const &), const codec_data (*) (codec_data const &)>> codec_var{
          {"-", {no_encode, no_decode}},
          {"Huffman", {huffman_encode, huffman_decode}},
          {"QOI", {qoi_encode, qoi_decode}}};
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
    // Draw table header
    std::cout << "   codec   ||  data type  || res ||    encode    ||    decode    ||  ratio  |\n";
    std::cout << "-----------||-------------||-----||--------------||--------------||---------|\n";
    static const long long units[]{
        1000, // ns
        1000, // us
        1000, // ms
        60,   // s
        60,   // M
        24    // H
    };
    static const char *const uname[]{"ns", "us", "ms", "s", "M", "H", "D"};

    for (test_result rs : rss) {
      // name
      std::cout << " " << std::setfill (' ') << std::setw (9) << rs.name;
      std::cout << " ||  ";
      std::cout << std::setfill (' ') << std::setw (10) << rs.data;
      std::cout << " || " << (rs.success ? "√" : "×") << "  || ";
      {
        long long dcast = rs.time_encode;
        unsigned int lcast = 0;
        std::cout << std::setprecision (2);
        for (const long long &u : units) {
          if (dcast < u)
            break;
          dcast /= u;
          ++lcast;
        }
        std::cout << std::setfill (' ') << std::setw (11) << dcast << " " << uname[lcast];
      }
      std::cout << " || ";
      {
        long long dcast = rs.time_decode;
        unsigned int lcast = 0;
        for (const long long &u : units) {
          if (dcast < u)
            break;
          dcast /= u;
          ++lcast;
        }
        std::cout << std::setfill (' ') << std::setw (11) << dcast << " " << uname[lcast];
      }
      std::cout << " || ";
      std::cout << std::setfill (' ') << std::setw (6) << rs.comp_ratio;
      std::cout << " % |" << std::endl;
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