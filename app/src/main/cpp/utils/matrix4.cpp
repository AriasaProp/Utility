#include "utils/matrix4.h"
#include "../adaptive_funct.h"
#include <stddef.h>

static const size_t MATRIX_SIZE = sizeof(float) * 16;

//matrix4 constructors
matrix4::matrix4() {}
matrix4::matrix4(float v[16])
{
    memcpy(values, v, MATRIX_SIZE);
}
matrix4::matrix4(const matrix4 &other)
{
    memcpy(values, other.values, MATRIX_SIZE);
}

//matriks function
float matrix4::det() const
{
    float result;
    result += values[0] * values[5] * values[10] * values[15];
    result += values[1] * values[6] * values[11] * values[12];
    result += values[2] * values[7] * values[8] * values[13];
    result += values[3] * values[4] * values[9] * values[14];
    result -= values[0] * values[7] * values[10] * values[13];
    result -= values[1] * values[4] * values[11] * values[14];
    result -= values[2] * values[5] * values[8] * values[11];
    result -= values[3] * values[6] * values[9] * values[12];
    return result;
}


//math operation safe
matrix4 &matrix4::operator+=(const matrix4 &v)
{
    for (size_t i = 0; i < 16; i++)
        this->values[i] += v.values[i];
    return *this;
}
matrix4 &matrix4::operator-=(const matrix4 &v)
{
    for (size_t i = 0; i < 16; i++)
        this->values[i] -= v.values[i];
    return *this;
}
matrix4 &matrix4::operator*=(const matrix4 &v)
{
    float a[16], b[16];
    memcpy(a, this->values, MATRIX_SIZE);
    memcpy(b, v.values, MATRIX_SIZE);
    for (size_t j = 0; j < 4; j++)
    {
        for (size_t i = 0; i < 4; i++)
        {
            size_t j4 = j * 4;
            this->values[j4 + i] = a[j4] * b[i];
            this->values[j4 + i] += a[j4+1] * b[i+4];
            this->values[j4 + i] += a[j4+2] * b[i+8];
            this->values[j4 + i] += a[j4+3] * b[i+12];
        }
    }
}
//math operation unsafe
matrix4 matrix4::operator+(const matrix4 &v) const
{
    matrix4 result;
    for (size_t i = 0; i < 16; i++)
        result.values[i] = this->values[i] + v.values[i];
    return result;
}
matrix4 matrix4::operator-(const matrix4 &v) const
{
    matrix4 result;
    for (size_t i = 0; i < 16; i++)
        result.values[i] = this->values[i] - v.values[i];
    return result;
}
matrix4 matrix4::operator*(const matrix4 &v) const
{
    matrix4 result;
    for (size_t j = 0; j < 4; j++)
    {
        for (size_t i = 0; i < 4; i++)
        {
            size_t j4 = j * 4;
            result.values[j4 + i] = this->values[j4] * v.values[i];
            result.values[j4 + i] += this->values[j4 + 1] * v.values[i + 4];
            result.values[j4 + i] += this->values[j4 + 2] * v.values[i + 8];
            result.values[j4 + i] += this->values[j4 + 3] * v.values[i + 12];
        }
    }
    return result;
}
