#ifndef _MATRIX_INCLUDED_
#define _MATRIX_INCLUDED_

#include <initializer_list>
#include <cstddef>

/*
   Horizontal ->
Vertical  
 |       a    b    c    d ....
 v       aa   ab   ac   ad .....
         ba   bb   bc   bd .....
       .... ..... .... ... .....
       ...a  ..b  ..c  ..d .....
*/

template <size_t H, size_t V>
struct matrix {
private:
    float data[V][H] {};
public:
    matrix();
    matrix(std::initializer_list<std::initializer_list<const float>>);
    ~matrix();

    float& operator[](size_t);
    float& operator()(size_t, size_t);
    matrix operator+(const matrix&);
    matrix operator*(const matrix&);
    
    void printInfo();

    size_t number_of_digits(float&);
    void print_matrix();
};


#endif //_MATRIX_INCLUDED_