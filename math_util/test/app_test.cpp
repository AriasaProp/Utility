#include <cstring>
#include <fstream>
#include <iostream>

std::ofstream output_file;
char text_buffer[2048];

extern bool BigInteger_test ();
extern bool matrix_test ();
extern bool ComplexNumber_test ();
extern bool hash_test (const char *);

extern bool extraction_test (const char *);
extern bool mining_test ();

int main (int argv, char *args[]) {

  strcpy (text_buffer, args[1]);
  strcpy (text_buffer, "/test_report.txt");

  output_file.open (text_buffer, std::ios::trunc | std::ios::out);
  if (!output_file.is_open ()) {
    std::cerr << "File test output error" << text_buffer << std::endl;
    return 1;
  }

  bool TestFailed =
      // basic class test
      !BigInteger_test () ||
      !matrix_test () ||
      !ComplexNumber_test () ||
      !hash_test (args[1]) ||
      // performance function test
      !extraction_test (args[1]) ||
      !mining_test ()
      // others ...
      ;

  if (TestFailed) std::cerr << "Test error" << std::endl;

  output_file.close ();
  return TestFailed;
}