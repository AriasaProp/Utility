#include <iostream>

//if NO_TEST defined in build, no test will run
//if one test defined in build, just run that test only

#ifndef NO_TEST

#if defined (SORTING_TEST) || defined (MATH_TEST) || defined (MINING_TEST)

#else

#define SORTING_TEST
#define MATH_TEST
#define MINING_TEST

#endif

#endif

#ifdef MATH_TEST
extern void math_test();
#endif

#ifdef SORTING_TEST
extern void sorting_test();
#endif

#ifdef MINING_TEST
extern "C" void mining_test();
#endif

int main (int argv, char *args[]) {

#ifdef NO_TEST
	std::cout << "no test will run!" << std::endl;
#else

#ifdef MATH_TEST
	math_test();
#endif

#ifdef SORTING_TEST
	sorting_test();
#endif

#ifdef MINING_TEST
	mining_test();
#endif

#endif
}