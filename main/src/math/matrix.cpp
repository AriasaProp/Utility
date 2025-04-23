#include "math/Matrix.hpp"

#include <cstring>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <string>
#include <stdexcept> // Untuk std::invalid_argument


#include "common.hpp"

/*
    Matrix Initialize
    cols ->
rows 00  01   02   03  ....
 |   10  11   12   13  .....
 v   20  21   22   23  .....
     .0  .1   .2   .3  ....
     
     index follows by [rows, cols]
*/

/*
TO DO
Add, sub logic -> must equal dimension
mul logic -> A cols == B rows
div logic -> B must square

*/

#define MATRIX_SIZE(A) (A.rows * A.cols)
#define MATRIX_IS_SQUARE(A) (A.rows == A.cols)
#define MATRIX_DIMS_EQ(A, B) (A.rows == B.rows) * (A.cols == B.cols)
#define MATRIX_SIZE_BYTE(A) (A.rows * A.cols * sizeof(vs))

// constructors
Matrix::Matrix () {}
Matrix::Matrix (size_t c, size_t r) : cols(c), rows(r){
  data = (vs*) calloc(sizeof(vs), MATRIX_SIZE((*this)));
}
Matrix::Matrix (size_t c, size_t r, vs *d) : cols(c), rows(r){
  size_t by = MATRIX_SIZE_BYTE((*this));
  data = (vs*) malloc(by);
  memcpy(data, d, by);
}
Matrix::Matrix (const Matrix &other) : cols(other.cols), rows(other.rows) {
  size_t by = MATRIX_SIZE_BYTE((*this));
  data = (vs*) malloc(by);
  memcpy(data, other.data, by);
}
// destructors
Matrix::~Matrix () {
  free(data);
}
// operator function
double &Matrix::operator[] (const size_t i) const {
  return data[i];
}
double &Matrix::operator() (const size_t c, const size_t r) const {
  return data[MIN (r, rows) * cols + MIN (c, cols)];
}
  // unique function
Matrix identity (const Matrix &a) {
  Matrix b(a.cols, a.rows);
  size_t i, j = MATRIX_SIZE(b);
  for(i = 0; i < j; i += b.cols + 1)
    b[i] = 1.0;
  return b;
}
Matrix inverse (const Matrix &a) {
  if (!MATRIX_IS_SQUARE(a)) throw "Matrix not square";
  size_t n = a.cols, i, j, k;
  Matrix b(n, n);
#define EPSILON 1e-9
  double** augmented = (double**)malloc(n * sizeof(double*));
  for (i = 0; i < n; ++i) {
    augmented[i] = (double*)malloc(2 * n * sizeof(double));
    for (j = 0; j < n; ++j) {
      augmented[i][j] = a(j, i);            // Kiri: matriks A
      augmented[i][j + n] = (i == j) ? 1.0 : 0.0;  // Kanan: identitas
    }
  }
  for (i = 0; i < n; ++i) {
    size_t max_row = i;
    for (k = i + 1; k < n; ++k)
      if (fabs(augmented[k][i]) > fabs(augmented[max_row][i]))
        max_row = k;

    if (fabs(augmented[max_row][i]) < EPSILON) {
      for (j = 0; j < n; ++j)
        free(augmented[j]);
      free(augmented);
      throw "fail on inverse";
    }
    if (i != max_row) {
      for (k = 0; k < 2 * n; ++k) {
        double temp = augmented[i][k];
        augmented[i][k] = augmented[j][k];
        augmented[j][k] = temp;
      }
    }
    double pivot = augmented[i][i];
    for (j = 0; j < 2 * n; ++j)
      augmented[i][j] /= pivot;
    for (k = 0; k < n; ++k) {
      if (k != i) {
        double factor = augmented[k][i];
        for (j = 0; j < 2 * n; ++j) {
          augmented[k][j] -= factor * augmented[i][j];
        }
      }
    }
  }
  for (i = 0; i < n; ++i)
    for (j = 0; j < n; ++j)
      b(j, i) = augmented[i][j + n];
    free(augmented[i]);
  free(augmented);
#undef EPSILON
  return b;
}
  // operator compare
