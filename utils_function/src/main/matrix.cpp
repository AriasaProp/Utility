#include "matrix.hpp"

#define MATF(R,F) \
template <unsigned H, unsigned V>\
R matrix<H,V>::F

MATF(, matrix)() {}
MATF(, ~matrix)() {}

MATF(float, &operator[])(unsigned l) { return data[l/V][l % H]; }
MATF(float, &operator[])(unsigned v, unsigned h) { return data[v][h]; }

MATF(matrix, operator+) (const matrix& other) {
    matrix result;
    for (unsigned i = 0; i < H * V; ++i) {
        result.data[i] = this->data[i] + other.data[i];
    }
    return result;
}
MATF(matrix, operator*)(const matrix& other) {
    matrix result;
    for (unsigned i = 0, j = 0, k = 0; i < H; ++i) {
        for (j = 0; j < V; ++j) {
            for (k = 0; k < V; ++k) {
                result(i, j) += this->data[i * V + k] * other.data[k * V + j];
            }
        }
    }
    return result;
}

MATF(void, printInfo) () {
    std::cout << std::endl;
    for (unsigned i = 0, j = 0; i < V; i++) {
        std::cout << "\n\[";
        for (j = 0; j < H; j++) {
            std::cout << "| " << data[v * H + h] << " |";
        }
        std::cout << "\]";
    }
    std::cout << std::endl;
}

MATF(size_t, number_of_digits) (double n) {
	std::ostringstream strs;

	strs << n;
	return strs.str().size();
}

MATF(void, print_matrix) () {
	size_t max_len_each_vert[nmax];
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
		for (size_t j = 0; j < H; ++j) {
			std::cout << (j == 0 ? "\n| " : "") << std::setw(max_len_each_vert[j]) << this->data[i][j] << (j == H - 1 ? " |" : " ");
    }
	}
	std::cout << '\n';
}

