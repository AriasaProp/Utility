#include <cstring>
#include <fstream>
#include <iostream>
#include <ostream>
#include <cstdlib>
#include <cstdint>

std::ostream *output_file;
char text_buffer[2048];
char *data_address;

extern bool BigInteger_test ();
extern bool matrix_test ();
extern bool complex_number_test ();
extern bool hash_test ();

extern bool extraction_test ();
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
  data_address = (char*) malloc(strlen(args[1]) + 6);
  strcpy(data_address, args[1]);
  strcat(data_address, "/data");
  
  bool TestSucces = 
      // basic class test
      BigInteger_test () &&
      hash_test () &&
      complex_number_test () &&
      matrix_test () &&
      // performance function test
      extraction_test () &&
      mining_test ()
      // others ...
      ;
  if (!TestSucces) std::cerr << "Test error" << std::endl;
  f.close ();
  free(data_address);

  return !TestSucces;
}