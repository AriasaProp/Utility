#include "simple_profiling.hpp"

#include <iostream>

//if NO_TEST defined in build, no test will run
//if one test defined in build, just run that test only
std::ostream *output_file;
char *text_buffer;
char *data_address;


#ifndef NO_TEST

#if !defined (SORTING_TEST) && !defined (MATH_TEST) && !defined (MINING_TEST) && !defined (CODEC_TEST)

#define SORTING_TEST
#define MATH_TEST
#define MINING_TEST
#define CODEC_TEST

#endif

#endif

#ifdef MATH_TEST
extern void QuickMath_test ();
extern void BigInteger_test ();
extern void ComplexNumber_test ();
extern void Matrix_test ();
#endif

#ifdef SORTING_TEST
extern void sorting_test();
#endif

#ifdef MINING_TEST
extern "C" void mining_test();
#endif

#ifdef CODEC_TEST
extern void hasher_test();
#endif

int main (int argv, char **args) {

#ifdef NO_TEST
	std::cout << "no test will run!" << std::endl;
#else
  data_address = (char*)malloc(512);
  text_buffer = (char*)malloc(2048);
  output_file = (std::ostream *)&std::cout;

  //strcpy (text_buffer, args[1]);
  //strcat (text_buffer, "/test_report.txt");
  /*
  std::ofstream f (text_buffer, std::fstream::trunc | std::fstream::out);
  if (!f.is_open ()) {
    std::cerr << "File test output error" << text_buffer << std::endl;
    return 1;
  }
  */

#ifdef MATH_TEST
  strcpy(data_address, "data/math");
  QuickMath_test ();
  BigInteger_test ();
  ComplexNumber_test ();
  Matrix_test ();
#endif

#ifdef SORTING_TEST
	sorting_test();
#endif

#ifdef MINING_TEST
	mining_test();
#endif

#ifdef CODEC_TEST
	strcpy(data_address, "data/codec");
  hasher_test();
#endif

  //f.close ();

  //return !TestSucces;
  free (data_address);
  free (text_buffer);
  
  
  //print_resources(*output_file);
#endif
}