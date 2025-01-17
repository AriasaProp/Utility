#include <cstring>
#include <fstream>

extern bool BigInteger_test (std::ofstream &);
extern bool matrix_test (std::ofstream &);
extern bool ComplexNumber_test (std::ofstream &);
extern bool hash_test (std::ofstream &, const char *);

extern bool extraction_test (std::ofstream &, const char *);
extern bool Mining_test (std::ofstream &);

int main (int argv, char *args[]) {

  char buff[2048];
  strcpy (buff, args[1]);
  strcpy (buff, "/test_report.txt");

  std::ofstream o (buff);

  if (
  	// basic class test
		!BigInteger_test (o) ||
    !matrix_test (o) ||
    !ComplexNumber_test (o) ||
    !hash_test (o, args[1]) ||
    // performance function test
    !extraction_test (o, args[1]) ||
		!Mining_test (o)
		// others ...
  )) return 1;

  o.close ();
  return 0;
}