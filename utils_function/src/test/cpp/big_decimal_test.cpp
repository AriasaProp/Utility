#include "big_decimal.h"

bool big_decimal_test()
{
    std::cout << "big_decimal test begin! " << std::endl;
    try {
	    big_decimal x = 1.333333f;
	    big_decimal y = 3.145928f;
	    big_decimal z = 7.0;
	    z /= 3;
	    
	    std::cout << x << std::endl;
	    std::cout << y << std::endl;
	    std::cout << z << std::endl;
    } catch (const char *er) {
    	std::cout << er << std::endl;
    }
    std::cout << "big_decimal test end! " << std::endl;
    return true;
}