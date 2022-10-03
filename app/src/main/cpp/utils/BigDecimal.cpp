#include "utils/BigDecimal.h"

//Constructors
BigDecimal::BigDecimal() {}
BigDecimal::BigDecimal(const BigDecimal &) {}
BigDecimal::BigDecimal(const signed &) {}
BigDecimal::BigDecimal(const char *) {}
//Destructor
BigDecimal::~BigDecimal() {}
//environment count
size_t BigDecimal::tot() const
{
		return 0;
}
char *BigDecimal::to_chars() const
{
		return 0;
}
double BigDecimal::to_double() const
{
		return 0;
}
int BigDecimal::to_int() const
{
		return 0;
}
// math operational function
BigDecimal BigDecimal::sqrt() const
{
		return 0;
}
// re-initialize operational function
BigDecimal &BigDecimal::operator=(const signed &)
{
		return *this;
}
BigDecimal &BigDecimal::operator=(const BigDecimal &)
{
		return *this;
}
// safe operator math function
BigDecimal &BigDecimal::operator--()
{
		return *this;
}
BigDecimal &BigDecimal::operator++()
{
		return *this;
}
BigDecimal &BigDecimal::operator+=(const signed &)
{
		return *this;
}
BigDecimal &BigDecimal::operator+=(const BigDecimal &)
{
		return *this;
}
BigDecimal &BigDecimal::operator-=(const signed &)
{
		return *this;
}
BigDecimal &BigDecimal::operator-=(const BigDecimal &)
{
		return *this;
}
BigDecimal &BigDecimal::operator*=(const signed &)
{
		return *this;
}
BigDecimal &BigDecimal::operator*=(const BigDecimal &)
{
		return *this;
}
BigDecimal &BigDecimal::operator/=(const signed &)
{
		return *this;
}
BigDecimal &BigDecimal::operator/=(const BigDecimal &)
{
		return *this;
}
BigDecimal &BigDecimal::operator^=(size_t)
{
		return *this;
}
// compare operator function
bool BigDecimal::operator==(const BigDecimal &) const
{
		return false;
}
bool BigDecimal::operator!=(const BigDecimal &) const
{
		return false;
}
bool BigDecimal::operator<=(const BigDecimal &) const
{
		return false;
}
bool BigDecimal::operator>=(const BigDecimal &) const
{
		return false;
}
bool BigDecimal::operator<(const BigDecimal &) const
{
		return false;
}
bool BigDecimal::operator>(const BigDecimal &) const
{
		return false;
}
// unsafe operator function
BigDecimal BigDecimal::operator+(const signed &) const
{
		return 0;
}
BigDecimal BigDecimal::operator+(const BigDecimal &) const
{
		return 0;
}
BigDecimal BigDecimal::operator-(const signed &) const
{
		return 0;
}
BigDecimal BigDecimal::operator-(const BigDecimal &) const
{
		return 0;
}
BigDecimal BigDecimal::operator*(const signed &) const
{
		return 0;
}
BigDecimal BigDecimal::operator*(const BigDecimal &) const
{
		return 0;
}
BigDecimal BigDecimal::operator/(const signed &) const
{
		return 0;
}
BigDecimal BigDecimal::operator/(const BigDecimal &) const
{
		return 0;
}
BigDecimal BigDecimal::operator^(size_t n) const
{
		return 0;
}
BigDecimal BigDecimal::operator-() const
{
		return 0;
}
// stream operator
std::ostream &operator<<(std::ostream &out, const BigDecimal &a)
{
		return out;
}