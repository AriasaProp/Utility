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
    matrix(const std::array<std::array<float, V>, H>& data);

    void print() const;

private:
    std::array<std::array<float, V>, H> matrixData;
};



#endif //_matrix_INCLUDED_