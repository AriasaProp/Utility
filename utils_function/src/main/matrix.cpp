#include "matrix.hpp"
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>

template <size_t H, size_t V>
matrix<H,V>::matrix() {}
template <size_t H, size_t V>
matrix<H,V>::matrix(std::initializer_list<std::initializer_list<const float>> values) {
    if (values.size() != V) throw std::invalid_argument("Jumlah vertical tidak sesuai.");
    /*
    for(size_t v = 0, h = 0; v < V; v++) {
        for (h = 0; h < H; h++) {
            this->data[v][h] = in[v][h];
        }
    }
    */
    size_t v = 0;
    for (const auto &vert : values) {
        if (vert.size() != H) throw std::invalid_argument("Jumlah horizontal tidak sesuai.");
        size_t h = 0;
        for (const auto &hor : values) {
            data[v][h] = hor;
            ++h;
        }
        ++v;
    }
}

template <size_t H, size_t V>
matrix<H,V>::~matrix() {}

template <size_t H, size_t V>
float &matrix<H,V>::operator[](size_t l) { return data[l/V][l%V]; }
template <size_t H, size_t V>
float &matrix<H,V>::operator()(size_t v, size_t h) { return data[v][h]; }

template <size_t H, size_t V>
matrix<H,V> matrix<H,V>::operator+(const matrix<H,V>& other) {
    matrix<H,V> result;
    for (size_t i = 0, j = 0; i < V; ++i) {
        for (j = 0; j < H; ++j) {
            result.data[i] = this->data[i][j] + other.data[i][j];
        }
    }
    return result;
}
template <size_t H, size_t V>
matrix<H,V> matrix<H,V>::operator*(const matrix<H,V>& other) {
    matrix<H,V> result;
    for (size_t i = 0, j = 0, k = 0; i < H; ++i) {
        for (j = 0; j < V; ++j) {
            for (k = 0; k < V; ++k) {
                result(i, j) += this->data[i * V + k] * other.data[k * V + j];
            }
        }
    }
    return result;
}

template <size_t H, size_t V>
void matrix<H,V>::printInfo() {
    std::cout << std::endl;
    for (size_t i = 0, j = 0; i < V; i++) {
        std::cout << "\n[";
        for (j = 0; j < H; j++) {
            std::cout << "| " << data[i][j] << " |";
        }
        std::cout << ']';
    }
    std::cout << std::endl;
}

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