bool operator== (const Matrix &a, const Matrix &b) {
  return MATRIX_DIMS_EQ(a, b) * (0 == memcmp(a.data, b.data, MATRIX_SIZE_BYTE(a)));
}
bool operator!= (const Matrix &a, const Matrix &b) {
  return !MATRIX_DIMS_EQ(a, b) * memcmp(a.data, b.data, MATRIX_SIZE_BYTE(a));
}
  // operators math
Matrix &Matrix::operator= (const Matrix b) {
  size_t by = MATRIX_SIZE_BYTE(b);
  if (!MATRIX_DIMS_EQ((*this), b)) {
    cols = b.cols;
    rows = b.rows;
    free (data);
    data = (vs*)malloc(by);
  }
  memcpy(data, b.data, by);
  return *this;
}
Matrix operator+ (const Matrix &a, const Matrix b) {
  if (!MATRIX_DIMS_EQ(a, b)) throw "cannot do Addition for different Matrix dimension";
  Matrix c(a.cols, a.rows);
  for (size_t i = 0; i < MATRIX_SIZE(a); ++i)
    c[i] = a[i] + b[i];
  return c;
}
Matrix &operator+= (Matrix &a, const Matrix b) {
  if (!MATRIX_DIMS_EQ(a, b)) throw "cannot do Addition for different Matrix dimension";
  for (size_t i = 0; i < MATRIX_SIZE(a); ++i)
    a[i] += b[i];
  return a;
}
Matrix operator- (const Matrix &a, const Matrix b) {
  if (!MATRIX_DIMS_EQ(a, b)) throw "cannot do Subtraction for different Matrix dimension";
  Matrix c(a.cols, a.rows);
  for (size_t i = 0; i < MATRIX_SIZE(a); ++i)
    c[i] = a[i] - b[i];
  return c;
}
Matrix &operator-= (Matrix &a, const Matrix b) {
  if (!MATRIX_DIMS_EQ(a, b)) throw "cannot do Subtraction for different Matrix dimension";
  for (size_t i = 0; i < MATRIX_SIZE(a); ++i)
    a[i] -= b[i];
  return a;
}
Matrix operator* (const Matrix &a, const double b) {
  Matrix c(a.cols, a.rows);
  for (size_t i = 0; i < MATRIX_SIZE(a); ++i)
    c[i] = a[i] * b;
  return c;
}
Matrix &operator*= (Matrix &a, const double b) {
  for (size_t i = 0; i < MATRIX_SIZE(a); ++i)
    a[i] *= b;
  return a;
}
Matrix operator* (const Matrix &a, const Matrix b) {
  if (a.cols != b.rows) throw "not match A cols and B rows to do multiplication";
  Matrix c(b.cols, a.rows);
  size_t i, j, k;
  for(i = 0; i < c.rows; ++i)
    for(j = 0; j < a.cols; ++j)
      for(k = 0; k < c.cols; ++k)
        c(k, i) += a(j, i) * b(k, j);
  return c;
}
Matrix &operator*= (Matrix &a, const Matrix b) {
  if (a.cols != b.rows) throw "not match A cols and B rows to do multiplication";
  vs *c = (vs*) calloc(b.cols*a.rows, sizeof(vs));
  size_t i, j, k;
  for(i = 0; i < a.rows; ++i)
    for(j = 0; j < a.cols; ++j)
      for(k = 0; k < b.cols; ++k)
        c[(i * b.cols) + k] += a(j, i) * b(k, j);
  free(a.data);
  a.cols = b.cols;
  a.data = c;
  return a;
}
Matrix operator/ (const Matrix &a, const double b) {
  Matrix c(a.cols, a.rows);
  for (size_t i = 0; i < MATRIX_SIZE(a); ++i)
    c[i] = a[i] / b;
  return c;
}
Matrix &operator/= (Matrix &a, const double b) {
  for (size_t i = 0; i < MATRIX_SIZE(a); ++i)
    a[i] /= b;
  return a;
}
Matrix operator/ (const Matrix &a, const Matrix b) {
  return a*inverse(b);
}
Matrix &operator/= (Matrix &a, const Matrix b) {
  return a*=inverse(b);
}
  /** stream operator **/
std::ostream &operator<< (std::ostream &o, const Matrix &a) {
  o << "[" << a.cols << " " << a.rows << "]{";
  o << std::fixed << std::setprecision(2);
  size_t i = 0, j = MATRIX_SIZE(a);
  while (i < j) {
    o << a[i];
    if (++i < j) o << " ";
  }
  o << "}";
  return o;
}