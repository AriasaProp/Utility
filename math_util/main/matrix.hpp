#ifndef _MATRIX_INCLUDED_
#define _MATRIX_INCLUDED_

#include <initializer_list>

/*
   Rows ->
Cols
 |       a    b    c    d ....
 v       aa   ab   ac   ad .....
         ba   bb   bc   bd .....
       .... ..... .... ... .....
       ...a  ..b  ..c  ..d .....
*/

struct matrix2D {
private:
  unsigned cols, rows;
  float *data;

public:
  // constructors
  matrix2D ();
  matrix2D (const matrix2D &);
  matrix2D (unsigned, unsigned, const std::initializer_list<float>);
  matrix2D (unsigned, unsigned, const float *);
  // destructors
  ~matrix2D ();
  // unique function
  matrix2D &identity ();
  matrix2D inverse () const;
  float det () const;
  // operators function
  float &operator() (unsigned, unsigned);
  // operator compare
  bool operator== (const matrix2D &) const;
  bool operator!= (const matrix2D &) const;
  // operators math
  matrix2D &operator= (const matrix2D &);
  matrix2D operator+ (const matrix2D &) const;
  matrix2D &operator+= (const matrix2D);
  matrix2D operator- (const matrix2D &) const;
  matrix2D &operator-= (const matrix2D);
  matrix2D operator* (const float &) const;
  matrix2D &operator*= (const float &);
  matrix2D operator* (const matrix2D &) const;
  matrix2D &operator*= (const matrix2D);
  matrix2D operator/ (const float &) const;
  matrix2D &operator/= (const float &);
  matrix2D operator/ (const matrix2D &) const;
  matrix2D &operator/= (const matrix2D);
  // helper
  void print () const;
  unsigned size () const;
};

#endif //_MATRIX_INCLUDED_