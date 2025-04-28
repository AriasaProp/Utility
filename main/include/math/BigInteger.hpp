#ifndef Included_Big_Integer
#define Included_Big_Integer

#include "unit.hpp"

#include <iostream>
#include <vector>

using word = unsigned int;
using wstack = std::vector<word>;

struct BigInteger {
private:
  // values
  bool neg = false;
  wstack words;

public:
  /** Constructors **/
  BigInteger ();
  BigInteger (const BigInteger &);
  BigInteger (const signed);
  BigInteger (const char *);
  BigInteger (const wstack&, bool);
  /** Destructor **/
  ~BigInteger ();
  /** operator casting **/
  operator bool () const;
  explicit operator int () const;
  // operator double() const;
  /** math operational function **/
  BigInteger sqrt () const;
  BigInteger pow (word) const;
  // returning division result and this has remaining
  BigInteger div_mod (const BigInteger);
  /** re-initialize operational function **/
  BigInteger &operator= (const signed);
  BigInteger &operator= (const BigInteger &);
  /** compare operator function **/
  friend bool operator== (const BigInteger &, const signed);
  friend bool operator!= (const BigInteger &, const signed);
  friend bool operator<= (const BigInteger &, const signed);
  friend bool operator>= (const BigInteger &, const signed);
  friend bool operator<  (const BigInteger &, const signed);
  friend bool operator>  (const BigInteger &, const signed);
  friend bool operator== (const BigInteger &, const BigInteger);
  friend bool operator!= (const BigInteger &, const BigInteger);
  friend bool operator<= (const BigInteger &, const BigInteger);
  friend bool operator>= (const BigInteger &, const BigInteger);
  friend bool operator<  (const BigInteger &, const BigInteger);
  friend bool operator>  (const BigInteger &, const BigInteger);
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
  /** stream operator **/
  friend std::ostream &operator<< (std::ostream &, const BigInteger);
};
#endif
