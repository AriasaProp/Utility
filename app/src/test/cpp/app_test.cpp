bool BigInteger_test();
bool big_decimal_test();


int main(int argv, char *args[])
{
    bool passed = true;
    
    passed &= BigInteger_test();
    
    passed &= big_decimal_test();
    
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}