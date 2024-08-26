#include "huffman_codec.hpp"

#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <cstddef>

// Node structure for Huffman Tree
struct Node {
	virtual unsigned int frequency() const {
		return 0;
	}
	virtual unsigned int type() const {
		return 0;
	}
	// Comparator for priority queue
	struct compare {
		bool operator()(Node* l, Node* r) {
			return l->frequency() > r->frequency();
		}
	};
};
struct Branch: public Node {
	Node *left,
	*right;
	Branch (Node *l, Node *r): left(l),
	right(r) {}

	unsigned int frequency() const override {
		return left->frequency() + right->frequency();
	}
	unsigned int type() const override {
		return 2;
	}
	~Branch() {
		delete left;
		delete right;
	}
};
struct Leaf: public Node {
	unsigned int freq;
	uint32_t data;
	Leaf(unsigned int f, unsigned int d): freq(f),
	data(d) {}

	unsigned int frequency() const override {
		return freq;
	}
	unsigned int type() const override {
		return 1;
	}
};

// Utility function to build Huffman Tree and store the codes
void buildHuffmanTree(Node* root, std::vector < bool > code, std::unordered_map < uint32_t, std::vector < bool>> &huffmanCode) {
	switch (root->type()) {
		case 1:
		// Leaf node
		huffmanCode[((Leaf*)root)->data] = code;
		break;
		case 2: {
			Branch *b = (Branch*)root;
			std::vector < bool > code_left = code;
			code_left.push_back(false);
			buildHuffmanTree(b->left, code_left, huffmanCode);
			std::vector < bool > code_right = code;
			code_right.push_back(true);
			buildHuffmanTree(b->right, code_right, huffmanCode);
			break;
		}
	}
}

// Function to encode data using Huffman coding
const codec_data huffman_encode(const codec_data &cd) {
	// store frequency each data
	std::unordered_map < uint32_t, size_t > freq;

	// Count frequency of each character
	for (codec_data::reader ro = cd.begin_read(); ro.left();) {
		uint32_t key;
		ro >> key;
		freq[key]++;
	}
	// Create priority queue to store live nodes of Huffman tree
	std::priority_queue < Node*, std::vector < Node*>, Node::compare > pq;

	// Create leaf nodes for each character and add it to the priority queue
	for (auto pair: freq) {
		pq.push(new Leaf(pair.first, pair.second));
	}

	// Create Huffman tree
	while (pq.size() > 1) {
		Node *left = pq.top();
		pq.pop();
		Node *right = pq.top();
		pq.pop();
		pq.push(new Branch(left, right));
	}
	// Traverse the Huffman tree and store Huffman codes in a map
	std::unordered_map < uint32_t,
	std::vector < bool>> huffmanCode;
	buildHuffmanTree(pq.top(), std::vector < bool > (), huffmanCode);

	delete pq.top();

	codec_data out_c;
	// Encode Huffman codes
/*
	// Encode input data using Huffman codes
	for (codec_data::reader ro = cd.begin_read(); ro.left();) {
		uint32_t key;
		ro >> key;
		for (bool s: huffmanCode[key])
		out_c << s;
	}
*/
	return out_c;
}

const codec_data huffman_decode (const codec_data &cd) {
	return codec_data();
}