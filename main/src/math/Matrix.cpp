#include "math/Matrix.hpp"
#include "common.hpp"

#include <cstring>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <string>
#include <stdexcept> // Untuk std::invalid_argument


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

#define THIS_MATRIX_SIZE (rows * cols)
#define THIS_MATRIX_SIZE_BYTE (THIS_MATRIX_SIZE * sizeof(double))
#define THIS_MATRIX_IS_SQUARE (rows == cols)
#define THIS_MATRIX_DIMS_EQ(B) ((rows == B.rows) * (cols == B.cols))

#define MATRIX_SIZE(A) (A.rows * A.cols)
#define MATRIX_SIZE_BYTE(A) (MATRIX_SIZE(A) * sizeof(double))
#define MATRIX_IS_SQUARE(A) (A.rows == A.cols)
#define MATRIX_DIMS_EQ(A, B) (A.rows == B.rows) * (A.cols == B.cols)

// constructors
Matrix::Matrix () {}
Matrix::Matrix (const char *c) {
  char *array = (char*)malloc(2048);
  char *a=array;
  if (sscanf(c, "[%zu %zu]{%[^}]}", &cols, &rows, array) < 3)
    throw c;
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
Matrix Matrix::identity () const {
  Matrix b(cols, rows);
  size_t i, j = MATRIX_SIZE(b);
  for(i = 0; i < j; i += b.cols + 1)
    b[i] = 1.0;
  return b;
}
Matrix Matrix::inverse () const {
  if (!THIS_MATRIX_IS_SQUARE) throw "Matrix not square";
  size_t n = cols, i, j, k;
  Matrix b(n, n);
#define EPSILON 1e-9
  double** augmented = (double**)malloc(n * sizeof(double*));
  for (i = 0; i < n; ++i) {
    augmented[i] = (double*)malloc(2 * n * sizeof(double));
    for (j = 0; j < n; ++j) {
      augmented[i][j] = (*this)(j, i);            // Kiri: matriks A
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
bool Matrix::operator== (const Matrix &b) const {
  return THIS_MATRIX_DIMS_EQ(b) * (0 == memcmp(data, b.data, THIS_MATRIX_SIZE_BYTE));
}
bool Matrix::operator!= (const Matrix &b) const {
  return !THIS_MATRIX_DIMS_EQ(b) * memcmp(data, b.data, THIS_MATRIX_SIZE_BYTE);
}
  // operators math
Matrix &Matrix::operator= (const Matrix b) {
  size_t by = MATRIX_SIZE_BYTE(b);
  if (!THIS_MATRIX_DIMS_EQ(b)) {
    cols = b.cols;
    rows = b.rows;
    data = (double*) realloc (data, by);
  }
  memcpy(data, b.data, by);
  return *this;
}
Matrix Matrix::operator+ (const Matrix b) const {
  if (!THIS_MATRIX_DIMS_EQ(b)) throw "cannot do Addition for different Matrix dimension";
  Matrix c(cols, rows);
  for (size_t i = 0; i < THIS_MATRIX_SIZE; ++i)
    c[i] = data[i] + b[i];
  return c;
}
Matrix Matrix::operator- (const Matrix b) const {
  if (!THIS_MATRIX_DIMS_EQ(b)) throw "cannot do Subtraction for different Matrix dimension";
  Matrix c(cols, rows);
  for (size_t i = 0; i < THIS_MATRIX_SIZE; ++i)
    c[i] = data[i] - b[i];
  return c;
}
Matrix Matrix::operator* (const double b) const {
  Matrix c(cols, rows);
  for (size_t i = 0; i < THIS_MATRIX_SIZE; ++i)
    c[i] = data[i] * b;
  return c;
}
Matrix Matrix::operator/ (const double b) const {
  Matrix c(cols, rows);
  for (size_t i = 0; i < THIS_MATRIX_SIZE; ++i)
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
  for (size_t i = 0; i < THIS_MATRIX_SIZE; ++i)
    data[i] += b[i];
  return *this;
}
Matrix &Matrix::operator-= (const Matrix b) {
  if (!THIS_MATRIX_DIMS_EQ(b)) throw "cannot do Subtraction for different Matrix dimension";
  for (size_t i = 0; i < THIS_MATRIX_SIZE; ++i)
    data[i] -= b[i];
  return *this;
}
Matrix &Matrix::operator*= (const double b) {
  for (size_t i = 0; i < THIS_MATRIX_SIZE; ++i)
    data[i] *= b;
  return *this;
}
Matrix &Matrix::operator/= (const double b) {
  for (size_t i = 0; i < THIS_MATRIX_SIZE; ++i)
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