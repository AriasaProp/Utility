#include "matrix.hpp"
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>

matrix2D::matrix2D(unsigned c = 1, unsigned r = 1, const float *d = nullptr) : cols(c), rows(r) {
    if (!cols || !rows) throw("matrix size cannot be 0");
    this->data = new float[cols*rows];
    memcpy(this->data, d, cols*rows*sizeof(float));
}
matrix2D::~matrix2D() {
    delete[] this->data;
}
matrix2D operator=(const float *d) {
    memcpy(this->data, d, cols*rows*sizeof(float));
}

void matrix2D::print () {
	unsigned len_each_row[rows] {};
	//find length each vertical
	for (unsigned i = 0, j = 0; i < rows; ++i) { //rows
		unsigned max_len {};
		for (j = 0; j < cols; ++j) { //vertical
	    std::ostringstream strs;
	    strs << this->data[j*rows+i];
			unsigned num_length = strs.str().size();
			if (num_length > max_len) max_len = num_length;
		}
		len_each_row[i] = max_len;
	}

	for (unsigned i = 0, j = 0; i < cols; ++i) {
		for (j = 0; j < rows; ++j) {
			std::cout << (j == 0 ? "\n| " : "") << std::setw(len_each_row[j]) << this->data[i*rows+j] << (j == rows - 1 ? " |" : " ");
    }
	}
	std::cout << '\n';
}

