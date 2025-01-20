#include <cstring>
#include <fstream>
#include <iostream>
#include <ostream>

std::ostream *output_file;
char text_buffer[2048];

extern bool BigInteger_test ();
extern bool matrix_test ();
extern bool ComplexNumber_test ();
extern bool hash_test (const char *);

extern bool extraction_test (const char *);
extern bool mining_test ();

int main (int argv, char *args[]) {
  strcpy (text_buffer, args[1]);
  strcat (text_buffer, "/test_report.txt");
  
  std::ofstream f (text_buffer, std::fstream::trunc | std::fstream::out);
  if (!f.is_open ()) {
    std::cerr << "File test output error" << text_buffer << std::endl;
    return 1;
  }
  output_file = (std::ostream *)&f;
  
  std::cout << "Nothing" << std::endl;

  bool TestFailed = 
      // basic class test
      !BigInteger_test () ||
      !matrix_test () /* ||
      !ComplexNumber_test () ||
      !hash_test (args[1]) ||
      // performance function test
      !extraction_test (args[1]) ||
      !mining_test ()
      // others ...
*/
      ;
  if (TestFailed) std::cerr << "Test error" << std::endl;
  f.close ();

  return TestFailed;
}