#include "huffman_codec.hpp"

#include <cstddef>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>

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
struct Eof_ : public Node {
  uint32_t frequency () const override {
    return 0;
  }
  uint32_t type () const override {
    return 0;
  }
};

// Utility function to build Huffman Tree and store the codes
void buildHuffmanTree (Node *root, std::vector<bool> code, std::unordered_map<uint8_t, std::vector<bool>> &huffmanCode, std::vector<bool> &eof_code) {
  switch (root->type ()) {
  case 0:
    break;
  case 1:
    // Leaf node
    huffmanCode[((Leaf *)root)->data] = code;
    break;
  case 2: {
    Branch *b = (Branch *)root;
    std::vector<bool> code_left = code;
    code_left.push_back (false);
    buildHuffmanTree (b->left, code_left, huffmanCode, eof_code);
    std::vector<bool> code_right = code;
    code_right.push_back (true);
    buildHuffmanTree (b->right, code_right, huffmanCode, eof_code);
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
  for (codec_data::reader ro = cd.begin_read (); ro.left ();) {
    ro >> temp;
    ++freq[temp];
  }
  // write actual 32bit data size and variations of key frequecy
  out_c << size_t (freq.size ());
  // Create priority queue to store live nodes of Huffman tree
  std::priority_queue<Node *, std::vector<Node *>, Node::compare> pq;

  for (std::pair<uint8_t, uint32_t> pair : freq) {
    // Write huffman tree
    out_c << pair.first << pair.second;
    //  Create leaf nodes for each character and add it to the priority queue
    pq.push (new Leaf (pair.first, pair.second));
  }
  pq.push (new Eof_);
  //  Create Huffman tree
  while (pq.size () > 1) {
    Node *left = pq.top ();
    pq.pop ();
    Node *right = pq.top ();
    pq.pop ();
    pq.push (new Branch (left, right));
  }
  // Traverse the Huffman tree and store Huffman codes in a map
  std::vector<bool> eof_code;
  std::unordered_map<uint8_t, std::vector<bool>> huffmanCode;
  buildHuffmanTree (pq.top (), std::vector<bool> (), huffmanCode, eof_code);
  delete pq.top ();

  // Encode input data using Huffman codes
  for (codec_data::reader ro = cd.begin_read (); ro.left ();) {
    ro >> temp;
    for (bool s : huffmanCode[temp])
      out_c << s;
  }
  for (bool s : eof_code)
    out_c << s;
  return out_c;
}

const codec_data huffman_decode (codec_data const &cd) {
  codec_data::reader ro = cd.begin_read ();
  size_t variations;
  ro >> variations;
  uint8_t key;
  uint32_t key_len;

  // Create priority queue to store live nodes of Huffman tree
  std::priority_queue<Node *, std::vector<Node *>, Node::compare> pq;
  for (unsigned i = 0; i < variations; ++i) {
    ro >> key >> key_len;
    pq.push (new Leaf (key, key_len));
  }
  pq.push (new Eof_);

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
  bool bit_read, eof_c = false;
  Branch *current_branch = tree;
  Node *cur_;
  do {
    ro >> bit_read;
    cur_ = bit_read ? current_branch->right : current_branch->left;
    switch (cur_->type ()) {
    case 0:
      eof_c = true;
      break;
    case 1:
      out_c << ((Leaf *)cur_)->data;
      current_branch = tree;
      break;
    case 2:
      current_branch = (Branch *)cur_;
      break;
    }
  } while (!eof_c && ro.left());
  delete tree;
  return out_c;
}