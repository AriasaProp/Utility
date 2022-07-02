#include "../../include/utils/big_decimal.h"

using namespace std;

//private function
template <typename T>
T MIN(const T &x, const T &y)
{
	return (x<y) ? x : y;
}
template <typename T>
T MAX(const T &x, const T &y)
{
	return (x>y) ? x : y;
}
//initialize big_decimal funct
void big_decimal::fft_ensure_table(int k) const
{
	int current_k = (int)twiddle_table.size() - 1;
	if (current_k >= k)
		return;

		if (k - 1 > current_k)
	{
		fft_ensure_table(k - 1);
	}

	size_t length = (size_t)1 << k;
	double omega = 2 * M_PI / length;
	length /= 2;

		vector<complex<double>> sub_table;
	for (size_t c = 0; c < length; c++)
	{
				double angle = omega * c;
		auto twiddle_factor = complex<double>(cos(angle), sin(angle));
		sub_table.push_back(twiddle_factor);
	}

		twiddle_table.push_back(move(sub_table));
}
void big_decimal::fft_forward(complex<double> *T, int k) const
{
		
		
			
		if (k == 1)
	{
		complex<double> a = T[0];
		complex<double> b = T[1];
		T[0] = a + b;
		T[1] = a - b;
		return;
	}

	size_t length = (size_t)1 << k;
	size_t half_length = length / 2;

		const vector<complex<double>> &local_table = twiddle_table[k];

		for (size_t c = 0; c < half_length; c++)
	{
				auto twiddle_factor = local_table[c];

				complex<double> a = T[c];
		complex<double> b = T[c + half_length];

				T[c] = a + b;
		T[c + half_length] = (a - b) * twiddle_factor;
	}

		fft_forward(T, k - 1);

		fft_forward(T + half_length, k - 1);
}
void big_decimal::fft_inverse(complex<double> *T, int k) const
{
		
		
			
		if (k == 1)
	{
		complex<double> a = T[0];
		complex<double> b = T[1];
		T[0] = a + b;
		T[1] = a - b;
		return;
	}

	size_t length = (size_t)1 << k;
	size_t half_length = length / 2;

		fft_inverse(T, k - 1);

		fft_inverse(T + half_length, k - 1);

		const vector<complex<double>> &local_table = twiddle_table[k];

		for (size_t c = 0; c < half_length; c++)
	{
				auto twiddle_factor = conj(local_table[c]);

				complex<double> a = T[c];
		complex<double> b = T[c + half_length] * twiddle_factor;

				T[c] = a + b;
		T[c + half_length] = a - b;
	}
}
void big_decimal::fft_pointwise(complex<double> *T, const complex<double> *A, int k) const
{
	
			
	size_t length = (size_t)1 << k;
	for (size_t c = 0; c < length; c++)
	{
		T[c] = T[c] * A[c];
	}
}
void big_decimal::int_to_fft(complex<double> *T, int k, const uint32_t *A, size_t AL) const
{
	
					
	size_t fft_length = (size_t)1 << k;
	complex<double> *Tstop = T + fft_length;

				if (fft_length < 3 * AL)
		throw "FFT length is too small.";

		for (size_t c = 0; c < AL; c++)
	{
		uint32_t word = A[c];

		*T++ = word % 1000;
		word /= 1000;
		*T++ = word % 1000;
		word /= 1000;
		*T++ = word;
	}

		while (T < Tstop)
		*T++ = complex<double>(0, 0);
}
void big_decimal::fft_to_int(const complex<double> *T, size_t length, uint32_t *A, size_t AL) const
{
		uint64_t carry = 0;
	for (size_t c = 0; c < AL; c++)
	{
		double f_point;
		uint64_t i_point;
		uint32_t word;

		f_point = (*T++).real() / length;	 		i_point = (uint64_t)(f_point + 0.5); 		carry += i_point;					 		word = carry % 1000;				 		carry /= 1000;

		f_point = (*T++).real() / length;	 		i_point = (uint64_t)(f_point + 0.5); 		carry += i_point;					 		word += (carry % 1000) * 1000;		 		carry /= 1000;

		f_point = (*T++).real() / length;	 		i_point = (uint64_t)(f_point + 0.5); 		carry += i_point;					 		word += (carry % 1000) * 1000000;	 		carry /= 1000;

		A[c] = word;
	}
}

