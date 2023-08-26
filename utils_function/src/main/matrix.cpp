#include "matrix.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>


matrix2D::matrix2D() : cols(1), rows(1), data(new float) {}
matrix2D::matrix2D(const matrix2D &other) : cols(other.cols), rows(other.rows) {
    this->data = new float[cols*rows];
    memcpy(this->data, other.data, cols*rows*sizeof(float));
}
matrix2D::matrix2D(unsigned c, unsigned r, const std::initializer_list<float> d) : cols(c), rows(r) {
    if (!cols || !rows) throw("matrix size cannot be 0");
    if (d.size() != cols*rows) throw("array size is wrong");
    this->data = new float[cols*rows]{};
    memcpy(this->data, d.data(), cols*rows*sizeof(float));
}
matrix2D::matrix2D(unsigned c, unsigned r, const float *d = nullptr) : cols(c), rows(r) {
    if (!cols || !rows) throw("matrix size cannot be 0");
    this->data = new float[cols*rows]{};
    if (d) memcpy(this->data, d, cols*rows*sizeof(float));
}
matrix2D::~matrix2D() {
    delete[] this->data;
}
//unique function
void matrix2D::invert() {
}
float matrix2D::determinant() {
    return 0;
}
void matrix2D::adj() {
}
//operator function
float &matrix2D::operator()(unsigned c, unsigned r) {
    return this->data[c*this->rows+r];
}

//operator math
matrix2D &matrix2D::operator=(const matrix2D &o) {
    this->cols = o.cols;
    this->rows = o.rows;
    delete[] this->data;
    this->data = new float[cols*rows];
    memcpy(this->data, o.data, cols*rows*sizeof(float));
    return *this;
}
matrix2D matrix2D::operator+(const matrix2D &o) const {
    matrix2D res(*this);
    return res += o;
}
matrix2D &matrix2D::operator+=(const matrix2D &o) {
    if ((this->rows != o.rows) || (this->cols != o.cols)) throw("cannot doing addition on different matrix dimension");
    for (unsigned i = 0, j = cols*rows; i < j; ++i) {
        this->data[i] += o.data[i];
    }
    return *this;
}
matrix2D matrix2D::operator-(const matrix2D &o) const {
    matrix2D res(*this);
    return res -= o;
}
matrix2D &matrix2D::operator-=(const matrix2D &o) {
    if ((this->rows != o.rows) || (this->cols != o.cols)) throw("cannot doing addition on different matrix dimension");
    for (unsigned i = 0, j = cols*rows; i < j; ++i) {
        this->data[i] -= o.data[i];
    }
    return *this;
}
matrix2D matrix2D::operator*(const float &o) const {
    float *temp = new float[this->cols*this->rows] {};
    for (unsigned i = 0, j = this->cols*this->rows; i < j; ++i) {
        temp[i] = this->data[i]*o;
    }
    return matrix2D(this->cols, this->rows, temp);
}
matrix2D &matrix2D::operator*=(const float &o) {
    for (unsigned i = 0, j = this->cols*this->rows; i < j; ++i) {
        this->data[i] *= o;
    }
    return *this;
}
matrix2D matrix2D::operator*(const matrix2D &o) const {
    if (this->rows != o.cols) throw("cannot doing multiplication matrix cause dimension is not fit");
    float *temp = new float[this->cols*o.rows] {};
    for (unsigned i = 0, j = this->cols*o.rows; i < j; ++i) {
        for (unsigned k = 0; k < this->rows; ++k) {
            temp[i] += this->data[i*this->rows+k]*o.data[i+k*o.rows];
        }
    }
    return matrix2D(this->cols, o.rows, temp);
}
matrix2D &matrix2D::operator*=(const matrix2D &o) {
    if (this->rows != o.cols) throw("cannot doing multiplication matrix cause dimension is not fit");
    float *temp = new float[this->cols*o.rows] {};
    for (unsigned i = 0, j = this->cols*o.rows; i < j; ++i) {
        for (unsigned k = 0; k < this->rows; ++k) {
            temp[i] += this->data[i*this->rows+k]*o.data[i+k*o.rows];
        }
    }
    delete[] this->data;
    this->data = temp;
    this->rows = o.rows;
    return *this;
}
matrix2D matrix2D::operator/(const float &o) const {
    float *temp = new float[this->cols*this->rows] {};
    for (unsigned i = 0, j = this->cols*this->rows; i < j; ++i) {
        temp[i] = this->data[i]/o;
    }
    return matrix2D(this->cols, this->rows, temp);
}
matrix2D &matrix2D::operator/=(const float &o) {
    for (unsigned i = 0, j = this->cols*this->rows; i < j; ++i) {
        this->data[i] /= o;
    }
    return *this;
}
matrix2D matrix2D::operator/(matrix2D) const {
    return matrix2D();
}
matrix2D &matrix2D::operator/=(matrix2D) {
    return *this;
}

void matrix2D::print () const {
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
	std::cout << "matrix (row_length:" << this->rows << ", col_length" << this->cols << ")" << std::endl;
	for (unsigned i = 0, j = 0; i < cols; ++i) {
  		for (j = 0; j < rows; ++j) {
  			  std::cout << (j == 0 ? "\n| " : "") << std::setw(len_each_row[j]) << this->data[i*rows+j] << (j == rows - 1 ? " |" : " ");
      }
	}
	std::cout << '\n';
}

unsigned matrix2D::size() const {
    return cols*rows;
}

