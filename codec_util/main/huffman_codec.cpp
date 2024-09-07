#include "huffman_codec.hpp"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>

extern bool codec_assert;

// Node structure for Huffman Tree
struct Node {
  virtual uint32_t frequency () const {
    return 0;
  }
  virtual uint32_t type () const {
    return 0;
  }
  // Comparator for priority queue
  struct compare {
    bool operator() (Node *l, Node *r) {
      return l->frequency () > r->frequency ();
    }
  };
};
struct Branch : public Node {
  Node *left,
      *right;
  Branch (Node *l, Node *r) : left (l), right (r) {}

  uint32_t frequency () const override {
    return left->frequency () + right->frequency ();
  }
  uint32_t type () const override {
    return 2;
  }
  ~Branch () {
    delete left;
    delete right;
  }
};
struct Leaf : public Node {
  uint8_t data;
  uint32_t freq;
  Leaf (uint8_t d, uint32_t f) : data (d), freq (f) {}

  uint32_t frequency () const override {
    return freq;
  }
  uint32_t type () const override {
    return 1;
  }
};

// Utility function to build Huffman Tree and store the codes
void buildHuffmanTree (Node *root, std::vector<bool> code, std::unordered_map<uint8_t, std::vector<bool>> &huffmanCode) {
  switch (root->type ()) {
  case 1:
    // Leaf node
    huffmanCode[((Leaf *)root)->data] = code;
    break;
  case 2: {
    Branch *b = (Branch *)root;
    std::vector<bool> code_left = code;
    code_left.push_back (false);
    buildHuffmanTree (b->left, code_left, huffmanCode);
    std::vector<bool> code_right = code;
    code_right.push_back (true);
    buildHuffmanTree (b->right, code_right, huffmanCode);
    break;
  }
  }
}

// Function to encode data using Huffman coding
const codec_data huffman_encode (codec_data const &cd) {
  uint8_t temp;
  // Encode Huffman codes
  codec_data out_c;

  // store frequency each data
  // store 32bit as key, and 16bit for at least 65535 copy
  std::unordered_map<uint8_t, uint32_t> freq;
  // Count frequency of each character
  size_t data_len = 0;
  for (codec_data::reader ro = cd.begin_read (); ro.left ();) {
    ro >> temp;
    ++freq[temp];
    ++data_len;
  }
  // write actual 32bit data size and variations of key frequecy
  out_c << data_len << size_t (freq.size ());
  // Create priority queue to store live nodes of Huffman tree
  std::priority_queue<Node *, std::vector<Node *>, Node::compare> pq;

  std::cout << "Keys \n";

  for (std::pair<uint8_t, uint32_t> pair : freq) {
    // Write huffman tree
    out_c << pair.first;
    codec_assert = true;
    out_c << pair.second;
    codec_assert = false;
    std::cout << std::setw (2) << std::setfill ('0') << std::hex << int (pair.first);
    std::cout << std::dec << "[" << std::setw (3) << std::setfill ('0') << pair.second << "]  ";
    // Create leaf nodes for each character and add it to the priority queue
    pq.push (new Leaf (pair.first, pair.second));
  }
  // Create Huffman tree
  while (pq.size () > 1) {
    Node *left = pq.top ();
    pq.pop ();
    Node *right = pq.top ();
    pq.pop ();
    pq.push (new Branch (left, right));
  }
  // Traverse the Huffman tree and store Huffman codes in a map
  std::unordered_map<uint8_t, std::vector<bool>> huffmanCode;
  buildHuffmanTree (pq.top (), std::vector<bool> (), huffmanCode);
  delete pq.top ();

  // Encode input data using Huffman codes
  for (codec_data::reader ro = cd.begin_read (); ro.left ();) {
    ro >> temp;
    for (bool s : huffmanCode[temp])
      out_c << s;
  }
  return out_c;
}

const codec_data huffman_decode (codec_data const &cd) {
  codec_data::reader ro = cd.begin_read ();
  size_t len_data, variations;
  ro >> len_data >> variations;
  uint8_t key;
  uint32_t key_len;

  std::cout << "Keys \n";

  // Create priority queue to store live nodes of Huffman tree
  std::priority_queue<Node *, std::vector<Node *>, Node::compare> pq;
  for (unsigned i = 0; i < variations; ++i) {
    ro >> key;
    codec_assert = true;
    ro >> key_len;
    codec_assert = false;
    std::cout << std::setw (2) << std::setfill ('0') << std::hex << int (key);
    std::cout << std::dec << "[" << std::setw (3) << std::setfill ('0') << key_len << "]  ";
    pq.push (new Leaf (key, key_len));
  }
  std::cout << std::endl;
  // Create Huffman tree
  while (pq.size () > 1) {
    Node *left = pq.top ();
    pq.pop ();
    Node *right = pq.top ();
    pq.pop ();
    pq.push (new Branch (left, right));
  }
  Branch *tree = (Branch *)pq.top ();
  codec_data out_c;
  // decode input data using Huffman codes
  bool bit_read;
  for (unsigned i = 0; i < len_data; ++i) {
    Branch *current_branch = tree;
    while (ro.left ()) {
      ro >> bit_read;
      Node *cur_ = bit_read ? current_branch->right : current_branch->left;
      if (cur_->type () == 1) {
        out_c << ((Leaf *)cur_)->data;
        break;
      }
      current_branch = (Branch *)cur_;
    }
  }
  delete tree;
  return out_c;
}