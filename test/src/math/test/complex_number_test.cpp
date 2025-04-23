#include "math/ComplexNumber.hpp"
#include "util/simple_clock.hpp"

#include <ostream>
#include <iostream>

extern std::ostream *output_file;

// test
bool ComplexNumber_test () {
	*output_file << "ComplexNumber Test: ";
	simple_timer_t c;
	try {
	  if (ComplexNumber (5, 7) != (ComplexNumber (2, 3) + ComplexNumber (3, 4)))
	  	throw "complex addition wrong";
	  if (ComplexNumber (-13, 9) != (ComplexNumber (-6, 8) - ComplexNumber (7, -1)))
	  	throw "complex subtract wrong";
	  if (ComplexNumber (4, 12) != (ComplexNumber (-1, 2) * ComplexNumber (4, -4)))
	  	throw "complex multiply wrong";
	  if (7 != (ComplexNumber (7, 14) / ComplexNumber (1, 2)))
	  	throw "complex division wrong";
	  if (ComplexNumber (-3, 6) != (-3 + 6 * ComplexNumber (0, 1)))
	  	throw "complex operate with number is wrong";
	  if (ComplexNumber (122.0f, -597.0f) != (ComplexNumber (2, 3) ^ 5))
	  	throw "complex powerOf wrong";
	} catch (const char *err) {
		std::cerr << "Error: " << err << std::endl;
		return false;
	}
	*output_file << c.end() << std::endl;
  return true;
}
