#include "matrix.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>

#define MIN(x,y) (x<y?x:y)

matrix2D::matrix2D() : cols(2), rows(2), data(new float) {}
matrix2D::matrix2D(const matrix2D &other) : cols(other.cols), rows(other.rows) {
    this->data = new float[cols*rows] {};
    memcpy(this->data, other.data, cols*rows*sizeof(float));
}
matrix2D::matrix2D(unsigned c, unsigned r, const std::initializer_list<float> d) : cols(c), rows(r) {
    if (!cols || !rows) throw("matrix size cannot be 0");
    if (d.size() != cols*rows) throw("array size is wrong");
    this->data = new float[cols*rows]{};
    memcpy(this->data, d.begin(), cols*rows*sizeof(float));
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
matrix2D &matrix2D::identity() {
		if (this->cols != this->rows) throw("cannot find identity for non-square matrix");
		memset(this->data, 0, sizeof(float)*this->cols*this->cols);
		for (unsigned i = 0; i < this->cols; ++i)
        this->data[this->cols*i+i] = 1;
}
matrix2D matrix2D::inverse() const {
		if (this->cols != this->rows) throw("cannot find inverse for non-square matrix");
		matrix2D res(this->cols, this->cols);
		res.identity();
		
		float *m1 = new float[this->cols * this->cols];
		memcpy(m1, this->data, sizeof(float)*this->cols*this->cols);
		
    unsigned i, j, k;
    float selector;
    for (i = 0; i < this->cols; ++i) {
        selector = m1[this->cols*i+i]];
        for (j = 0; j < this->cols; ++j) {
        		k = this->cols*i+j;
            m1[k] /= selector;
            rs.data[k] /= selector;
        }
        for (j = 0; j < this->cols; ++j) {
            if (j == i || m1[this->cols*j+i] == 0)
                continue;
            selector = m1[this->cols*j+i];
            for (k = 0; k < this->cols; k++) {
                m1[this->cols*j+k] -= selector * m1[this->cols*i+k];
                res.data[this->cols*j+k] -= selector * res.data[this->cols*i+k];
            }
        }
    }
    delete[] m1;
		return res;
}
static float detPart(float *matrix, unsigned n) {
    switch (n) {
    case 1:
        return 0;
    case 2:
        return matrix[0] * matrix[3] - matrix[1] * matrix[2];
    default:
        float d = 0;
        unsigned minorSize = n - 1;
        float minorMatrix[minorSize * minorSize];
        for (unsigned i = 0; i < n; ++i) {
            unsigned minorRow = 0;
            for (unsigned row = 1; row < n; ++row) {
                unsigned minorCol = 0;
                for (unsigned col = 0; col < n; ++col) {
                    if (col != i) {
                        minorMatrix[minorRow * minorSize + minorCol] = matrix[row * n + col];
                        minorCol++;
                    }
                }
                minorRow++;
            }
            float minorDet = detPart(minorMatrix, minorSize);
            if (i&1)
                d -= matrix[i] * minorDet;
            else
                d += matrix[i] * minorDet;
        }
        return d;
    }
}
float matrix2D::det() {
    if (this->cols != this->rows) throw("cannot find determinant for non-square matrix");
    return detPart(this->data, this->cols);
}

//operator function
float &matrix2D::operator()(unsigned c, unsigned r) {
    return this->data[MIN(c, cols)*this->rows+MIN(r, rows)];
}

//operator compare
bool &matrix2D::operator==(const matrix2D &o) const {
		return (this->cols == o.cols) && (this->rows == o.rows) && (memcmp(this->data, o.data, sizeof(float)*this->cols*this->rows) == 0);
}
bool &matrix2D::operator!=(const matrix2D &o) const {
		return (this->cols != o.cols) || (this->rows != o.rows) || (memcmp(this->data, o.data, sizeof(float)*this->cols*this->rows) != 0);
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
    for (unsigned i = 0, I = this->cols*o.rows; i < I; ++i) {
        for (unsigned j = 0, J = this->rows; j < J; ++j) {
            temp[i] += this->data[(i/this->rows)*this->rows+j]*o.data[(i%this->rows)+j*o.rows];
        }
    }
    return matrix2D(this->cols, o.rows, temp);
}
matrix2D &matrix2D::operator*=(const matrix2D &o) {
    if (this->rows != o.cols) throw("cannot doing multiplication matrix cause dimension is not fit");
    float *temp = new float[this->cols*o.rows] {};
    for (unsigned i = 0, I = this->cols*o.rows; i < I; ++i) {
        for (unsigned j = 0, J = this->rows; j < J; ++j) {
            temp[i] += this->data[(i/this->rows)*this->rows+j]*o.data[(i%this->rows)+j*o.rows];
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
matrix2D matrix2D::operator/(const matrix2D &o) const {
    return matrix2D(this) * o.inverse();
}
matrix2D &matrix2D::operator/=(const matrix2D &o) {
		float det = o.det();
		if (det == 0.0f) throw ("it's singular matrix, i can't do devision!");
    return *this *= o.inverse();
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
	std::cout << "matrix (row_length:" << this->rows << ", col_length: " << this->cols << ")" << std::endl;
	for (unsigned i = 0, j = 0; i < cols; ++i) {
	    std::cout << "|";
  		for (j = 0; j < rows; ++j) {
  			  std::cout << " " << std::setw(len_each_row[j]) << this->data[i*rows+j] << " ";
      }
	    std::cout << "|" << std::endl;
	}
}

unsigned matrix2D::size() const {
    return cols*rows;
}

