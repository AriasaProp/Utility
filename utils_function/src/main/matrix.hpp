#ifndef _MATRIX_INCLUDED_
#define _MATRIX_INCLUDED_

#include <iostream>
#include <iomanip>
#include <sstream>

/*
   Horizontal ->
Vertical  
 |       a    b    c    d ....
 v       aa   ab   ac   ad .....
         ba   bb   bc   bd .....
       .... ..... .... ... .....
       ...a  ..b  ..c  ..d .....
*/

template <unsigned H, unsigned V>
struct matrix {
private:
    float data[V][H] {};
public:
    matrix();
    ~matrix();

    float& operator[](unsigned);
    float& operator()(unsigned, unsigned);
    matrix operator+(const matrix&);
    matrix operator*(const matrix&);
    
    void printInfo();

    size_t number_of_digits(float&);
    void print_matrix();
};


#endif //_MATRIX_INCLUDED_