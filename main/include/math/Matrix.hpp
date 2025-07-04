#ifndef _MATRIX_INCLUDED_
#define _MATRIX_INCLUDED_

#include <ostream>
#include <vector>

#include "unit.hpp"

/*
Matrix

format M[cols rows]{<value>}

*/

struct Matrix {
private:
  size_t cols, rows;
  double *data;

public:
  static Matrix identity (size_t, size_t);
  // constructors
  Matrix ();
  // matrix string format [cols rows]{v .... }
  Matrix (const char *);
  Matrix (size_t, size_t);
  Matrix (size_t, size_t, double*);
  Matrix (const Matrix &);
  // destructors
  ~Matrix ();
  // operators function
  double &operator[] (const size_t) const;
  double &operator() (const size_t, const size_t) const;
  // unique function
  Matrix inverse () const;
  // operator compare
  bool operator== (const Matrix&) const;
  bool operator!= (const Matrix&) const;
  // operators math
  Matrix &operator= (const Matrix);
  Matrix operator+ (const Matrix) const;
  Matrix operator- (const Matrix) const;
  Matrix operator* (const double) const;
  Matrix operator/ (const double) const;
  Matrix operator* (const Matrix) const;
  Matrix operator/ (const Matrix) const;
  Matrix &operator+= (const Matrix);
  Matrix &operator-= (const Matrix);
  Matrix &operator*= (const double);
  Matrix &operator/= (const double);
  Matrix &operator*= (const Matrix);
  Matrix &operator/= (const Matrix);
  /** stream operator **/
  friend std::ostream &operator<< (std::ostream &, const Matrix&);
};

#endif //_MATRIX_INCLUDED_