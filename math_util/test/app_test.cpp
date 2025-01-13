extern bool BigInteger_test ();
[[maybe_unused]] extern bool matrix_test ();
[[maybe_unused]] extern bool ComplexNumber_test ();

extern bool Math_test (const char *);


extern bool Mining_test ();

int main (int argv, char *args[]) {

  if (!BigInteger_test ()) return 1;
  /*
    passed &= matrix_test ();
    passed &= ComplexNumber_test ();
  */
  if (!Math_test (args[1])) return 1;
  
  
  if (!Mining_test ()) return 1;

  return 0;
}