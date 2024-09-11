#include "arithmatic_codec.hpp"

#include <unordered_map>
/*
struct Node {
  virtual uint32_t length() { return 0;}
  virtual size_t type() { return 0;}
};
struct Key: public Node {
  uint8_t key;
  uint32_t len;
  Key(uint8_t k, uint32_t l): key(k), len(l) {}
  uint32_t length() override { return 0;}
  size_t type() override { return 1;}
};
struct Eof : public Node{
  uint32_t length() override { return 1;}
  size_t type() override { return 2;}
};

const codec_data arithmetic_encode (codec_data const &cd) {
  uint8_t temp;
  // store frequency each data
  // store 32bit as key, and 16bit for at least 65535 copy
  std::unordered_map<uint8_t, uint32_t> freq;
  // Count frequency of each character
  for (codec_data::reader ro = cd.begin_read (); ro.left ();) {
    ro >> temp;
    ++freq[temp];
  }
  // Encode Huffman codes
  codec_data out_c;
  // write actual 32bit variations of key frequecy
  out_c << size_t (freq.size ());

  Node *tree;

  for (std::pair<uint8_t, uint32_t> pair : freq) {
    // Write huffman tree
    out_c << pair.first << pair.second;
  }
  tree.push_back(Eof());
  return out_c;
}

const codec_data arithmetic_decode (codec_data const &cd) {
  codec_data::reader ro = cd.begin_read();
  size_t variations;
  ro >> variations;
  uint8_t key;
  uint32_t key_len;

  // store 32bit as key, and 16bit for at least 65535 copy
  std::unordered_map<uint8_t, uint32_t> freq;
  for (unsigned i = 0; i < variations; ++i) {
    // Write freq
    ro >> key >> key_len;
    freq[key] = key_len;
  }

  codec_data out_c;

  return out_c;
}
*/