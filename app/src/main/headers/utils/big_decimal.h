#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdint>
#include <cstring>
#include <complex>
#include <memory>
#include <vector>

using namespace std;

struct big_decimal
{
  private:
	bool sign;	 //  true = positive or zero, false = negative
	int64_t exp; //  Exponent
	size_t L;	 //  Length
	unique_ptr<uint32_t[]> T;

	mutable vector<vector<complex<double>>> twiddle_table;

	//fft function
	void fft_ensure_table(int k) const;
	void fft_forward(complex<double> *T, int k) const;
	void fft_inverse(complex<double> *T, int k) const;
	void fft_pointwise(complex<double> *T, const complex<double> *A, int k) const;
	void int_to_fft(complex<double> *T, int k, const uint32_t *A, size_t AL) const;
	void fft_to_int(const complex<double> *T, size_t length, uint32_t *A, size_t AL) const;

	//  Internal helpers
	int64_t to_string_trimmed(size_t digits, string &str) const;
	void compres_posible_value();

  public:
	// Constructors
	big_decimal();
	big_decimal(const big_decimal &x);
	big_decimal(const uint32_t &x, const bool &sign = true);
	//big_decimal(const double &x);
	//Destructors
	~big_decimal();

	//Helpers
	string to_string(size_t digits = 0) const;
	string to_string_sci(size_t digits = 0) const;
	size_t get_precision() const;
	int64_t get_exponent() const;
	uint32_t word_at(int64_t mag) const;
	long get_integer() const;
	long double to_double() const;
	int count_decimal_front(double &out) const;

	//Aritmathics
	void negate();
	big_decimal operator+(const big_decimal &x) const;
	big_decimal operator-(const big_decimal &x) const;
	big_decimal operator*(const uint32_t &x) const;
	big_decimal operator*(const big_decimal &x) const;
	big_decimal rcp(size_t p) const;
	big_decimal rcp() const;
	big_decimal operator/(const uint32_t &x) const;
	big_decimal operator/(const big_decimal &x) const;
	//Aritmathics sets
	big_decimal operator=(const big_decimal &x);
	big_decimal operator+=(const big_decimal &x);
	big_decimal operator-=(const big_decimal &x);
	big_decimal operator*=(const big_decimal &x);
	big_decimal operator/=(const big_decimal &x);
};