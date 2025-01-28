#include "matrix.hpp"

#include <cstring>
#include <iomanip>

#define MIN(x, y) (x < y ? x : y)

matrix2D::matrix2D () : cols (2), rows (2), data (new float) {}
matrix2D::matrix2D (const matrix2D &other) : cols (other.cols), rows (other.rows) {
  this->data = new float[cols * rows]{};
  memcpy (this->data, other.data, cols * rows * sizeof (float));
}
matrix2D::matrix2D (size_t c, size_t r, const std::initializer_list<float> d) : cols (c), rows (r) {
  if (!cols || !rows) throw ("matrix size cannot be 0");
  if (d.size () != cols * rows) throw ("array size is wrong");
  this->data = new float[cols * rows]{};
  memcpy (this->data, d.begin (), cols * rows * sizeof (float));
}
matrix2D::matrix2D (size_t c, size_t r, const float *d = nullptr) : cols (c), rows (r) {
  if (!cols || !rows) throw ("matrix size cannot be 0");
  this->data = new float[cols * rows]{};
  if (d) memcpy (this->data, d, cols * rows * sizeof (float));
}
matrix2D::~matrix2D () {
  delete this->data;
}
// unique function
matrix2D &matrix2D::identity () {
  if (this->cols != this->rows) throw ("cannot find identity for non-square matrix");
  memset (this->data, 0, sizeof (float) * this->cols * this->cols);
  for (size_t i = 0; i < this->cols; ++i)
    this->data[this->cols * i + i] = 1;
  return *this;
}
matrix2D matrix2D::inverse () const {
  if (this->cols != this->rows) throw ("cannot find inverse for non-square matrix");
  matrix2D res (this->cols, this->cols);
  res.identity ();

  float *m1 = new float[this->cols * this->cols];
  memcpy (m1, this->data, sizeof (float) * this->cols * this->cols);

  size_t i, j, k;
  float selector;
  for (i = 0; i < this->cols; ++i) {
    selector = m1[this->cols * i + i];
    for (j = this->cols * i, k = j + this->cols; j < k; ++j) {
      m1[j] /= selector;
      res.data[j] /= selector;
    }
    for (j = 0; j < this->cols; ++j) {
      if (j == i || m1[this->cols * j + i] == 0)
        continue;
      selector = m1[this->cols * j + i];
      for (k = 0; k < this->cols; k++) {
        m1[this->cols * j + k] -= selector * m1[this->cols * i + k];
        res.data[this->cols * j + k] -= selector * res.data[this->cols * i + k];
      }
    }
  }
  delete[] m1;
  return res;
}
static float detPart (float *matrix, size_t n) {
  switch (n) {
  case 1:
    return 0;
  case 2:
    return matrix[0] * matrix[3] - matrix[1] * matrix[2];
  default:
    float d = 0;
    size_t minorSize = n - 1;
    float minorMatrix[minorSize * minorSize];
    for (size_t i = 0; i < n; ++i) {
      size_t minorRow = 0;
      for (size_t row = 1; row < n; ++row) {
        size_t minorCol = 0;
        for (size_t col = 0; col < n; ++col) {
          if (col != i) {
            minorMatrix[minorRow * minorSize + minorCol] = matrix[row * n + col];
            minorCol++;
          }
        }
        minorRow++;
      }
      float minorDet = detPart (minorMatrix, minorSize);
      if (i & 1)
        d -= matrix[i] * minorDet;
      else
        d += matrix[i] * minorDet;
    }
    return d;
  }
}
float matrix2D::det () const {
  if (this->cols != this->rows) throw ("cannot find determinant for non-square matrix");
  return detPart (this->data, this->cols);
}

// operator function
float &matrix2D::operator() (size_t c, size_t r) {
  return this->data[MIN (c, cols) * this->rows + MIN (r, rows)];
}

