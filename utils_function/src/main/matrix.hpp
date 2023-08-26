#ifndef _MATRIX_INCLUDED_
#define _MATRIX_INCLUDED_

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
    //constructors
    matrix2D();
    matrix2D(const matrix2D&);
    matrix2D(unsigned, unsigned, const float*);
    //destructors
    ~matrix2D ();
    //unique function
    void invert();
    float determinant();
    void adj();
    //operators function
    float &operator()(unsigned, unsigned);
    //operators math
    matrix2D &operator=(const matrix2D&);
    matrix2D operator+(const matrix2D&) const;
    matrix2D &operator+=(const matrix2D&);
    matrix2D operator-(const matrix2D&) const;
    matrix2D &operator-=(const matrix2D&);
    matrix2D operator*(const float&) const;
    matrix2D &operator*=(const float&);
    matrix2D operator*(const matrix2D&) const;
    matrix2D &operator*=(const matrix2D&);
    matrix2D operator/(const float&) const;
    matrix2D &operator/=(const float&);
    matrix2D operator/(matrix2D) const;
    matrix2D &operator/=(matrix2D);
    //helper
    void print() const;
    unsigned size() const;
};



#endif //_MATRIX_INCLUDED_