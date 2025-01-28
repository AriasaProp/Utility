#include "complex_number.hpp"
#include "simple_clock.hpp"

#include <ostream>
#include <iostream>

extern std::ostream *output_file;

// test
bool complex_number_test () {
	*output_file << "complex_number Test: ";
	simple_timer_t c;
	try {
	  if (complex_number (5, 7) != (complex_number (2, 3) + complex_number (3, 4)))
	  	throw "complex addition wrong";
	  if (complex_number (-13, 9) != (complex_number (-6, 8) - complex_number (7, -1)))
	  	throw "complex subtract wrong";
	  if (complex_number (4, 12) != (complex_number (-1, 2) * complex_number (4, -4)))
	  	throw "complex multiply wrong";
	  if (7 != (complex_number (7, 14) / complex_number (1, 2)))
	  	throw "complex division wrong";
	  if (complex_number (-3, 6) != (-3 + 6 * complex_number (0, 1)))
	  	throw "complex operate with number is wrong";
	  if (complex_number (122.0f, -597.0f) != (complex_number (2, 3) ^ 5))
	  	throw "complex powerOf wrong";
	} catch (const char *err) {
		std::cerr << "Error: " << err << std::endl;
		return false;
	}
	*output_file << c.end() << std::endl;
  return true;
}
