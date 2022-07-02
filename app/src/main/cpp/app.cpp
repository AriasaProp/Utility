#include <iostream>
#include <stdlib.h>
#include "app.h"

std::string Function_Test::Greeter::greeting() {
    return std::string("Hello, World!");
}

int main () {
    Function_Test::Greeter greeter;
    std::cout << greeter.greeting() << std::endl;
    return 0;
}