int64_t big_decimal::to_string_trimmed(size_t digits, string &str) const
{
	
			
	if (L == 0)
	{
		str = "0";
		return 0;
	}

		int64_t exponent = exp;
	size_t length = L;
	uint32_t *ptr = T.get();

	if (digits == 0)
	{
		digits = length * 9;
	}
	else
	{
				size_t words = (digits + 17) / 9;
		if (words < length)
		{
			size_t chop = length - words;
			exponent += chop;
			length = words;
			ptr += chop;
		}
	}
	exponent *= 9;

		char buffer[] = "012345678";
	str.clear();
	size_t c = length;
	while (c-- > 0)
	{
		uint32_t word = ptr[c];
		for (int i = 8; i >= 0; i--)
		{
			buffer[i] = word % 10 + '0';
			word /= 10;
		}
		str += buffer;
	}

		size_t leading_zeros = 0;
	while (str[leading_zeros] == '0')
		leading_zeros++;
	digits += leading_zeros;

		if (digits < str.size())
	{
		exponent += str.size() - digits;
		str.resize(digits);
	}

	return exponent;
}
void big_decimal::compres_posible_value()
{
	if (L == 0)
		return;
	uint32_t *V = T.get();
	while (V[L - 1] == 0)
		--L;
	/*
	int i = 0;
	for (; i < L - 1; ++i)
	{
		if (V[i] != 0)
			break;
		++exp;
		--L;
	}
	uint32_t v[L];
	memcpy(v, V + i, L * sizeof(uint32_t));
	T = unique_ptr<uint32_t[]>(v);
	*/
}
big_decimal::big_decimal()
	: sign(true), exp(0), L(0)
{
}
big_decimal::big_decimal(const big_decimal &x)
	: sign(x.sign), exp(x.exp), L(x.L)
{
	T = unique_ptr<uint32_t[]>(new uint32_t[x.L]);
	memcpy(T.get(), x.T.get(), x.L * sizeof(uint32_t));
}
big_decimal::big_decimal(const uint32_t &x, const bool &sign_)
	: sign(!x | sign_), exp(0), L(x != 0)
{
	if (!x)
		return;
	T = unique_ptr<uint32_t[]>(new uint32_t[1]);
	T[0] = x;
}
/*
big_decimal::big_decimal(const double &x)
	: sign(x > 0)
{
	if (!x)
		return;
	this->exp = -2;
	this->L = 5;
	this->T = unique_ptr<uint32_t[]>(new uint32_t[5]);

	this->T[0] = uint32_t(x * 1000000000000000000.);
	this->T[1] = uint32_t(x * 1000000000.);
	this->T[2] = uint32_t(x);
	this->T[3] = uint32_t(x / 1000000000.);
	this->T[4] = uint32_t(x / 1000000000000000000.);
}
*/
big_decimal::~big_decimal()
{
}

string big_decimal::to_string(size_t digits) const
{
	if (L == 0)
		return "0.";
	int64_t mag = exp + L;
	if (mag > 1 || mag < 0)
		return to_string_sci();
	string str;
	int64_t exponent = to_string_trimmed(digits, str);
	if (mag == 0)
		return (sign ? string("0.") : string("-0.")) + str;

	string after_decimal = (exponent >= 0) ? "" : str.substr((size_t)(str.size() + exponent), (size_t)-exponent);
	return string(sign ? "" : "-") + std::to_string((long long)T[L - 1]) + "." + after_decimal;
}
string big_decimal::to_string_sci(size_t digits) const
{
	if (L == 0)
		return "0.";

	string str;
	int64_t exponent = to_string_trimmed(digits, str);

	{
		size_t leading_zeros = 0;
		while (str[leading_zeros] == '0')
			leading_zeros++;
		str = &str[leading_zeros];
	}

	exponent += str.size() - 1;
	str = str.substr(0, 1) + "." + &str[1];

	if (exponent != 0)
	{
		str += " * 10^";
		str += to_string(exponent);
	}

		if (!sign)
		str = string("-") + str;

	return str;
}

size_t big_decimal::get_precision() const
{
	return L;
}
int64_t big_decimal::get_exponent() const
{
	return exp;
}
uint32_t big_decimal::word_at(int64_t mag) const
{
	if (mag < exp || mag >= exp + int64_t(L))
		return 0;
	return T[size_t(mag - exp)];
}
long double big_decimal::to_double() const
{
	long double r = 0.0;
	uint32_t *f = T.get();
	int64_t st = MAX(exp, int64_t(-30));
	int64_t ed = MIN(int64_t(L) + exp, int64_t(30));
	for (int64_t i = st; i < ed; i++)
	{
		r += double(f[i - exp]) * pow(1000000000.0, i);
	}
	return sign ? r : -r;
}
int big_decimal::count_decimal_front(double &out) const
{
	size_t a = 9 * (L - 1 + size_t(exp));
	double uT = double(T[L - 1]);
	while (uT > 10)
	{
		uT /= 10;
		a++;
	}
	out = uT;
	return a;
}

