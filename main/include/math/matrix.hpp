#ifndef _MATRIX_INCLUDED_
#define _MATRIX_INCLUDED_

#include <ostream>
#include <vector>

#include "unit.hpp"

typedef double vs; // value system

/*
Matrix

format M[cols rows]{<value>}

*/

struct Matrix {
private:
  size_t cols, rows;
  vs *data;

public:
  // constructors
  Matrix ();
  Matrix (size_t, size_t);
  Matrix (size_t, size_t, vs*);
  Matrix (const char *);
  Matrix (const Matrix &);
  // destructors
  ~Matrix ();
  // operators function
  double &operator[] (const size_t) const;
  double &operator() (const size_t, const size_t) const;
  // unique function
  friend Matrix identity (const Matrix&);
  friend Matrix inverse (const Matrix&);
  // operator compare
  friend bool operator== (const Matrix&, const Matrix&);
  friend bool operator!= (const Matrix&, const Matrix&);
  // operators math
  Matrix &operator= (const Matrix);
  friend Matrix operator+ (const Matrix&, const Matrix);
  friend Matrix &operator+= (Matrix&, const Matrix);
  friend Matrix operator- (const Matrix&, const Matrix);
  friend Matrix &operator-= (Matrix&, const Matrix);
  friend Matrix operator* (const Matrix&, const double);
  friend Matrix &operator*= (Matrix&, const double);
  friend Matrix operator* (const Matrix&, const Matrix);
  friend Matrix &operator*= (Matrix&, const Matrix);
  friend Matrix operator/ (const Matrix&, const double);
  friend Matrix &operator/= (Matrix&, const double);
  friend Matrix operator/ (const Matrix&, const Matrix);
  friend Matrix &operator/= (Matrix&, const Matrix);
  /** stream operator **/
  friend std::ostream &operator<< (std::ostream &, const Matrix&);
};

#endif //_MATRIX_INCLUDED_