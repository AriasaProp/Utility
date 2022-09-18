#include <iostream>
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
	int64_t to_string_trimmed(size_t, string &) const;
	void compres_posible_value();
	string to_string(size_t) const;
	string to_string_sci() const;
	string to_string_sci(size_t) const;
	int count_decimal_front(double &) const;
	uint32_t word_at(int64_t) const;
	big_decimal rcp() const;
	big_decimal rcp(size_t) const;

  public:
	// Constructors
	big_decimal();
	big_decimal(const big_decimal &);
	big_decimal(const uint32_t &, const bool &);
	big_decimal(const double &);
	//Destructors
	~big_decimal();
	
	//helpers
	string to_string() const;
	int get_integer() const;
	double to_double() const;
	size_t get_precision() const;
	int64_t get_exponent() const;
	
	//re-initialize
	big_decimal &operator=(const big_decimal &);

	//operational math unsafe
	big_decimal operator-() const;
	big_decimal operator+(const big_decimal &) const;
	big_decimal operator-(const big_decimal &) const;
	big_decimal operator*(const uint32_t &) const;
	big_decimal operator*(const big_decimal &) const;
	big_decimal operator/(const uint32_t &) const;
	big_decimal operator/(const big_decimal &) const;
	//operational math safe
	big_decimal &operator+=(const big_decimal &);
	big_decimal &operator-=(const big_decimal &);
	big_decimal &operator*=(const big_decimal &);
	big_decimal &operator/=(const big_decimal &);
	
	//output
	friend std::ostream &operator<<(std::ostream &, const big_decimal &);
};
