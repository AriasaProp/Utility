#ifndef _MATRIX_INCLUDED_
#define _MATRIX_INCLUDED_

#include <initializer_list>

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
    matrix(std::initializer_list<std::initializer_list<const float>>);
    ~matrix();

    float& operator[](unsigned);
    float& operator()(unsigned, unsigned);
    matrix operator+(const matrix&);
    matrix operator*(const matrix&);
    
    void printInfo();

    unsigned number_of_digits(float&);
    void print_matrix();
};


#endif //_MATRIX_INCLUDED_