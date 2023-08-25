#include "matrix.hpp"
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>

template <size_t H, size_t V>
matrix<H, V>::matrix(const float d[V][H]) {
    for (size_t i = 0; i < V; ++i) {
        for (size_t j = 0; j < H; ++j) {
            this->data[i][j] = d[i][j];
        }
    }
}

template <size_t H, size_t V>
void matrix<H, V>::print() const {
    for (size_t i = 0; i < V; ++i) {
        for (size_t j = 0; j < H; ++j) {
            std::cout << this->data[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

/*
template <size_t H, size_t V>
size_t matrix<H,V>::number_of_digits (float &n) {
	std::ostringstream strs;
	strs << n;
	return strs.str().size();
}

template <size_t H, size_t V>
void matrix<H,V>::print_matrix () {
	size_t max_len_each_vert[H] {};
	//find length each vertical
	for (size_t j = 0; j < H; ++j) { //horizontal
		size_t max_len {};

		for (size_t i = 0; i < V; ++i) { //vertical
			if (const auto num_length {number_of_digits(this->data[i][j])}; num_length > max_len)
				max_len = num_length;
		}
		max_len_each_vert[j] = max_len;
	}

	for (size_t i = 0, j = 0; i < V; ++i) {
		for (j = 0; j < H; ++j) {
			std::cout << (j == 0 ? "\n| " : "") << std::setw(max_len_each_vert[j]) << this->data[i][j] << (j == H - 1 ? " |" : " ");
    }
	}
	std::cout << '\n';
}

*/