// operator compare
bool matrix2D::operator== (const matrix2D &o) const {
  return (this->cols == o.cols) && (this->rows == o.rows) && (memcmp (this->data, o.data, sizeof (float) * this->cols * this->rows) == 0);
}
bool matrix2D::operator!= (const matrix2D &o) const {
  return (this->cols != o.cols) || (this->rows != o.rows) || (memcmp (this->data, o.data, sizeof (float) * this->cols * this->rows) != 0);
}
// operator math
matrix2D &matrix2D::operator= (const matrix2D &o) {
  if ((this->cols != o.cols) || (this->rows != o.rows)) {
    this->cols = o.cols;
    this->rows = o.rows;
    delete this->data;
    this->data = new float[cols * rows];
  }
  memcpy (this->data, o.data, cols * rows * sizeof (float));
  return *this;
}
matrix2D matrix2D::operator+ (const matrix2D &o) const {
  matrix2D res (*this);
  return res += o;
}
matrix2D &matrix2D::operator+= (const matrix2D o) {
  if ((this->rows != o.rows) || (this->cols != o.cols)) throw ("cannot doing addition on different matrix dimension");
  for (size_t i = 0, j = cols * rows; i < j; ++i) {
    this->data[i] += o.data[i];
  }
  return *this;
}
matrix2D matrix2D::operator- (const matrix2D &o) const {
  matrix2D res (*this);
  return res -= o;
}
matrix2D &matrix2D::operator-= (const matrix2D o) {
  if ((this->rows != o.rows) || (this->cols != o.cols)) throw ("cannot doing addition on different matrix dimension");
  for (size_t i = 0, j = cols * rows; i < j; ++i) {
    this->data[i] -= o.data[i];
  }
  return *this;
}
matrix2D matrix2D::operator* (const float &o) const {
  float *temp = new float[this->cols * this->rows]{};
  for (size_t i = 0, j = this->cols * this->rows; i < j; ++i) {
    temp[i] = this->data[i] * o;
  }
  return matrix2D (this->cols, this->rows, temp);
}
matrix2D &matrix2D::operator*= (const float &o) {
  for (size_t i = 0, j = this->cols * this->rows; i < j; ++i) {
    this->data[i] *= o;
  }
  return *this;
}
matrix2D matrix2D::operator* (const matrix2D &o) const {
  if (this->rows != o.cols) throw ("cannot doing multiplication matrix cause dimension is not fit");
  float *temp = new float[this->cols * o.rows]{};
  for (size_t i = 0, I = this->cols * o.rows; i < I; ++i) {
    for (size_t j = 0, J = this->rows; j < J; ++j) {
      temp[i] += this->data[(i / this->rows) * this->rows + j] * o.data[(i % this->rows) + j * o.rows];
    }
  }
  return matrix2D (this->cols, o.rows, temp);
}
matrix2D &matrix2D::operator*= (const matrix2D o) {
  if (this->rows != o.cols) throw ("cannot doing multiplication matrix cause dimension is not fit");
  float *temp = new float[this->cols * o.rows]{};
  for (size_t i = 0, I = this->cols * o.rows; i < I; ++i) {
    for (size_t j = 0, J = this->rows; j < J; ++j) {
      temp[i] += this->data[(i / this->rows) * this->rows + j] * o.data[(i % this->rows) + j * o.rows];
    }
  }
  delete this->data;
  this->data = temp;
  this->rows = o.rows;
  return *this;
}
matrix2D matrix2D::operator/ (const float &o) const {
  float *temp = new float[this->cols * this->rows]{};
  for (size_t i = 0, j = this->cols * this->rows; i < j; ++i) {
    temp[i] = this->data[i] / o;
  }
  return matrix2D (this->cols, this->rows, temp);
}
matrix2D &matrix2D::operator/= (const float &o) {
  for (size_t i = 0, j = this->cols * this->rows; i < j; ++i) {
    this->data[i] /= o;
  }
  return *this;
}
matrix2D matrix2D::operator/ (const matrix2D &o) const {
  return matrix2D (*this) * o.inverse ();
}
matrix2D &matrix2D::operator/= (const matrix2D o) {
  float det = o.det ();
  if (det == 0.0f) throw ("it's singular matrix, i can't do devision!");
  return *this *= o.inverse ();
}

/** stream operator **/
std::ostream &operator<< (std::ostream &o, const matrix2D &a) {
  o << "[";
	if (a.size()) {
	  o << a.rows << ", " << a.cols << "]{";
	  for (size_t i = 0, j = a.size(); i < j; ++i) {
	    o << std::setprecision(2) << a.data[i];
	    if (i < (j-1)) o << ",";
	  }
	  o << "}";
	} else {
		o << "0]";
	}
  return o;
}

size_t matrix2D::size () const {
  return cols * rows;
}
