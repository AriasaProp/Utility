#ifndef Included_Matrix4
#define Included_Matrix4 1

/*
matrix sort 
0, 1, 2, 3,
4, 5, 6, 7,
8, 9, 10, 11,
12, 13, 14, 15
*/
class matrix4 {
private:
	float values[16];
public:
	//constructors
	matrix4();
	matrix4(const float (&)[16]);
	matrix4(const matrix4 &);
	//destructors
	~matrix4();
	//re-initialize
	matrix4 &operator=(const float (&)[16]);
	matrix4 &operator=(const matrix4 &);
	//compare function
	bool operator==(const matrix4 &) const;
	//matrix function
	float det() const;
	matrix4 &adj() const;
	matrix4 invers() const;
	float *getValues() const;
	//math operation safe
	matrix4 &operator+=(const matrix4 &);
	matrix4 &operator-=(const matrix4 &);
	matrix4 &operator*=(const float &);
	matrix4 &operator*=(const matrix4 &);
	matrix4 &operator/=(const float &);
	matrix4 &operator/=(const matrix4 &);
	//math operation unsafe
	matrix4 operator+(const matrix4 &) const;
	matrix4 operator-(const matrix4 &) const;
	matrix4 operator*(const float &) const;
	matrix4 operator*(const matrix4 &) const;
	matrix4 operator/(const float &) const;
	matrix4 operator/(const matrix4 &) const;
};
#endif //Included_Matrix_4x4