void big_decimal::negate()
{
	if (L == 0)
		return;
	sign = !sign;
}

big_decimal big_decimal::operator+(const big_decimal &x) const
{
		int64_t magA = exp + L;
	int64_t magB = x.exp + x.L;
	int64_t top = MAX(magA, magB);
	int64_t bot = MIN(exp, x.exp);
		int64_t TL = top - bot;
		big_decimal z;
	z.exp = bot;
	z.L = (uint32_t)TL;

		if (sign == x.sign)
	{
		z.sign = sign;
		z.T = unique_ptr<uint32_t[]>(new uint32_t[z.L + 1]);
				uint32_t carry = 0;
		for (size_t c = 0; bot < top; bot++, c++)
		{
			uint32_t word = word_at(bot) + x.word_at(bot) + carry;
			carry = 0;
			if (word >= 1000000000)
			{
				word -= 1000000000;
				carry = 1;
			}
			z.T[c] = word;
		}
		if (carry != 0)
		{
			z.T[z.L++] = 1;
		}
	}
	else
	{
		bool ucmp = false; 		if (magA != magB)
			ucmp = magA > magB;
		else
		{
			for (int64_t mag = magA; mag >= exp || mag >= x.exp; mag--)
			{
				uint32_t wordA = word_at(mag);
				uint32_t wordB = x.word_at(mag);
				if (wordA != wordB)
				{
					ucmp = wordA > wordB;
					break;
				}
			}
		}
		z.T = unique_ptr<uint32_t[]>(new uint32_t[z.L]);
		int32_t carry = 0;
		if (ucmp)
		{
			z.sign = sign;
						for (size_t c = 0; bot < top; bot++, c++)
			{
				int32_t word = (int32_t)word_at(bot) - (int32_t)x.word_at(bot) - carry;
				carry = 0;
				if (word < 0)
				{
					word += 1000000000;
					carry = 1;
				}
				z.T[c] = word;
			}
		}
		else
		{
			z.sign = x.sign;
						for (size_t c = 0; bot < top; bot++, c++)
			{
				int32_t word = (int32_t)x.word_at(bot) - (int32_t)word_at(bot) - carry;
				carry = 0;
				if (word < 0)
				{
					word += 1000000000;
					carry = 1;
				}
				z.T[c] = word;
			}
		}
				while (z.L > 0 && z.T[z.L - 1] == 0)
			z.L--;
		if (z.L == 0)
		{
			z.exp = 0;
			z.sign = true;
			z.T.reset();
		}
	}
	z.compres_posible_value();
	return z;
}

big_decimal big_decimal::operator-(const big_decimal &x) const
{
		int64_t magA = exp + L;
	int64_t magB = x.exp + x.L;
	int64_t top = MAX(magA, magB);
	int64_t bot = MIN(exp, x.exp);
		int64_t TL = top - bot;

		big_decimal z;
	z.exp = bot;
	z.L = (uint32_t)TL;
		if (sign != x.sign)
	{
		z.sign = sign;
		z.T = unique_ptr<uint32_t[]>(new uint32_t[z.L + 1]);
				uint32_t carry = 0;
		for (size_t c = 0; bot < top; bot++, c++)
		{
			uint32_t word = word_at(bot) + x.word_at(bot) + carry;
			carry = 0;
			if (word >= 1000000000)
			{
				word -= 1000000000;
				carry = 1;
			}
			z.T[c] = word;
		}
				if (carry != 0)
		{
			z.T[z.L++] = 1;
		}
	}
	else
	{
		bool ucmp = false; 		if (magA != magB)
			ucmp = magA > magB;
		else
		{
			for (int64_t mag = magA; mag >= exp || mag >= x.exp; mag--)
			{
				uint32_t wordA = word_at(mag);
				uint32_t wordB = x.word_at(mag);
				if (wordA != wordB)
				{
					ucmp = wordA > wordB;
					break;
				}
			}
		}
		z.T = unique_ptr<uint32_t[]>(new uint32_t[z.L]);
		int32_t carry = 0;
		if (ucmp)
		{
			z.sign = sign;
						for (size_t c = 0; bot < top; bot++, c++)
			{
				int32_t word = (int32_t)word_at(bot) - (int32_t)x.word_at(bot) - carry;
				carry = 0;
				if (word < 0)
				{
					word += 1000000000;
					carry = 1;
				}
				z.T[c] = word;
			}
		}
		else
		{
			z.sign = x.sign;
						for (size_t c = 0; bot < top; bot++, c++)
			{
				int32_t word = (int32_t)x.word_at(bot) - (int32_t)word_at(bot) - carry;
				carry = 0;
				if (word < 0)
				{
					word += 1000000000;
					carry = 1;
				}
				z.T[c] = word;
			}
			z.negate();
		}
				while (z.L > 0 && z.T[z.L - 1] == 0)
			z.L--;
		if (z.L == 0)
		{
			z.exp = 0;
			z.sign = true;
			z.T.reset();
		}
	}
	z.compres_posible_value();
	return z;
}

