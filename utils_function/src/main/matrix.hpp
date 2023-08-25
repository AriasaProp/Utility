#ifndef _matrix_INCLUDED_
#define _matrix_INCLUDED_

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
public:
    matrix(const float[V][H]);

    void print() const;
private:
    float data[V][H];
};



#endif //_matrix_INCLUDED_