#include "huffman_codec.hpp"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>

typedef uint32_t dat_t;
typedef uint32_t dat_len;

namespace encode {
// Node structure for Huffman Tree
struct Node {
  virtual dat_len frequency () const {
    return 0;
  }
  virtual dat_len type () const {
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
  Node *left, *right;
  Branch (Node *l, Node *r) : left (l), right (r) {}

  dat_len frequency () const override {
    return left->frequency () + right->frequency ();
  }
  dat_len type () const override {
    return 2;
  }
  ~Branch () {
    delete left;
    delete right;
  }
};
struct Leaf : public Node {
  dat_t data;
  dat_len freq;
  Leaf (dat_t d, dat_len f) : data (d), freq (f) {}

  dat_len frequency () const override {
    return freq;
  }
  dat_len type () const override {
    return 1;
  }
};
struct Eof_ : public Node {
  dat_len frequency () const override {
    return 0;
  }
  dat_len type () const override {
    return 3;
  }
};

// Utility function to build Huffman Tree and store the codes
void buildHuffmanTree (codec_data &cd, Node *root, std::vector<bool> code, std::unordered_map<dat_t, std::vector<bool>> &huffmanCode, std::vector<bool> &eof_code) {
  switch (root->type ()) {
  default:
  case 0:
    break;
  case 3:
    cd << true << true; // like 3
    eof_code = code;
    break;
  case 1: {
    // Leaf node
    dat_t &key = ((Leaf *)root)->data;
    huffmanCode[key] = code;
    cd << true << false; // like 1
    cd << key;
    break;
  }
  case 2:
    // Branch node
    Branch *b = (Branch *)root;
    std::vector<bool> code_left = code;
    code_left.push_back (false);
    cd << false << true; // like 2
    buildHuffmanTree (cd, b->left, code_left, huffmanCode, eof_code);
    std::vector<bool> code_right = code;
    code_right.push_back (true);
    cd << false << true; // like 2
    buildHuffmanTree (cd, b->right, code_right, huffmanCode, eof_code);
    break;
  }
}
} // namespace encode
// Function to encode data using Huffman coding
const codec_data huffman_encode (codec_data const &cd) {
  dat_t temp;

  // store frequency each data
  // store 32bit as key, and 16bit for at least 65535 copy
  std::unordered_map<dat_t, dat_len> freq;
  // Count frequency of each character
  for (codec_data::reader ro = cd.begin_read (); ro.left ();) {
    ro >> temp;
    ++freq[temp];
  }
  // Encode Huffman codes
  codec_data out_c;
  // Create priority queue to store live nodes of Huffman tree
  std::priority_queue<encode::Node *, std::vector<encode::Node *>, encode::Node::compare> pq;

  for (std::pair<dat_t, dat_len> pair : freq) {
    //  Create leaf nodes for each character and add it to the priority queue
    pq.push (new encode::Leaf (pair.first, pair.second));
  }
  pq.push (new encode::Eof_);
  //  Create Huffman tree
  while (pq.size () > 1) {
    encode::Node *left = pq.top ();
    pq.pop ();
    encode::Node *right = pq.top ();
    pq.pop ();
    pq.push (new encode::Branch (left, right));
  }
  // Traverse the Huffman tree and store Huffman codes in a map
  std::vector<bool> eof_code;
  std::unordered_map<dat_t, std::vector<bool>> huffmanCode;
  encode::buildHuffmanTree (out_c, pq.top (), std::vector<bool> (), huffmanCode, eof_code);
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

namespace decode {
struct Node {
  virtual dat_len type () const {
    return 0;
  }
};
struct Branch : public Node {
  Node *left, *right;
  dat_len type () const override {
    return 2;
  }
  ~Branch () {
    delete left;
    delete right;
  }
};
struct Leaf : public Node {
  dat_t data;
  Leaf (dat_t d) : data (d) {}

  dat_len type () const override {
    return 1;
  }
};
struct Eof_ : public Node {
  dat_len type () const override {
    return 3;
  }
};

Node *readHuffmanTree (codec_data::reader &ro, unsigned char type) {
  switch (type) {
  default:
  case 0:
    break;
  case 1: {
  	std::cout << "Get Leaf" << std::endl;
    dat_t key;
    ro >> key;
    return new Leaf (key);
  }
  case 2: {
  	std::cout << "Get Branch" << std::endl;
    bool a, b;
    Branch *root = new Branch;
    ro >> a >> b;
  	std::cout << "left" << std::endl;
    root->left = readHuffmanTree (ro, a | (b << 1));
    ro >> a >> b;
  	std::cout << "right" << std::endl;
    root->right = readHuffmanTree (ro, a | (b << 1));
    return root;
  }
  case 3:
  	std::cout << "Get EOF" << std::endl;
    return new Eof_;
  }
  throw "resulting 0 key";
}
} // namespace decode

const codec_data huffman_decode (codec_data const &cd) {
  std::cout << "Dencode begin" << std::endl;
  codec_data::reader ro = cd.begin_read ();
  std::cout << "Reading begin" << std::endl;
  std::cout << "Reading Tree" << std::endl;
  decode::Branch *tree = (decode::Branch *)decode::readHuffmanTree (ro, 2);
  std::cout << "Reading Tree End" << std::endl;
  codec_data out_c;
  // decode input data using Huffman codes
  bool bit_read, eof_c = false;
  decode::Branch *current_branch = tree;
  decode::Node *cur_;
  do {
    ro >> bit_read;
    cur_ = bit_read ? current_branch->right : current_branch->left;
    switch (cur_->type ()) {
    default:
    case 0:
      break;
    case 3:
      eof_c = true;
      break;
    case 1:
      out_c << ((decode::Leaf *)cur_)->data;
      current_branch = tree;
      break;
    case 2:
      current_branch = (decode::Branch *)cur_;
      break;
    }
  } while (!eof_c && ro.left ());
  delete tree;
  return out_c;
}