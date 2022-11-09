bool BigInteger_test();
bool big_decimal_test();
bool matrix4_test();


int main(int argv, char *args[])
{
    bool passed = true;
    
    //passed &= BigInteger_test();
    //passed &= big_decimal_test();
    passed &= matrix4_test();
    
    return !passed;
}