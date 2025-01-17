#include <fstream>

extern bool BigInteger_test (std::ofstream &);
extern bool matrix_test (std::ofstream &);
extern bool ComplexNumber_test (std::ofstream &);
extern bool hash_test (std::ofstream &, const char *);

extern bool extraction_test (std::ofstream &, const char *);
extern bool Mining_test (std::ofstream &);

int main (int argv, char *args[]) {
  bool passed;

  char buff[2048];
  strcpy (buff, args[1]);
  strcpy (buff, "/test_report.txt");

  std::ofstream o (buff);

  passed = BigInteger_test (o) &&
      matrix_test (o) &&
      ComplexNumber_test (o) &&
      hash_test (o);

  if (!extraction_test (o, args[1])) return 1;

  if (!Mining_test (o)) return 1;

  outfile.close ();
  return 0;
}