#include "matrix.hpp"

template <unsigned H, unsigned V>
matrix<H,V>::matrix() {}
template <unsigned H, unsigned V>
matrix<H,V>::~matrix() {}

template <unsigned H, unsigned V>
float &matrix<H,V>::operator[](unsigned l) { return data[l/V][l%V]; }
template <unsigned H, unsigned V>
float &matrix<H,V>::operator()(unsigned v, unsigned h) { return data[v][h]; }

template <unsigned H, unsigned V>
matrix<H,V> matrix<H,V>::operator+(const matrix<H,V>& other) {
    matrix<H,V> result;
    for (unsigned i = 0, j = 0; i < V; ++i) {
        for (j = 0; j < H; ++j) {
            result.data[i] = this->data[i][j] + other.data[i][j];
        }
    }
    return result;
}
template <unsigned H, unsigned V>
matrix<H,V> matrix<H,V>::operator*(const matrix<H,V>& other) {
    matrix<H,V> result;
    for (unsigned i = 0, j = 0, k = 0; i < H; ++i) {
        for (j = 0; j < V; ++j) {
            for (k = 0; k < V; ++k) {
                result(i, j) += this->data[i * V + k] * other.data[k * V + j];
            }
        }
    }
    return result;
}

template <unsigned H, unsigned V>
void matrix<H,V>::printInfo() {
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

template <unsigned H, unsigned V>
size_t matrix<H,V>::number_of_digits (float &n) {
	std::ostringstream strs;
	strs << n;
	return strs.str().size();
}

template <unsigned H, unsigned V>
void matrix<H,V>::print_matrix () {
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
		for (j = 0; j < H; ++j) {
			std::cout << (j == 0 ? "\n| " : "") << std::setw(max_len_each_vert[j]) << this->data[i][j] << (j == H - 1 ? " |" : " ");
    }
	}
	std::cout << '\n';
}

