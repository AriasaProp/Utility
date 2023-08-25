#ifndef _MATRIX_INCLUDED_
#define _MATRIX_INCLUDED_

#include <initializer_list>
#include <cstddef>
#include <iostream>
#include <array>

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
class matrix {
private:
    float data[V][H];
public:
    matrix();
    matrix<H, V> operator=(const float[V][H]);
    void print() const;
};



#endif //_MATRIX_INCLUDED_