#include "utils/big_decimal.h"

int main(int argc, char * argv[])
{
    std::cout << "big_decimal test begin! " << std::endl;
    
    big_decimal x = 1.333333;
    big_decimal y = 3.145928;
    big_decimal z = 7.0;
    z /= 3;
    
    std::cout << x << std::endl;
    std::cout << y << std::endl;
    std::cout << z << std::endl;
    
    return EXIT_SUCCESS;
}