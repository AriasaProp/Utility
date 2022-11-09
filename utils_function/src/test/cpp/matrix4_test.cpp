#include <chrono>
#include <iostream>
#include "matrix4.h"

bool matrix4_test() {
	bool passed = true;
	bool result;
	auto restart = std::chrono::high_resolution_clock::now();
	std::cout << "Matrix4 code test \n";
	// arithmatics operation test
	matrix4 a(float[16]{6,9,7,2, 6,1,8,21, 6,1,12,4, 7,12,13,9}),
		b(float[16]{9,9,2,1, 8,6,2,1, 8,6,5,9, 6,4,7,24, 7,9,4,9}),
		c(float[16]{15,18,9,3, 14,7,13,30, 12,5,19,28, 14,21,17,18});
	matrix d;
	// "+" operator test
	result = true;
	d = a + b;
	result &= c == d;
	passed &= result;
	if (!result) std::cout << "    '+' operator error \n";
	// "-" operator test
	result = true;
	d = c - b;
	result &= a == d;
	passed &= result;
	if (!result) std::cout << "    '-' operator error \n";
	// "+=" operator test
	result = true;
	d = a;
	d += b;
	result &= c == d;
	if (!result) std::cout << "    '+=' operator error \n";
	passed &= result;
	// "-=" operator test
	d = c;
	d -= b;
	result &= a == d;
	passed &= result;
	if (!result) std::cout << "    '-=' operator error \n";
	c = float[16]{182,154,114,273, 257,281,157,396, 162,144,117,339, 300,268,201,508};
	// "*" operator test
	d = a * b;
	result &= c == d;
	passed &= result;
	if (!result) std::cout << "    '*' operator error \n";
	// "*=" operator test
	result = true;
	d = a;
	d *= b;
	result &= c == d;
	passed &= result;
	if (!result) std::cout << "    '*=' operator error \n";
	std::cout << "In : " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - restart).count() << " us" << std::endl;
	std::cout << (passed ? "has " : " hasn\'t ") << " Passed" << std::endl;
	return passed;
}


