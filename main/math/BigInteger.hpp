#ifndef Included_Big_Integer
#define Included_Big_Integer

#include "unit.hpp"

#include <iostream>
#include <vector>

typedef unsigned long word;

struct BigInteger {
private:
  // values
  bool neg = false;
  std::vector<word> words;

public:
  /** Constructors **/
  BigInteger ();
  BigInteger (const BigInteger &);
  BigInteger (const signed);
  BigInteger (const char *);
  BigInteger (const std::vector<word>&, bool);
  /** Destructor **/
  ~BigInteger ();
  /** operator casting **/
  operator bool () const;
  explicit operator int () const;
  explicit operator char () const;
  // operator double() const;
  /** math operational function **/
  BigInteger sqrt () const;
  // returning division result and this has remaining
  BigInteger div_mod (const BigInteger);
  /** re-initialize operational function **/
  BigInteger &operator= (const signed);
  BigInteger &operator= (const BigInteger &);
  /** compare operator function **/
  bool operator== (const signed) const;
  bool operator!= (const signed) const;
  bool operator<= (const signed) const;
  bool operator>= (const signed) const;
  bool operator<  (const signed) const;
  bool operator>  (const signed) const;
  bool operator== (const BigInteger) const;
  bool operator!= (const BigInteger) const;
  bool operator<= (const BigInteger) const;
  bool operator>= (const BigInteger) const;
  bool operator<  (const BigInteger) const;
  bool operator>  (const BigInteger) const;
  /** safe operator math function **/
  BigInteger &operator-- ();
  BigInteger &operator++ ();
  BigInteger &operator+= (const signed);
  BigInteger &operator-= (const signed);
  BigInteger &operator*= (const signed);
  BigInteger &operator/= (const signed);
  BigInteger &operator%= (const signed);
  BigInteger &operator+= (const BigInteger);
  BigInteger &operator-= (const BigInteger);
  BigInteger &operator*= (const BigInteger);
  BigInteger &operator/= (const BigInteger);
  BigInteger &operator%= (const BigInteger);
  BigInteger &operator^= (const size_t);
  BigInteger &operator<<=(const size_t);
  BigInteger &operator>>=(const size_t);
  /** new object generate, operator function **/
  BigInteger operator- () const;
  BigInteger operator+ (const signed) const;
  BigInteger operator- (const signed) const;
  BigInteger operator* (const signed) const;
  BigInteger operator/ (const signed) const;
  BigInteger operator% (const signed) const;
  BigInteger operator+ (const BigInteger) const;
  BigInteger operator- (const BigInteger) const;
  BigInteger operator* (const BigInteger) const;
  BigInteger operator/ (const BigInteger) const;
  BigInteger operator% (const BigInteger) const;
  BigInteger operator^ (const size_t) const;
  BigInteger operator<<(const size_t) const;
  BigInteger operator>>(const size_t) const;
  /** stream operator **/
  friend std::ostream &operator<< (std::ostream &, const BigInteger);
};
#endif
