#include <cstring>
#include <fstream>
#include <iostream>
#include <ostream>
#include <cstdlib>
#include <cstdint>

std::ostream *output_file;
char text_buffer[2048];
const char *data_address = "data/math";

extern bool BigInteger_test ();
extern bool matrix_test ();
extern bool ComplexNumber_test ();

extern bool extraction_test ();

void math_test () {
  //strcpy (text_buffer, args[1]);
  //strcat (text_buffer, "/test_report.txt");
  /*
  std::ofstream f (text_buffer, std::fstream::trunc | std::fstream::out);
  if (!f.is_open ()) {
    std::cerr << "File test output error" << text_buffer << std::endl;
    return 1;
  }
  */
  output_file = (std::ostream *)&std::cout;
  
  bool TestSucces = 
      // basic class test
      BigInteger_test () &&
      ComplexNumber_test () &&
      matrix_test () &&
      // performance function test
      extraction_test ()
      // others ...
      ;
  if (!TestSucces) std::cerr << "Test error" << std::endl;
  //f.close ();

  //return !TestSucces;
}