big_decimal big_decimal::operator*(const uint32_t &x) const
{
	if (L == 0 || x == 0)
		return big_decimal();
	big_decimal z;
	z.sign = sign;
	z.exp = exp;
	z.L = L;
	z.T = unique_ptr<uint32_t[]>(new uint32_t[z.L + 1]);

	uint64_t carry = 0;
	for (size_t c = 0; c < L; c++)
	{
		carry += (uint64_t)T[c] * x;			 		z.T[c] = (uint32_t)(carry % 1000000000); 		carry /= 1000000000;					 	}
	if (carry != 0)
		z.T[z.L++] = (uint32_t)carry;
	return z;
}

big_decimal big_decimal::operator*(const big_decimal &x) const
{
	if (L == 0 || x.L == 0)
		return big_decimal();

	size_t p = L + x.L;
		int64_t Aexp = exp;
	int64_t Bexp = x.exp;
	size_t AL = L;
	size_t BL = x.L;
	uint32_t *AT = T.get();
	uint32_t *BT = x.T.get();

		if (AL > p)
	{
		size_t chop = AL - p;
		AL = p;
		Aexp += chop;
		AT += chop;
	}
	if (BL > p)
	{
		size_t chop = BL - p;
		BL = p;
		Bexp += chop;
		BT += chop;
	}

		big_decimal z;
	z.sign = ~(sign ^ x.sign); 	z.exp = Aexp + Bexp;	   	z.L = AL + BL;			   
		z.T = unique_ptr<uint32_t[]>(new uint32_t[z.L]);

			int k = 0;
	size_t length = 1;
	while (length < 3 * z.L)
	{
		length <<= 1;
		k++;
	}
	if (k > 29)
		throw "FFT size limit exceeded.";

		complex<double> *Ta = new complex<double>[length];
	complex<double> *Tb = new complex<double>[length];

		fft_ensure_table(k);
	{ 		complex<double> *T = Ta;
		complex<double> *Tstop = T + length;
		for (size_t c = 0; c < AL; c++)
		{
			uint32_t word = AT[c];

			*T++ = word % 1000;
			word /= 1000;
			*T++ = word % 1000;
			word /= 1000;
			*T++ = word;
		}

				while (T < Tstop)
			*T++ = complex<double>(0, 0);
	}
	{ 		complex<double> *T = Tb;
		complex<double> *Tstop = T + length;
		for (size_t c = 0; c < BL; c++)
		{
			uint32_t word = BT[c];

			*T++ = word % 1000;
			word /= 1000;
			*T++ = word % 1000;
			word /= 1000;
			*T++ = word;
		}
		while (T < Tstop)
			*T++ = complex<double>(0, 0);
	}

	fft_forward(Ta, k);							fft_forward(Tb, k);							fft_pointwise(Ta, Tb, k);					fft_inverse(Ta, k);							fft_to_int(Ta, length, z.T.get(), z.L); 
	delete[] Ta;
	delete[] Tb;
		z.compres_posible_value();
	return z;
}

big_decimal big_decimal::rcp(size_t p) const
{
			if (L == 0)
		throw "Divide by Zero";
		int64_t Aexp = exp;
	size_t AL = L;
	uint32_t *AT = T.get();
		if (p == 0)
	{
				p = 3;
		if (AL > p)
		{
			size_t chop = AL - p;
			AL = p;
			Aexp += chop;
			AT += chop;
		}

				double val = AT[0];
		if (AL >= 2)
			val += AT[1] * 1000000000.;
		if (AL >= 3)
			val += AT[2] * 1000000000000000000.;

				val = 1. / val;
		Aexp = -Aexp;

				while (val < 1000000000.)
		{
			val *= 1000000000.;
			Aexp--;
		}

				uint64_t val64 = (uint64_t)val;

		big_decimal out;
		out.sign = sign;

		out.T = std::unique_ptr<uint32_t[]>(new uint32_t[2]);
		out.T[0] = (uint32_t)(val64 % 1000000000);
		out.T[1] = (uint32_t)(val64 / 1000000000);
		out.L = 2;
		out.exp = Aexp;

		return out;
	}

		size_t s = p / 2 + 1;
	if (p == 1)
		s = 0;
	if (p == 2)
		s = 1;

		big_decimal T = rcp(s);
		return T * 2 - *this * T * T;
}

