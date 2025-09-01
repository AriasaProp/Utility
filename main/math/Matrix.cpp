#include "math/Matrix.hpp"
#include "common.hpp"

#include <iostream>
#include <cstring>
#include <utility>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <string>
#include <stdexcept> // for std::invalid_argument


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
Add, sub logic -> must be equal dimension √
mul logic -> A cols == B rows √
div logic -> A * B^-1 that B must be square √
inverse: gauss-jordan augment √

*/
#define EPSILON 1e-13

#define THIS_MATRIX_SIZE (rows * cols)
#define THIS_MATRIX_SIZE_BYTE (THIS_MATRIX_SIZE * sizeof(double))
#define THIS_MATRIX_IS_SQUARE (rows == cols)
#define THIS_MATRIX_DIMS_EQ(B) ((rows == B.rows) * (cols == B.cols))

#define MATRIX_SIZE(A) (A.rows * A.cols)
#define MATRIX_SIZE_BYTE(A) (MATRIX_SIZE(A) * sizeof(double))
#define MATRIX_IS_SQUARE(A) (A.rows == A.cols)
#define MATRIX_DIMS_EQ(A, B) (A.rows == B.rows) * (A.cols == B.cols)
static INLINE bool is_approach_zero(double a) {
  return fabs(a) < EPSILON;
}
static bool floating_equals(double *a, double *b, size_t n) {
  size_t i = 0;
  while ((i < n) && is_approach_zero(a[i] - b[i]))
    ++i;
  return i >= n;
}
// make identity Matrix by size
Matrix Matrix::identity (size_t c, size_t r) {
  Matrix b(c, r);
  size_t i, j = MIN(c,r);
  for(i = 0; i < j; ++i)
    b(i, i) = 1.0;
  return b;
}
// constructors
Matrix::Matrix () {}
// matrix string format [cols rows]{v .... }
Matrix::Matrix (const char *c) {
  char *array = (char*)malloc(2048);
  char *a = array;
  if (sscanf(c, "[%zu %zu]{%[^}]}", &cols, &rows, array) < 3)
    throw "Matrix string argument construction doesn't follow \"[cols rows]{v .... }\" format text";
  data = (double*) malloc(THIS_MATRIX_SIZE_BYTE);
  size_t i = 0, j = THIS_MATRIX_SIZE;
  while (*a && (i < j)) {
    switch (*a) {
      case '-': case '0': case '1':
      case '2': case '3': case '4':
      case '5': case '6': case '7':
      case '8': case '9': case '+':
        data[i++] = strtod(a, &a);
        break;
      case ' ':
        ++a;
        break;
      default:
        throw *this;
    }
  }
  free (array);
}
Matrix::Matrix (size_t c, size_t r) : cols(c), rows(r){
  data = (double*) calloc(sizeof(double), THIS_MATRIX_SIZE);
}
Matrix::Matrix (size_t c, size_t r, double *d) : cols(c), rows(r){
  size_t by = THIS_MATRIX_SIZE_BYTE;
  data = (double*) malloc(by);
  memcpy(data, d, by);
}
Matrix::Matrix (const Matrix &other) : cols(other.cols), rows(other.rows) {
  size_t by = THIS_MATRIX_SIZE_BYTE;
  data = (double*) malloc(by);
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
Matrix Matrix::inverse () const {
  if (!THIS_MATRIX_IS_SQUARE) throw "Matrix not square";
  const size_t &n = cols;
  size_t i, j, k;
  // gauss-jordan matrix inverse
  Matrix A(*this), C = identity(n,n), D(n,n), E(n,n);
  struct M { size_t a, b; };
  M *m = new M[n]{};
  // convert to row echelons form
  for (i = 0; i < n; ++i) {
    if (is_approach_zero(A(i, i))) {
      // get order
      for (j = 0; j < n; ++j) {
        for (k = 0; is_approach_zero (A(k, j)) && (k++ < n); );
        // is singular return itself
        if (k >= n) return *this;
        m[j].a = j, m[j].b = k;
      }
      static const auto sf = [](M a, M b) { return a.b < b.b; };
      std::sort(m, m + n, sf);
      for (j = 0; j < n; ++j) {
        M &mp = m[j];
        memcpy(D.data+(j * n), A.data+(mp.a * n), n * sizeof(double));
        memcpy(E.data+(j * n), C.data+(mp.a * n), n * sizeof(double));
      }
      A = D, C = E;
    }
    // this is singular
    if (is_approach_zero(A(i, i))) return *this;
    for (j = i + 1; j < n; ++j) A(j, i) /= A(i, i);
    for (j = 0; j < n; ++j) C(j, i) /= A(i, i);
    A(i, i) = 1.0;
    // delete elements in row below
    for (j = i + 1; j < n; ++j) {
      if (!is_approach_zero (A(i, j))) {
        for (k = i + 1; k < n; ++k) A(k, j) -= A(i, j) * A(k, i);
        for (k = 0; k < n; ++k) C(k, j) -= A(i, j) * C(k, i);
        A(i, j) = 0;
      }
    }
  }
  // backtrack
  while (--i) {
    for (j = i; j--; ) {
      if (!is_approach_zero(A(i, j))) {
        for (k = 0; k < n; ++k) C(k, j) -= A(i, j) * C(k, i);
        A(i, j) = 0;
      }
    }
  }
  delete[] m;
  return C;
}
  // operator compare
bool Matrix::operator== (const Matrix &b) const {
  return (rows == b.rows) && (cols == b.cols) &&
    floating_equals(data, b.data, THIS_MATRIX_SIZE);
}
bool Matrix::operator!= (const Matrix &b) const {
  return (rows != b.rows) || (cols != b.cols) ||
    !floating_equals(data, b.data, THIS_MATRIX_SIZE);
}
  // operators math
Matrix &Matrix::operator= (const Matrix b) {
  size_t by = MATRIX_SIZE_BYTE(b);
  if (!THIS_MATRIX_DIMS_EQ(b)) {
    cols = b.cols, rows = b.rows;
    data = (double*) realloc (data, by);
  }
  memcpy(data, b.data, by);
  return *this;
}
Matrix Matrix::operator+ (const Matrix b) const {
  if (!THIS_MATRIX_DIMS_EQ(b)) throw "cannot do Addition for different Matrix dimension";
  Matrix c(cols, rows);
  for (size_t i = 0, j = THIS_MATRIX_SIZE; i < j; ++i)
    c[i] = data[i] + b[i];
  return c;
}
Matrix Matrix::operator- (const Matrix b) const {
  if (!THIS_MATRIX_DIMS_EQ(b)) throw "cannot do Subtraction for different Matrix dimension";
  Matrix c(cols, rows);
  for (size_t i = 0, j = THIS_MATRIX_SIZE; i < j; ++i)
    c[i] = data[i] - b[i];
  return c;
}
Matrix Matrix::operator* (const double b) const {
  Matrix c(cols, rows);
  for (size_t i = 0, j = THIS_MATRIX_SIZE; i < j; ++i)
    c[i] = data[i] * b;
  return c;
}
Matrix Matrix::operator/ (const double b) const {
  Matrix c(cols, rows);
  for (size_t i = 0, j = THIS_MATRIX_SIZE; i < j; ++i)
    c[i] = data[i] / b;
  return c;
}
Matrix Matrix::operator* (const Matrix b) const {
  if (cols != b.rows) throw "not match A cols and B rows to do multiplication";
  Matrix c(b.cols, rows);
  size_t i, j, k;
  for(i = 0; i < c.rows; ++i)
    for(j = 0; j < cols; ++j)
      for(k = 0; k < c.cols; ++k)
        c(k, i) += (*this)(j, i) * b(k, j);
  return c;
}
Matrix Matrix::operator/ (const Matrix b) const {
  return (*this)*b.inverse();
}
Matrix &Matrix::operator+= (const Matrix b) {
  if (!THIS_MATRIX_DIMS_EQ(b)) throw "cannot do Addition for different Matrix dimension";
  for (size_t i = 0, j = THIS_MATRIX_SIZE; i < j; ++i)
    data[i] += b[i];
  return *this;
}
Matrix &Matrix::operator-= (const Matrix b) {
  if (!THIS_MATRIX_DIMS_EQ(b)) throw "cannot do Subtraction for different Matrix dimension";
  for (size_t i = 0, j = THIS_MATRIX_SIZE; i < j; ++i)
    data[i] -= b[i];
  return *this;
}
Matrix &Matrix::operator*= (const double b) {
  for (size_t i = 0, j = THIS_MATRIX_SIZE; i < j; ++i)
    data[i] *= b;
  return *this;
}
Matrix &Matrix::operator/= (const double b) {
  for (size_t i = 0, j = THIS_MATRIX_SIZE; i < j; ++i)
    data[i] /= b;
  return *this;
}
Matrix &Matrix::operator*= (const Matrix b) {
  if (cols != b.rows) throw "not match A cols and B rows to do multiplication";
  double *c = (double*) calloc(b.cols*rows, sizeof(double));
  size_t i, j, k;
  for(i = 0; i < rows; ++i)
    for(j = 0; j < cols; ++j)
      for(k = 0; k < b.cols; ++k)
        c[(i * b.cols) + k] += (*this)(j, i) * b(k, j);
  free(data);
  cols = b.cols;
  data = c;
  return *this;
}
Matrix &Matrix::operator/= (const Matrix b) {
  return (*this)*=b.inverse();
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