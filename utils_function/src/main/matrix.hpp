#ifndef _MATRIX_INCLUDED_
#define _MATRIX_INCLUDED_

#include <initializer_list>
#include <cstddef>
#include <iostream>
#include <array>

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
    unsigned cols, row;
    float *data;
public:
    matrix2D(unsigned, unsigned, const float*);
    ~matrix2D ();
    matrix2D operator=(const float *);
    void print() const;
};



#endif //_MATRIX_INCLUDED_