big_decimal big_decimal::rcp() const
{
	return rcp(6);
}
big_decimal big_decimal::operator/(const uint32_t &x) const
{
	big_decimal d;
	if (x == 0 || x == 1)
		return *this;
	long double val = 1. / (long double)x;
	d.sign = sign;
	d.T = std::unique_ptr<uint32_t[]>(new uint32_t[2]);
	d.L = 2;
	d.exp = 0;
		while (val < 1000000000.)
	{
		val *= 1000000000.;
		d.exp--;
	}
	uint64_t val64 = (uint64_t)val;
	d.T[0] = (uint32_t)(val64 % 1000000000);
	d.T[1] = (uint32_t)(val64 / 1000000000);
	d.compres_posible_value();
	return *this * d;
}
big_decimal big_decimal::operator/(const big_decimal &x) const
{
	return *this * x.rcp();
}

big_decimal big_decimal::operator=(const big_decimal &x)
{
	sign = x.sign;
	exp = x.exp;
	L = x.L;
	T = unique_ptr<uint32_t[]>(new uint32_t[x.L]);
	memcpy(T.get(), x.T.get(), x.L * sizeof(uint32_t));
	return *this;
}
big_decimal big_decimal::operator+=(const big_decimal &x)
{
	return ( (*this) = (*this) + x);
	/*
	int64_t magA = exp + L;
	int64_t magB = x.exp + x.L;
	int64_t top = MAX(magA, magB);
	int64_t bot = MIN(exp, x.exp);
	int64_t TL = top - bot;
	big_decimal z;
	z.exp = bot;
	z.L = (uint32_t)TL;

	if (sign == x.sign)
	{
		z.sign = sign;
		z.T = unique_ptr<uint32_t[]>(new uint32_t[z.L + 1]);
				uint32_t carry = 0;
		for (size_t c = 0; bot < top; bot++, c++)
		{
			uint32_t word = word_at(bot) + x.word_at(bot) + carry;
			carry = 0;
			if (word >= 1000000000)
			{
				word -= 1000000000;
				carry = 1;
			}
			z.T[c] = word;
		}
		if (carry != 0)
		{
			z.T[z.L++] = 1;
		}
	}
	else
	{
		bool ucmp = false; 		if (magA != magB)
			ucmp = magA > magB;
		else
		{
			for (int64_t mag = magA; mag >= exp || mag >= x.exp; mag--)
			{
				uint32_t wordA = word_at(mag);
				uint32_t wordB = x.word_at(mag);
				if (wordA != wordB)
				{
					ucmp = wordA > wordB;
					break;
				}
			}
		}
		z.T = unique_ptr<uint32_t[]>(new uint32_t[z.L]);
		int32_t carry = 0;
		if (ucmp)
		{
			z.sign = sign;
						for (size_t c = 0; bot < top; bot++, c++)
			{
				int32_t word = (int32_t)word_at(bot) - (int32_t)x.word_at(bot) - carry;
				carry = 0;
				if (word < 0)
				{
					word += 1000000000;
					carry = 1;
				}
				z.T[c] = word;
			}
		}
		else
		{
			z.sign = x.sign;
						for (size_t c = 0; bot < top; bot++, c++)
			{
				int32_t word = (int32_t)x.word_at(bot) - (int32_t)word_at(bot) - carry;
				carry = 0;
				if (word < 0)
				{
					word += 1000000000;
					carry = 1;
				}
				z.T[c] = word;
			}
		}
				while (z.L > 0 && z.T[z.L - 1] == 0)
			z.L--;
		if (z.L == 0)
		{
			z.exp = 0;
			z.sign = true;
			z.T.reset();
		}
	}
	z.compres_posible_value();
	return z;
	*/
}
big_decimal big_decimal::operator-=(const big_decimal &x)
{
	return ( (*this) = (*this) - x);
}
big_decimal big_decimal::operator*=(const big_decimal &x)
{
	return ( (*this) = (*this) * x);
}
big_decimal big_decimal::operator/=(const big_decimal &x)
{
	return ( (*this) = (*this) / x);
}