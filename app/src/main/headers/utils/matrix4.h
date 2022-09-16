#ifndef Included_Matrix4
#define Included_Matrix4

class matrix4
{
private:
    /*
    matrix sort 
    0, 1, 2, 3,
    4, 5, 6, 7,
    8, 9, 10, 11,
    12, 13, 14, 15
    */
    float values[16];
    
    
public:
    matrix4();
    matrix4(float[16]);
    matrix4(const matrix4 &);
    //matrix function
    float det() const;
    //math operation safe
    matrix4 &operator+=(const matrix4 &);
    matrix4 &operator-=(const matrix4 &);
    matrix4 &operator*=(const matrix4 &);
    //math operation unsafe
    matrix4 operator+(const matrix4 &) const;
    matrix4 operator-(const matrix4 &) const;
    matrix4 operator*(const matrix4 &) const;
}

#endif //Included_Matrix_4x4